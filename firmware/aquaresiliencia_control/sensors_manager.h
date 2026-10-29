/**
 * ============================================================================
 * PROYECTO: AquaResiliencia Tijuana — Sistema Automatizado de Gestión Hídrica
 * FIRMWARE: ESP32 DevKit v1 (WROOM-32)
 * ARCHIVO:  sensors_manager.h
 * ============================================================================
 */

#ifndef SENSORS_MANAGER_H
#define SENSORS_MANAGER_H

#include <Arduino.h>
#include "config.h"

// Estructura que almacena los estados procesados de todos los sensores
struct SensorData {
    // [P-01] Pozo somero
    bool wellWaterPresent;       // true si hay agua en el pozo
    int  wellRawADC;             // Lectura cruda ADC (0-4095)

    // [P-02] Presostato / Transductor Ósmosis
    float roPressurePSI;         // Presión calculada en PSI
    bool  roPressureBelow60;     // true si presión < 60 PSI (demanda de arranque)
    bool  roPressureCutoff80;    // true si presión >= 80 PSI (corte de alta)

    // [P-05] Tótem 2 (Tanque superior)
    bool totem2Full;             // true si el tanque superior alcanzó el nivel máximo

    // [P-06] Tótem 1 (Tanque inferior)
    bool totem1HasMin20L;        // true si hay >= 20 Litros disponibles

    // [P-07] Barril Presurizado
    bool barrelCutoffReached;    // true si el presostato auxiliar cortó a 60 PSI

    // [P-08] Caudalímetro
    float flowRateLPM;           // Caudal instantáneo en Litros por Minuto (L/min)
    float totalLiters;           // Volumen total acumulado histórico (L)
    float batchLiters;           // Volumen acumulado en el lote de floculación actual (L)
    unsigned long rawPulseCount; // Total de pulsos contados
};

class SensorsManager {
public:
    SensorsManager();
    void begin();
    void update();
    
    // Obtener los datos más recientes
    SensorData getData();

    // Gestión de lote de floculación y volumen
    void resetBatchVolume();
    void resetTotalVolume();

    // Notificación de pulso desde la interrupción
    void handleFlowPulse();

private:
    SensorData data;

    // Control de tiempo para cálculo de caudal cada 1 segundo
    unsigned long lastFlowCalcTime;
    volatile unsigned long isrPulseDelta;
    volatile unsigned long isrTotalPulses;

    // Filtro anti-rebote (debounce) para boyas y presostatos
    unsigned long lastDebounceTimeP01;
    unsigned long lastDebounceTimeP05;
    unsigned long lastDebounceTimeP06;
    unsigned long lastDebounceTimeP07;

    bool lastStateP01;
    bool lastStateP05;
    bool lastStateP06;
    bool lastStateP07;

    float readROPressurePSI();
};

extern SensorsManager sensors;

#endif // SENSORS_MANAGER_H
