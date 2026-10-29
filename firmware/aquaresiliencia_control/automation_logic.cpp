/**
 * ============================================================================
 * PROYECTO: AquaResiliencia Tijuana — Sistema Automatizado de Gestión Hídrica
 * FIRMWARE: ESP32 DevKit v1 (WROOM-32)
 * ARCHIVO:  automation_logic.cpp
 * ============================================================================
 */

#include "automation_logic.h"

AutomationLogic automation;

AutomationLogic::AutomationLogic() {
    flocTargetLiters = DEFAULT_FLOC_TARGET_LITERS;
    flocBatchActive = false;
    flocBatchStartTime = 0;
    dryRunAlarm = false;
    totem1LowAlarm = false;
    roActiveCycle = false;
    diagState = STATE_STANDBY;
    snprintf(lastEventBuffer, sizeof(lastEventBuffer), "Sistema en espera (Standby)");
}

void AutomationLogic::begin() {
    logEvent("Motor de reglas de automatización inicializado.");
}

void AutomationLogic::logEvent(const char* message) {
    snprintf(lastEventBuffer, sizeof(lastEventBuffer), "[%lu s] %s", millis() / 1000UL, message);
    Serial.println(lastEventBuffer);
}

const char* AutomationLogic::getLastSystemEvent() const {
    return lastEventBuffer;
}

void AutomationLogic::setFlocTargetVolume(float liters) {
    if (liters < MIN_FLOC_TARGET_LITERS) liters = MIN_FLOC_TARGET_LITERS;
    if (liters > MAX_FLOC_TARGET_LITERS) liters = MAX_FLOC_TARGET_LITERS;
    flocTargetLiters = liters;

    char msg[80];
    snprintf(msg, sizeof(msg), "Objetivo de lote de floculación actualizado a %.1f Litros.", flocTargetLiters);
    logEvent(msg);
}

float AutomationLogic::getFlocTargetVolume() const {
    return flocTargetLiters;
}

void AutomationLogic::startFlocBatch() {
    if (relays.isEmergencyStopActive()) {
        logEvent("No se puede iniciar lote: PARO DE EMERGENCIA ACTIVO.");
        return;
    }

    flocBatchActive = true;
    flocBatchStartTime = millis();
    sensors.resetBatchVolume();

    // Abrir válvula solenoide de gravedad (Relé 3)
    relays.setRelay(RELAY_GRAVITY_VALV, true, true);

    char msg[80];
    snprintf(msg, sizeof(msg), "Iniciado lote de floculación por gravedad (Meta: %.1f L).", flocTargetLiters);
    logEvent(msg);
}

void AutomationLogic::stopFlocBatch() {
    flocBatchActive = false;

    // Cerrar válvula solenoide de gravedad (Relé 3)
    relays.setRelay(RELAY_GRAVITY_VALV, false, true);

    logEvent("Lote de floculación detenido manualmente.");
}

bool AutomationLogic::isFlocBatchActive() const {
    return flocBatchActive;
}

float AutomationLogic::getFlocBatchProgressPercent() const {
    if (flocTargetLiters <= 0.0f) return 0.0f;
    float current = sensors.getData().batchLiters;
    float pct = (current / flocTargetLiters) * 100.0f;
    if (pct > 100.0f) pct = 100.0f;
    return pct;
}

void AutomationLogic::update() {
    // Si hay paro de emergencia, no evaluar reglas automáticas
    if (relays.isEmergencyStopActive()) {
        diagState = STATE_EMERGENCY_LOCKOUT;
        return;
    }

    // Si el modo de operación es manual, no forzar cambios automáticos en los relés
    if (relays.getOperationMode() == MODE_MANUAL) {
        return;
    }

    SensorData s = sensors.getData();

    // Evaluar las 3 reglas de automatización
    evaluateWellPumpRule(s);
    evaluateOsmosisRule(s);
    evaluateFlocculationRule(s);
}

// ----------------------------------------------------------------------------
// REGLA 1: Bomba de Pozo (Relé 1)
// - Enciende si: [P-01] tiene agua Y [P-05] (Tótem 2) no está lleno.
// - Apaga si: Tótem 2 se llena O pozo detecta nivel bajo (marcha en seco).
// ----------------------------------------------------------------------------
void AutomationLogic::evaluateWellPumpRule(const SensorData& s) {
    bool pumpCurrentlyRunning = relays.getRelayState(RELAY_PUMP_WELL);

#if WELL_LEVEL_SENSOR_INSTALLED
    // Condición de paro por pozo seco (Solo si el sensor está instalado físicamente)
    if (!s.wellWaterPresent) {
        if (pumpCurrentlyRunning) {
            relays.setRelay(RELAY_PUMP_WELL, false, true);
            dryRunAlarm = true;
            diagState = STATE_WELL_DRY_PROTECTION;
            logEvent("ALERTA: Bomba de pozo apagada por PROTECCIÓN CONTRA TRABAJO EN SECO.");
        }
        return;
    } else {
        dryRunAlarm = false;
    }
    bool wellReady = s.wellWaterPresent;
#else
    dryRunAlarm = false;
    bool wellReady = true; // Sin sensor en pozo: el control depende de la capacidad del Tótem 2
#endif

    // Condición de paro por Tótem 2 (Tanque superior) lleno - PRIORIDAD MÁXIMA DE SEGURIDAD
    if (s.totem2Full) {
        if (pumpCurrentlyRunning) {
            relays.setRelay(RELAY_PUMP_WELL, false, true);
            diagState = STATE_TOTEM2_FULL;
            logEvent("Tótem 2 superior lleno (100%). Bomba de pozo detenida por nivel máximo de seguridad.");
        }
        return;
    }

    // Condición de arranque: Pozo listo y Tótem 2 superior tiene capacidad libre
    if (wellReady && !s.totem2Full && !pumpCurrentlyRunning) {
        relays.setRelay(RELAY_PUMP_WELL, true, true);
        diagState = STATE_PUMPING_WELL;
        logEvent("Arrancando Bomba de Pozo: Llenando Tótem 2 superior.");
    }
}

// ----------------------------------------------------------------------------
// REGLA 2: Sistema de Ósmosis Inversa (Relé 2 - Bomba Evans 24V y Solenoide)
// - Presostato Único Mecánico en Cople D del Barril de Cerveza Sanke D (40 PSI)
// - Enciende si: Tótem 1 [P-06] >= 20L Y el barril no ha alcanzado 40 PSI.
// - Apaga si: Barril [P-07] alcanza 40 PSI, o Tótem 1 se queda sin agua (< 20L).
// ----------------------------------------------------------------------------
void AutomationLogic::evaluateOsmosisRule(const SensorData& s) {
    bool roCurrentlyRunning = relays.getRelayState(RELAY_OSMOSIS_VALV);

    // Condición de corte 1: Tótem 1 inferior sin agua suficiente (< 20 Litros)
    if (!s.totem1HasMin20L) {
        if (roCurrentlyRunning) {
            relays.setRelay(RELAY_OSMOSIS_VALV, false, true);
            roActiveCycle = false;
            totem1LowAlarm = true;
            diagState = STATE_OSMOSIS_TOTEM1_EMPTY;
            logEvent("Ósmosis detenida: Tótem 1 inferior por debajo del umbral mínimo de 20 Litros.");
        }
        return;
    } else {
        totem1LowAlarm = false;
    }

    // Condición de corte 2: Barril de cerveza Sanke D lleno (Presostato cortó a 40 PSI)
    if (s.barrelCutoffReached) {
        if (roCurrentlyRunning) {
            relays.setRelay(RELAY_OSMOSIS_VALV, false, true);
            roActiveCycle = false;
            diagState = STATE_BARREL_FULL;
            logEvent("Ósmosis detenida: Presostato en Cople D alcanzó 40 PSI (Barril lleno de agua pura).");
        }
        return;
    }

#if !USE_SINGLE_PRESSURE_SWITCH
    // Corte de alta presión secundario en membrana (si no se usa presostato único)
    if (s.roPressureCutoff80) {
        if (roCurrentlyRunning) {
            relays.setRelay(RELAY_OSMOSIS_VALV, false, true);
            roActiveCycle = false;
            diagState = STATE_OSMOSIS_CUTOFF_HIGH_PSI;
            logEvent("Ósmosis detenida: Presión en membrana alcanzó límite seguro de 80 PSI.");
        }
        return;
    }
#endif

    // Condición de encendido: Hay agua en Tótem 1 y el barril Sanke D necesita agua (< 40 PSI)
    if (s.totem1HasMin20L && !s.barrelCutoffReached && !roCurrentlyRunning) {
        relays.setRelay(RELAY_OSMOSIS_VALV, true, true);
        roActiveCycle = true;
        diagState = STATE_OSMOSIS_PURIFYING;
        logEvent("Activando Ósmosis: Produciendo agua pura hacia Barril Sanke D.");
    }
}

// ----------------------------------------------------------------------------
// REGLA 3: Floculación por Caudal (Relé 3 y Sensor [P-08])
// - Monitorear caudal en L/min en tiempo real.
// - Válvula de gravedad (Relé 3) controlada por volumen de lote ajustado por slider.
// - Al alcanzar el objetivo, cerrar solenoide y registrar evento.
// ----------------------------------------------------------------------------
void AutomationLogic::evaluateFlocculationRule(const SensorData& s) {
    if (flocBatchActive) {
        // Verificar si se alcanzó el volumen de lote establecido
        if (s.batchLiters >= flocTargetLiters) {
            relays.setRelay(RELAY_GRAVITY_VALV, false, true);
            flocBatchActive = false;
            diagState = STATE_FLOC_BATCH_COMPLETED;

            char msg[100];
            snprintf(msg, sizeof(msg), "🎉 LOTE DE FLOCULACIÓN COMPLETADO: %.2f Litros dosificados.", s.batchLiters);
            logEvent(msg);
        } else {
            diagState = STATE_FLOC_BATCH_ACTIVE;
        }
    }
}

bool AutomationLogic::isDryRunAlert() const {
    return dryRunAlarm;
}

bool AutomationLogic::isTotem1LowAlert() const {
    return totem1LowAlarm;
}

SystemDiagnosticState AutomationLogic::getDiagnosticState() const {
    return diagState;
}

const char* AutomationLogic::getDiagnosticStateStr() const {
    switch (diagState) {
        case STATE_STANDBY: return "En Espera (Standby)";
        case STATE_PUMPING_WELL: return "Extrayendo Agua del Pozo";
        case STATE_WELL_DRY_PROTECTION: return "Alerta: Pozo Seco (Protección)";
        case STATE_TOTEM2_FULL: return "Tótem 2 Lleno (100%)";
        case STATE_OSMOSIS_PURIFYING: return "Ósmosis Activa (Purificando)";
        case STATE_OSMOSIS_CUTOFF_HIGH_PSI: return "Ósmosis en Corte (80 PSI)";
        case STATE_OSMOSIS_TOTEM1_EMPTY: return "Alerta: Tótem 1 Sin Agua (<20L)";
        case STATE_BARREL_FULL: return "Barril Hidroneumático Lleno (60 PSI)";
        case STATE_FLOC_BATCH_ACTIVE: return "Dosificando Lote de Floculación";
        case STATE_FLOC_BATCH_COMPLETED: return "Lote de Floculación Completado";
        case STATE_EMERGENCY_LOCKOUT: return "🚨 BLOQUEO DE EMERGENCIA ACTIVO";
        default: return "Operación Normal";
    }
}
