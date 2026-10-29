/**
 * ============================================================================
 * PROYECTO: AquaResiliencia Tijuana — Sistema Automatizado de Gestión Hídrica
 * FIRMWARE: ESP32 DevKit v1 (WROOM-32)
 * ARCHIVO:  relay_controller.cpp
 * ============================================================================
 */

#include "relay_controller.h"

RelayController relays;

RelayController::RelayController() {
    relayPins[0] = PIN_RELAY_1_PUMP;
    relayPins[1] = PIN_RELAY_2_OSMOSIS;
    relayPins[2] = PIN_RELAY_3_GRAVITY;
    relayPins[3] = PIN_RELAY_4_AUX;

    for (int i = 0; i < 4; i++) {
        relayStates[i] = false;
    }

    emergencyStopActive = false;
    currentMode = MODE_AUTO;
}

void RelayController::begin() {
    // Configuración de pin de LED de estado
    pinMode(PIN_STATUS_LED, OUTPUT);
    digitalWrite(PIN_STATUS_LED, LOW);

    // INICIALIZACIÓN SEGURA FAIL-SAFE:
    // Escribir el estado apagado ANTES de configurar como OUTPUT para evitar
    // transitorios indeseados (chatter) durante el encendido del microcontrolador.
    for (int i = 0; i < 4; i++) {
        digitalWrite(relayPins[i], RELAY_STATE_OFF);
        pinMode(relayPins[i], OUTPUT);
        digitalWrite(relayPins[i], RELAY_STATE_OFF);
        relayStates[i] = false;
    }

    Serial.println("[RELAYS] Módulo de 4 canales inicializado en modo SEGURO (OFF).");
    Serial.printf(" - Polaridad activa: %s\n", RELAYS_ACTIVE_LOW ? "LOW (Active-LOW)" : "HIGH (Active-HIGH)");
    Serial.printf(" - Relé 1 (Bomba Pozo): D%d\n", PIN_RELAY_1_PUMP);
    Serial.printf(" - Relé 2 (Ósmosis):    D%d\n", PIN_RELAY_2_OSMOSIS);
    Serial.printf(" - Relé 3 (Gravedad):   D%d\n", PIN_RELAY_3_GRAVITY);
    Serial.printf(" - Relé 4 (Auxiliar):   D%d\n", PIN_RELAY_4_AUX);
}

void RelayController::applyHardwarePinState(uint8_t channel, bool turnOn) {
    if (channel >= 4) return;
    uint8_t pin = relayPins[channel];
    uint8_t physicalState = turnOn ? RELAY_STATE_ON : RELAY_STATE_OFF;
    digitalWrite(pin, physicalState);
    relayStates[channel] = turnOn;
}

bool RelayController::setRelay(uint8_t channel, bool turnOn, bool forceOverride) {
    if (channel >= 4) return false;

    // Si el paro de emergencia está activo, bloquear cualquier intento de encendido
    if (emergencyStopActive && turnOn && !forceOverride) {
        Serial.printf("[RELAYS] Bloqueo: Relé %d denegado por PARO DE EMERGENCIA ACTIVO.\n", channel + 1);
        return false;
    }

    // Si estamos en modo AUTO y no es una acción forzada, permitir solo si la automatización lo instruye
    if (currentMode == MODE_AUTO && !forceOverride) {
        // En modo auto las decisiones las toma automation_logic
    }

    applyHardwarePinState(channel, turnOn);
    return true;
}

bool RelayController::getRelayState(uint8_t channel) const {
    if (channel >= 4) return false;
    return relayStates[channel];
}

void RelayController::triggerEmergencyStop() {
    emergencyStopActive = true;
    currentMode = MODE_EMERGENCY_STOP;

    // Apagado inmediato por hardware de todos los canales
    for (int i = 0; i < 4; i++) {
        applyHardwarePinState(i, false);
    }

    // Encender LED fijo de alarma en placa
    digitalWrite(PIN_STATUS_LED, HIGH);

    Serial.println("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    Serial.println("  🚨 ¡PARO DE EMERGENCIA ACTIVADO! TODOS LOS RELÉS CORTADOS");
    Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
}

void RelayController::resetEmergencyStop() {
    emergencyStopActive = false;
    currentMode = MODE_AUTO; // Regresar a modo automático por defecto

    // Apagar LED de alarma en placa
    digitalWrite(PIN_STATUS_LED, LOW);

    Serial.println("\n[EMERGENCIA] Paro de emergencia rearmado. Sistema en MODO AUTOMÁTICO.\n");
}

bool RelayController::isEmergencyStopActive() const {
    return emergencyStopActive;
}

void RelayController::setOperationMode(SystemOperationMode mode) {
    if (emergencyStopActive && mode != MODE_EMERGENCY_STOP) {
        Serial.println("[RELAYS] Advertencia: No se puede cambiar de modo mientras el Paro de Emergencia esté activo.");
        return;
    }
    currentMode = mode;
    Serial.printf("[SISTEMA] Modo cambiado a: %s\n", getOperationModeStr());
}

SystemOperationMode RelayController::getOperationMode() const {
    return currentMode;
}

const char* RelayController::getOperationModeStr() const {
    switch (currentMode) {
        case MODE_AUTO: return "AUTO";
        case MODE_MANUAL: return "MANUAL";
        case MODE_EMERGENCY_STOP: return "EMERGENCY_STOP";
        default: return "DESCONOCIDO";
    }
}

const char* RelayController::getRelayName(uint8_t channel) const {
    switch (channel) {
        case 0: return "Bomba de Pozo (Relé 1)";
        case 1: return "Solenoide Ósmosis (Relé 2)";
        case 2: return "Solenoide Gravedad (Relé 3)";
        case 3: return "Canal Auxiliar (Relé 4)";
        default: return "Canal Inválido";
    }
}
