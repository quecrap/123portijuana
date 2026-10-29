#ifndef POWER_MGMT_H
#define POWER_MGMT_H

#include <Arduino.h>
#include "config.h"

enum WellPumpState {
    PUMP_OFF,
    PUMP_WORKING,               // Extracción activa (pulso de 30s)
    PUMP_RESTING,               // Descanso freático (pausa de 60s)
    PUMP_LOCKED_DRY,            // Bloqueada por sensor magnético de pozo seco
    PUMP_LOCKED_T1_OVERPRESSURE // Bloqueada por presostato T1 (alcanzó 90 PSI)
};

enum ROState {
    RO_OFF,
    RO_FILLING_T2,              // Bomba booster y electroválvula 120V activas llenando barril limpio
    RO_T2_FULL_PAUSE,           // T2 alcanzó 40 PSI (corte de llenado)
    RO_LOCKED_OVERHEAT          // Bloqueo de seguridad: Agua excede TEMP_RO_MAX_SAFE (protección membrana)
};

class PowerManager {
public:
    PowerManager();
    void begin();
    void update(bool wellLevelSafe, bool t1Cutoff90, bool t1Demand60, bool t2Full40, bool t2Demand20, bool waterOverheat = false);

    // Controles manuales y comandos remotos
    void setManualExtraction(bool enable);
    void setROBooster(bool enable);
    void setSolenoid120V(bool open);
    void emergencyStop();

    // Consulta de estado de actuadores
    WellPumpState getWellPumpState();
    ROState getROState();
    bool isWellPumpRunning();
    bool isROBoosterRunning();
    bool isSolenoid120VOpen();
    bool isAutoMode();

    void setAutoMode(bool enable);

private:
    WellPumpState wellPumpState;
    ROState roState;

    bool autoMode;
    bool manualExtractionOverride;
    bool manualROOverride;
    bool manualSolenoidOverride;

    unsigned long lastPumpStateChange;
    void applyActuators();
};

extern PowerManager powerMgmt;

#endif // POWER_MGMT_H
