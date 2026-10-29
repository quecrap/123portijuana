/**
 * ============================================================================
 * PROYECTO: AquaResiliencia Tijuana — Sistema Automatizado de Gestión Hídrica
 * FIRMWARE: ESP32 DevKit v1 (WROOM-32)
 * ARCHIVO:  relay_controller.h
 * ============================================================================
 */

#ifndef RELAY_CONTROLLER_H
#define RELAY_CONTROLLER_H

#include <Arduino.h>
#include "config.h"

// Enumeración de canales de relés
enum RelayChannel {
    RELAY_PUMP_WELL   = 0, // Relé 1 (IN1 -> D26): Bomba de Pozo
    RELAY_OSMOSIS_VALV = 1, // Relé 2 (IN2 -> D25): Solenoide Ósmosis
    RELAY_GRAVITY_VALV = 2, // Relé 3 (IN3 -> D33): Solenoide Gravedad Floculación
    RELAY_AUX_CHANNEL  = 3  // Relé 4 (IN4 -> D32): Canal Auxiliar / Expansión
};

// Modos globales de operación del sistema
enum SystemOperationMode {
    MODE_AUTO,          // Automatización completa basada en reglas de sensores
    MODE_MANUAL,        // Control manual por operador desde interfaz web
    MODE_EMERGENCY_STOP // Bloqueo de seguridad total (corte forzado de todos los relés)
};

class RelayController {
public:
    RelayController();
    void begin();

    // Control de relés individuales
    bool setRelay(uint8_t channel, bool turnOn, bool forceOverride = false);
    bool getRelayState(uint8_t channel) const;

    // Control de paro de emergencia
    void triggerEmergencyStop();
    void resetEmergencyStop();
    bool isEmergencyStopActive() const;

    // Gestión de modo de operación
    void setOperationMode(SystemOperationMode mode);
    SystemOperationMode getOperationMode() const;
    const char* getOperationModeStr() const;

    // Acceso rápido a nombres descriptivos
    const char* getRelayName(uint8_t channel) const;

private:
    bool relayStates[4];
    uint8_t relayPins[4];
    bool emergencyStopActive;
    SystemOperationMode currentMode;

    void applyHardwarePinState(uint8_t channel, bool turnOn);
};

extern RelayController relays;

#endif // RELAY_CONTROLLER_H
