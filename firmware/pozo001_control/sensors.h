#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include "config.h"

struct SensorData {
    // Hidráulica y Flujo
    float flowRateLPM;              // Caudal instantáneo medido por sensor Steren (L/min)
    float totalLiters;              // Volumen total acumulado (Litros)
    unsigned long pulseCount;       // Contador total de pulsos Hall

    // Enclavamientos de Seguridad y Nivel
    bool wellLevelSafe;             // true = agua presente | false = pozo seco (sensor magnético activado)
    
    // Presiones de Línea y Tanques
    bool pressureT1Cutoff;          // true = T1 presurizado a 90 PSI (corte alcanzado)
    bool pressureT1Demand;          // true = T1 requiere recarga (presión cayó a <= 60 PSI)
    bool t2CleanWaterFull;          // true = T2 (barril agua limpia) presurizado a 40 PSI (lleno)
    bool t2CleanWaterDemand;        // true = T2 cayó a <= 20 PSI (demanda de llenado activa)

    // Climatología y Temperatura de Proceso
    float temperatureC;             // Temperatura cabina (°C)
    float humidityPercent;          // Humedad relativa (%)
    float waterTempC;               // Temperatura del agua en intercambiador/entrada RO (°C, sensor DS18B20)
    bool waterTempOverheat;         // true = Temperatura del agua excede TEMP_RO_MAX_SAFE (Peligro membrana)
};

class SensorManager {
public:
    SensorManager();
    void begin();
    void update();                  // Muestreo periódico en loop()
    SensorData getData();
    
    void resetTotalVolume();

private:
    SensorData data;
    unsigned long lastFlowCalculation;
    unsigned long lastDHTRead;
    unsigned long lastDigitalPoll;

    void calculateFlow();
    void readDigitalSwitches();
    void readDHT();
    void readWaterTemperature();
};

extern SensorManager sensors;

#endif // SENSORS_H
