/**
 * ============================================================================
 * PROYECTO: AquaResiliencia Tijuana — Sistema Automatizado de Gestión Hídrica
 * FIRMWARE: ESP32 DevKit v1 (WROOM-32)
 * ARCHIVO:  automation_logic.h
 * ============================================================================
 */

#ifndef AUTOMATION_LOGIC_H
#define AUTOMATION_LOGIC_H

#include <Arduino.h>
#include "sensors_manager.h"
#include "relay_controller.h"

// Estados de diagnóstico del sistema
enum SystemDiagnosticState {
    STATE_STANDBY,
    STATE_PUMPING_WELL,
    STATE_WELL_DRY_PROTECTION,
    STATE_TOTEM2_FULL,
    STATE_OSMOSIS_PURIFYING,
    STATE_OSMOSIS_CUTOFF_HIGH_PSI,
    STATE_OSMOSIS_TOTEM1_EMPTY,
    STATE_BARREL_FULL,
    STATE_FLOC_BATCH_ACTIVE,
    STATE_FLOC_BATCH_COMPLETED,
    STATE_EMERGENCY_LOCKOUT
};

class AutomationLogic {
public:
    AutomationLogic();
    void begin();
    void update();

    // Configuración dinámica desde la interfaz web (Slider)
    void setFlocTargetVolume(float liters);
    float getFlocTargetVolume() const;

    // Control de lote de floculación por gravedad
    void startFlocBatch();
    void stopFlocBatch();
    bool isFlocBatchActive() const;
    float getFlocBatchProgressPercent() const;

    // Estados de diagnóstico y mensajes legibles
    SystemDiagnosticState getDiagnosticState() const;
    const char* getDiagnosticStateStr() const;
    const char* getLastSystemEvent() const;

    // Verificación de alertas activas
    bool isDryRunAlert() const;
    bool isTotem1LowAlert() const;

private:
    float flocTargetLiters;
    bool flocBatchActive;
    unsigned long flocBatchStartTime;

    bool dryRunAlarm;
    bool totem1LowAlarm;
    bool roActiveCycle;

    SystemDiagnosticState diagState;
    char lastEventBuffer[128];

    void evaluateWellPumpRule(const SensorData& s);
    void evaluateOsmosisRule(const SensorData& s);
    void evaluateFlocculationRule(const SensorData& s);
    void logEvent(const char* message);
};

extern AutomationLogic automation;

#endif // AUTOMATION_LOGIC_H
