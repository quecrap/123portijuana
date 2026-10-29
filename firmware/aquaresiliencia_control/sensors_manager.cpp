/**
 * ============================================================================
 * PROYECTO: AquaResiliencia Tijuana — Sistema Automatizado de Gestión Hídrica
 * HARDWARE: ESP8266 (ESP8266MOD / NodeMCU) y ESP32 DevKit
 * ARCHIVO:  sensors_manager.cpp
 * ============================================================================
 */

#include "sensors_manager.h"

SensorsManager sensors;

// Rutina de Servicio de Interrupción (ISR) para pulsos del caudalímetro
#if defined(ESP8266)
static void ICACHE_RAM_ATTR isrFlowSensor() {
    sensors.handleFlowPulse();
}
#else
static void IRAM_ATTR isrFlowSensor() {
    sensors.handleFlowPulse();
}
#endif

SensorsManager::SensorsManager() {
    data.wellWaterPresent = false;
    data.wellRawADC = 0;
    data.roPressurePSI = 0.0f;
    data.roPressureBelow60 = false;
    data.roPressureCutoff80 = false;
    data.totem2Full = false;
    data.totem1HasMin20L = false;
    data.barrelCutoffReached = false;
    data.flowRateLPM = 0.0f;
    data.totalLiters = 0.0f;
    data.batchLiters = 0.0f;
    data.rawPulseCount = 0;

    lastFlowCalcTime = 0;
    isrPulseDelta = 0;
    isrTotalPulses = 0;

    lastDebounceTimeP01 = 0;
    lastDebounceTimeP05 = 0;
    lastDebounceTimeP06 = 0;
    lastDebounceTimeP07 = 0;

    lastStateP01 = false;
    lastStateP05 = false;
    lastStateP06 = false;
    lastStateP07 = false;
}

void SensorsManager::begin() {
    // Configuración de pines de entrada según la arquitectura de microcontrolador
#if defined(ESP8266)
    pinMode(PIN_P01_WELL_WATER, INPUT);
    pinMode(PIN_P02_RO_PRESSURE, INPUT); // A0 (ADC0)
    pinMode(PIN_P05_TOTEM2_MAX, INPUT_PULLUP);
    pinMode(PIN_P06_TOTEM1_MIN, INPUT_PULLUP);
    pinMode(PIN_P07_BARREL_PRESSURE, INPUT); // D8 tiene pulldown físico en NodeMCU
    pinMode(PIN_P08_FLOW_METER, INPUT_PULLUP);
#else
    pinMode(PIN_P01_WELL_WATER, INPUT);
    pinMode(PIN_P02_RO_PRESSURE, INPUT);
    pinMode(PIN_P05_TOTEM2_MAX, INPUT_PULLUP);
    pinMode(PIN_P06_TOTEM1_MIN, INPUT_PULLUP);
    pinMode(PIN_P07_BARREL_PRESSURE, INPUT_PULLUP);
    pinMode(PIN_P08_FLOW_METER, INPUT_PULLUP);
#endif

    attachInterrupt(digitalPinToInterrupt(PIN_P08_FLOW_METER), isrFlowSensor, RISING);
    lastFlowCalcTime = millis();
}

void SensorsManager::handleFlowPulse() {
    isrPulseDelta++;
    isrTotalPulses++;
}

void SensorsManager::update() {
    unsigned long now = millis();

    // ------------------------------------------------------------------------
    // 1. [P-01] Sensor de nivel / presencia de agua en Pozo
    // ------------------------------------------------------------------------
#if !WELL_LEVEL_SENSOR_INSTALLED
    // Sin sensor físico instalado en el pozo actualmente: marcar siempre presencia de agua
    data.wellWaterPresent = true;
    data.wellRawADC = 1023;
#else
#if defined(ESP8266)
    // En ESP8266MOD D0 es entrada digital; A0 se reserva para el presostato
    bool rawP01 = (digitalRead(PIN_P01_WELL_WATER) == P01_WATER_PRESENT_LOGIC);
    data.wellRawADC = rawP01 ? 1023 : 0;
#else
    data.wellRawADC = analogRead(PIN_P01_WELL_WATER);
    bool rawP01 = (digitalRead(PIN_P01_WELL_WATER) == P01_WATER_PRESENT_LOGIC);
    if (data.wellRawADC > 1800) {
        rawP01 = true;
    }
#endif

    if (rawP01 != lastStateP01) {
        lastDebounceTimeP01 = now;
        lastStateP01 = rawP01;
    }
    if ((now - lastDebounceTimeP01) >= DEBOUNCE_DELAY_MS) {
        data.wellWaterPresent = rawP01;
    }
#endif

    // ------------------------------------------------------------------------
    // 2. [P-02] Presostato principal de Ósmosis (60 - 80 PSI)
    // ------------------------------------------------------------------------
    data.roPressurePSI = readROPressurePSI();
    data.roPressureBelow60 = (data.roPressurePSI < RO_PRESSURE_START_PSI);
    data.roPressureCutoff80 = (data.roPressurePSI >= RO_PRESSURE_STOP_PSI);

    // ------------------------------------------------------------------------
    // 3. [P-05] Sensor de nivel máximo Tótem 2 (Tanque superior)
    // ------------------------------------------------------------------------
    bool rawP05 = (digitalRead(PIN_P05_TOTEM2_MAX) == P05_TANK_FULL_LOGIC);
    if (rawP05 != lastStateP05) {
        lastDebounceTimeP05 = now;
        lastStateP05 = rawP05;
    }
    if ((now - lastDebounceTimeP05) >= DEBOUNCE_DELAY_MS) {
        data.totem2Full = rawP05;
    }

    // ------------------------------------------------------------------------
    // 4. [P-06] Sensor de nivel mínimo Tótem 1 (Tanque inferior >= 20 Litros)
    // ------------------------------------------------------------------------
    bool rawP06 = (digitalRead(PIN_P06_TOTEM1_MIN) == P06_LEVEL_OK_LOGIC);
    if (rawP06 != lastStateP06) {
        lastDebounceTimeP06 = now;
        lastStateP06 = rawP06;
    }
    if ((now - lastDebounceTimeP06) >= DEBOUNCE_DELAY_MS) {
        data.totem1HasMin20L = rawP06;
    }

    // ------------------------------------------------------------------------
    // 5. [P-07] Presostato auxiliar barril presurizado (Corte a 60 PSI)
    // ------------------------------------------------------------------------
    bool rawP07 = (digitalRead(PIN_P07_BARREL_PRESSURE) == P07_CUTOFF_LOGIC);
    if (rawP07 != lastStateP07) {
        lastDebounceTimeP07 = now;
        lastStateP07 = rawP07;
    }
    if ((now - lastDebounceTimeP07) >= DEBOUNCE_DELAY_MS) {
        data.barrelCutoffReached = rawP07;
    }

    // ------------------------------------------------------------------------
    // 6. [P-08] Caudalímetro (Cálculo no bloqueante de L/min y acumulados)
    // ------------------------------------------------------------------------
    if (now - lastFlowCalcTime >= 1000) {
        noInterrupts();
        unsigned long pulsesInInterval = isrPulseDelta;
        isrPulseDelta = 0;
        data.rawPulseCount = isrTotalPulses;
        interrupts();

        float secondsElapsed = (float)(now - lastFlowCalcTime) / 1000.0f;
        lastFlowCalcTime = now;

        // Frecuencia instantánea (Hz = pulsos/segundo)
        float pulseFrequency = (float)pulsesInInterval / secondsElapsed;

        // Caudal en Litros por Minuto: Q (L/min) = Frecuencia (Hz) / FACTOR
        data.flowRateLPM = pulseFrequency / FLOW_CALIBRATION_FACTOR;
        if (data.flowRateLPM < 0.05f) {
            data.flowRateLPM = 0.0f;
        }

        // Volumen en este intervalo
        float litersDelta = (data.flowRateLPM / 60.0f) * secondsElapsed;
        data.totalLiters += litersDelta;
        data.batchLiters += litersDelta;
    }
}

float SensorsManager::readROPressurePSI() {
#if P02_USE_ANALOG_TRANSDUCER
    long adcSum = 0;
    for (int i = 0; i < 8; i++) {
        adcSum += analogRead(PIN_P02_RO_PRESSURE);
        delayMicroseconds(50);
    }
    float rawADC = (float)adcSum / 8.0f;

#if defined(ESP8266)
    // ADC de 10 bits en ESP8266 (0 - 1023)
    float voltage = (rawADC / 1023.0f) * 3.3f;
#else
    // ADC de 12 bits en ESP32 (0 - 4095)
    float voltage = (rawADC / 4095.0f) * 3.3f;
#endif

    if (voltage < PRESSURE_SENSOR_VMIN) {
        return 0.0f;
    }
    float psi = ((voltage - PRESSURE_SENSOR_VMIN) / (PRESSURE_SENSOR_VMAX - PRESSURE_SENSOR_VMIN)) * PRESSURE_SENSOR_MAX_PSI;
    if (psi > PRESSURE_SENSOR_MAX_PSI) psi = PRESSURE_SENSOR_MAX_PSI;
    return psi;
#else
    bool isLowPressure = (digitalRead(PIN_P02_RO_PRESSURE) == LOW);
    return isLowPressure ? 50.0f : 85.0f;
#endif
}

void SensorsManager::resetBatchVolume() {
    data.batchLiters = 0.0f;
}

void SensorsManager::resetTotalVolume() {
    data.totalLiters = 0.0f;
    noInterrupts();
    isrTotalPulses = 0;
    interrupts();
    data.rawPulseCount = 0;
}

SensorData SensorsManager::getData() {
    return data;
}
