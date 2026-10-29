#include "sensors.h"
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Instancia global del sensor DHT22
static DHT dht(PIN_DHT_DATA, DHT22);

// Sensor digital de temperatura de agua DS18B20 por bus OneWire
static OneWire oneWireWater(PIN_ONEWIRE_WATER_TEMP);
static DallasTemperature ds18b20Water(&oneWireWater);

// Contador volátil de pulsos para interrupción externa del caudalímetro
static volatile unsigned long isrPulseCount = 0;

// Rutina de interrupción optimizada en IRAM
void IRAM_ATTR flowSensorISR() {
    isrPulseCount++;
}

SensorManager sensors;

SensorManager::SensorManager() {
    data.flowRateLPM = 0.0f;
    data.totalLiters = 0.0f;
    data.pulseCount = 0;
    data.wellLevelSafe = true;
    data.pressureT1Cutoff = false;
    data.pressureT1Demand = true;
    data.t2CleanWaterFull = false;
    data.t2CleanWaterDemand = true;
    data.temperatureC = 20.0f;
    data.humidityPercent = 50.0f;
    data.waterTempC = 21.0f;
    data.waterTempOverheat = false;

    lastFlowCalculation = 0;
    lastDHTRead = 0;
    lastDigitalPoll = 0;
}

void SensorManager::begin() {
    // 1. Configurar entrada de interrupción para caudalímetro Steren
    pinMode(PIN_FLOW_SENSOR, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_FLOW_SENSOR), flowSensorISR, RISING);

    // 2. Configurar entradas digitales con pull-up para switches de seguridad y presostatos
    pinMode(PIN_MAGNETIC_FLOAT_WELL, INPUT_PULLUP);
    pinMode(PIN_PRESSURE_SWITCH_T1, INPUT_PULLUP);
    pinMode(PIN_PRESSURE_SWITCH_T2, INPUT_PULLUP);

    // 3. Inicializar sensor climático DHT22 y sensor de agua DS18B20
    dht.begin();
    ds18b20Water.begin();
    ds18b20Water.setWaitForConversion(false); // No bloqueante

    lastFlowCalculation = millis();
}

void SensorManager::update() {
    unsigned long currentMillis = millis();

    // 1. Cálculo de caudal cada 1 segundo (1000 ms)
    if (currentMillis - lastFlowCalculation >= 1000) {
        calculateFlow();
        lastFlowCalculation = currentMillis;
    }

    // 2. Sondeo de presostatos y contacto magnético cada 100 ms (respuesta rápida)
    if (currentMillis - lastDigitalPoll >= 100) {
        readDigitalSwitches();
        lastDigitalPoll = currentMillis;
    }

    // 3. Lectura de DHT22 y DS18B20 cada 2.5 segundos
    if (currentMillis - lastDHTRead >= 2500) {
        readDHT();
        readWaterTemperature();
        lastDHTRead = currentMillis;
    }
}

void SensorManager::calculateFlow() {
    noInterrupts();
    unsigned long pulses = isrPulseCount;
    isrPulseCount = 0;
    interrupts();

    data.pulseCount += pulses;

    // Fórmula Steren / YF-S201: Frecuencia (Hz) = 7.5 * Q (L/min)
    data.flowRateLPM = ((float)pulses) / FLOW_CALIBRATION_FACTOR;

    // Acumulación de litros por intervalo de 1s
    float litersInInterval = ((float)pulses) / (FLOW_CALIBRATION_FACTOR * 60.0f);
    data.totalLiters += litersInInterval;
}

void SensorManager::readDigitalSwitches() {
    // 1. Sensor magnético tipo alarma de puerta en el pozo
    // Con INPUT_PULLUP: Si el contacto cierra a GND (ej. flotador al fondo por pozo seco) -> LOW
    int magState = digitalRead(PIN_MAGNETIC_FLOAT_WELL);
    data.wellLevelSafe = (magState != LOW); // true si NO está activada la alarma de pozo seco

    // 2. Presostato T1 (60-90 PSI)
    // Cuando la presión llega a 90 PSI, el presostato abre el circuito -> HIGH
    // Cuando la presión cae a 60 PSI, cierra el circuito demandando agua -> LOW
    int p1State = digitalRead(PIN_PRESSURE_SWITCH_T1);
    data.pressureT1Cutoff = (p1State == HIGH);   // true = alcanzó 90 PSI (cortar bomba)
    data.pressureT1Demand = (p1State == LOW);    // true = cayó a 60 PSI (pedir bomba)

    // 3. Presostato T2 Barril Agua Limpia (20-40 PSI)
    // A 40 PSI abre circuito -> HIGH (barril lleno)
    // A <= 20 PSI cierra circuito -> LOW (demanda de llenado)
    int p2State = digitalRead(PIN_PRESSURE_SWITCH_T2);
    data.t2CleanWaterFull = (p2State == HIGH);   // true = 40 PSI, barril lleno
    data.t2CleanWaterDemand = (p2State == LOW);  // true = <=20 PSI, barril pide agua
}

void SensorManager::readDHT() {
    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t)) {
        data.temperatureC = t;
    }
    if (!isnan(h)) {
        data.humidityPercent = h;
    }
}

void SensorManager::readWaterTemperature() {
    // Solicitar lectura no bloqueante al DS18B20
    float tw = ds18b20Water.getTempCByIndex(0);
    ds18b20Water.requestTemperatures(); // Preparar siguiente conversión

    // Filtrar lecturas inválidas (-127C es desconectado en DallasTemperature)
    if (tw > -50.0f && tw < 125.0f) {
        data.waterTempC = tw;

        // Evaluación de sobretemperatura con histéresis
        if (tw >= TEMP_RO_MAX_SAFE) {
            data.waterTempOverheat = true;
        } else if (tw <= (TEMP_RO_MAX_SAFE - TEMP_RO_RESUME_HYST)) {
            data.waterTempOverheat = false;
        }
    }
}

SensorData SensorManager::getData() {
    return data;
}

void SensorManager::resetTotalVolume() {
    data.totalLiters = 0.0f;
    data.pulseCount = 0;
}
