#include "power_mgmt.h"

// Módulos de relés optoacoplados: Activos en LOW
#define RELAY_ON   LOW
#define RELAY_OFF  HIGH

PowerManager powerMgmt;

PowerManager::PowerManager() {
    wellPumpState = PUMP_OFF;
    roState = RO_OFF;
    autoMode = true;
    manualExtractionOverride = false;
    manualROOverride = false;
    manualSolenoidOverride = false;
    lastPumpStateChange = 0;
}

void PowerManager::begin() {
    // 1. Configurar pines de relés como salidas e iniciar apagados (FAIL-SAFE)
    pinMode(PIN_RELAY_PUMP_WELL, OUTPUT);
    pinMode(PIN_RELAY_RO_BOOSTER, OUTPUT);
    pinMode(PIN_RELAY_SOLENOID_120V, OUTPUT);
    pinMode(PIN_LED_STATUS, OUTPUT);

    digitalWrite(PIN_RELAY_PUMP_WELL, RELAY_OFF);
    digitalWrite(PIN_RELAY_RO_BOOSTER, RELAY_OFF);
    digitalWrite(PIN_RELAY_SOLENOID_120V, RELAY_OFF);
    digitalWrite(PIN_LED_STATUS, LOW);

    lastPumpStateChange = millis();
}

void PowerManager::update(bool wellLevelSafe, bool t1Cutoff90, bool t1Demand60, bool t2Full40, bool t2Demand20, bool waterOverheat) {
    unsigned long currentMillis = millis();

    // ========================================================================
    // 1. ENCLAVAMIENTOS DE SEGURIDAD CRÍTICOS (INTERLOCKS)
    // ========================================================================

    // Enclavamiento 1: Pozo Seco detectado por sensor magnético al fondo
    if (!wellLevelSafe) {
        wellPumpState = PUMP_LOCKED_DRY;
        applyActuators();
        return;
    }

    // Enclavamiento 2: Presostato T1 llegó a 90 PSI (corte de sobrepresión en barril T1)
    if (t1Cutoff90) {
        wellPumpState = PUMP_LOCKED_T1_OVERPRESSURE;
    } else if (wellPumpState == PUMP_LOCKED_T1_OVERPRESSURE && t1Demand60) {
        // Rearme automático si la presión cayó a 60 PSI
        wellPumpState = PUMP_OFF;
    }

    // ========================================================================
    // 2. MÁQUINA DE ESTADOS: BOMBA 1 EXTRACCIÓN (24V 1A)
    // ========================================================================
    bool allowExtraction = (autoMode && !t1Cutoff90) || manualExtractionOverride;

    if (!allowExtraction) {
        if (wellPumpState != PUMP_LOCKED_T1_OVERPRESSURE) {
            wellPumpState = PUMP_OFF;
        }
    } else {
        // Ciclo intermitente: 30s bombeo activo / 60s descanso freático
        if (wellPumpState == PUMP_OFF || wellPumpState == PUMP_LOCKED_DRY) {
            wellPumpState = PUMP_WORKING;
            lastPumpStateChange = currentMillis;
        } else if (wellPumpState == PUMP_WORKING) {
            if (currentMillis - lastPumpStateChange >= PUMP_WORK_DURATION_MS) {
                wellPumpState = PUMP_RESTING; // Pausa para recarga de venero
                lastPumpStateChange = currentMillis;
            }
        } else if (wellPumpState == PUMP_RESTING) {
            if (currentMillis - lastPumpStateChange >= PUMP_REST_DURATION_MS) {
                wellPumpState = PUMP_WORKING; // Reanudar pulso
                lastPumpStateChange = currentMillis;
            }
        }
    }

    // ========================================================================
    // 3. MÁQUINA DE ESTADOS: ÓSMOSIS INVERSA Y LLENADO BARRIL T2 (AGUA LIMPIA)
    // ========================================================================
    // Enclavamiento Térmico: Si el agua excede el umbral seguro (32°C), forzar corte de RO
    if (waterOverheat) {
        roState = RO_LOCKED_OVERHEAT;
    } else if (roState == RO_LOCKED_OVERHEAT && !waterOverheat) {
        roState = RO_OFF; // Enfriamiento completado (histéresis superada), listo para reanudar
    }

    if (roState != RO_LOCKED_OVERHEAT) {
        // Control de presión T2: Dejar de llenar a 40 PSI, volver a llenar a 20 PSI
        if (t2Full40) {
            roState = RO_T2_FULL_PAUSE; // Barril limpio lleno
        } else if (t2Demand20 || roState == RO_FILLING_T2) {
            // Si T2 pide agua y el sistema no está en paro de emergencia
            if (!t2Full40 && (autoMode || manualROOverride)) {
                roState = RO_FILLING_T2;
            } else {
                roState = RO_OFF;
            }
        } else if (!autoMode && !manualROOverride) {
            roState = RO_OFF;
        }
    }

    applyActuators();
}

void PowerManager::applyActuators() {
    // 1. Actuador Bomba 1 Pozo (24V)
    if (wellPumpState == PUMP_WORKING) {
        digitalWrite(PIN_RELAY_PUMP_WELL, RELAY_ON);
        digitalWrite(PIN_LED_STATUS, HIGH);
    } else {
        digitalWrite(PIN_RELAY_PUMP_WELL, RELAY_OFF);
        digitalWrite(PIN_LED_STATUS, LOW);
    }

    // 2. Actuador Bomba Booster RO
    bool runBooster = (roState == RO_FILLING_T2) || manualROOverride;
    digitalWrite(PIN_RELAY_RO_BOOSTER, runBooster ? RELAY_ON : RELAY_OFF);

    // 3. Actuador Electroválvula 120 VAC
    // Se abre cuando la Booster RO está operando o por anulación manual
    bool openSolenoid = (roState == RO_FILLING_T2) || manualSolenoidOverride;
    digitalWrite(PIN_RELAY_SOLENOID_120V, openSolenoid ? RELAY_ON : RELAY_OFF);
}

void PowerManager::setManualExtraction(bool enable) {
    manualExtractionOverride = enable;
    if (!enable && wellPumpState == PUMP_WORKING) {
        wellPumpState = PUMP_OFF;
    }
}

void PowerManager::setROBooster(bool enable) {
    manualROOverride = enable;
}

void PowerManager::setSolenoid120V(bool open) {
    manualSolenoidOverride = open;
}

void PowerManager::emergencyStop() {
    autoMode = false;
    manualExtractionOverride = false;
    manualROOverride = false;
    manualSolenoidOverride = false;
    wellPumpState = PUMP_OFF;
    roState = RO_OFF;
    applyActuators();
}

void PowerManager::setAutoMode(bool enable) {
    autoMode = enable;
}

WellPumpState PowerManager::getWellPumpState() {
    return wellPumpState;
}

ROState PowerManager::getROState() {
    return roState;
}

bool PowerManager::isWellPumpRunning() {
    return (wellPumpState == PUMP_WORKING);
}

bool PowerManager::isROBoosterRunning() {
    return (roState == RO_FILLING_T2 || manualROOverride);
}

bool PowerManager::isSolenoid120VOpen() {
    return (roState == RO_FILLING_T2 || manualSolenoidOverride);
}

bool PowerManager::isAutoMode() {
    return autoMode;
}
