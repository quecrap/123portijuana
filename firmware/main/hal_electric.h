#ifndef HAL_ELECTRIC_H
#define HAL_ELECTRIC_H

#include <Arduino.h>
#include "config.h"

// ============================================================================
// ESTRUCTURA DE CANAL DE POTENCIA / SSR FOTEK / RELÉ LÓGICO / BIXOLON MOSFET
// ============================================================================
struct ActuatorChannel {
  uint8_t     id;           // Índice 0..9 (ActuatorId)
  uint8_t     pin;          // GPIO físico en ESP-32S
  const char* iecCode;      // Código oficial (ej. "SSR1 Fotek", "RL1", "BIX1-M1")
  const char* boardLocation;// Etapa de potencia (ej. "SSR Fotek #1", "Bixolon #1 · MOSFET 1")
  const char* loadDesc;     // Carga controlada según inventario oficial Nodo 2
  bool        activeLow;    // true = Activo en LOW (0V), false = Activo en HIGH (3.3V)
  bool        state;        // Estado lógico actual (true = ENCENDIDO, false = APAGADO)
  uint32_t    pulseEndMs;   // Temporizador para pulso de 3s (0 = sin pulso activo)
};

// ============================================================================
// ESTRUCTURA DEL MULTÍMETRO EN VIVO E INSTRUMENTACIÓN DE CAMPO (NODO 2)
// ============================================================================
struct MultimeterTelemetry {
  // CT1 (GPIO 34 - ADC1 / Pin 5): Divisor de Tensión 4:1 en Electrodo Activo 12V
  uint16_t ct1Raw;
  float    ct1Volts;        // Voltaje medido en el pin GPIO 34 (0 - 3.3V)
  float    cellVolts;       // Voltaje real en el electrodo activo (ct1Volts * FACTOR_DIVISOR_CELDA_P34)
  float    ct1AcAmps;       // Corriente AC RMS o fluctuación
  float    ct1Amps;         // Corriente DC calculada en la celda (I_DC)
  float    ct1PpmInstant;
  float    ct1FeGramPerSec;

  // CT2 (GPIO 35 - ADC1): Sensor Corriente #2 (Bombas de diafragma DC)
  uint16_t ct2Raw;
  float    ct2Volts;
  float    ct2Amps;
  bool     ct2DeadHead;

  // PT1 (GPIO 32 / ADC1_CH4, Alta Resolución ADC_0db 256x): Transductor 0-100 PSI (0.5V - 4.5V)
  uint8_t  pt1Pin;          // Pin ADC1 activo (32 por defecto, o 36 si se conectó en VP)
  uint16_t pt1Raw;          // Valor ADC crudo entero (0 - 4095 en escala ADC_0db)
  float    pt1RawFloat;     // Valor ADC crudo con decimales por sobremuestreo 256x
  float    pt1Mv;           // Voltaje en milivoltios con resolución sub-mV (ej. 500.00 - 562.58 mV)
  float    pt1Volts;        // Voltaje en voltios (pt1Mv / 1000.0)
  float    pt1Psi;          // Presión hidrostática en PSI (0.000 - 1.565 PSI para 0 - 110 cm)
  float    pt1HeightCm;     // Altura de columna de agua en cm (0.0 - 110.0 cm; cono=35cm + cilindro=75cm)
  float    pt1LevelPct;     // Nivel equivalente en altura del tanque (0.0 - 100.0% sobre 110 cm)
  float    pt1Liters;       // Volumen real en Litros (Cono 35cm = 15.79L + Cilindro 75cm = 101.51L -> 117.3L)
  float    pt1ZeroMv;       // Voltaje calibrado en vacío (0 cm), por defecto 500.00 mV
  float    pt1MvPerCm;      // Sensibilidad calibrada (mV/cm de agua), por defecto 0.56893 mV/cm
  bool     pt1Connected;    // true si se detecta señal analógica (>250 mV); false si sigue en GPIO 22 digital

  // VP / Ánodo Pasivo (GPIO 36 - ADC1_CH0): Varilla de fierro con divisor 2x10kOhm (~0.8V base)
  uint16_t passiveRaw;
  float    passivePinVolts; // Voltaje en el pin GPIO 36 (0 - 3.3V)
  float    passiveVolts;    // Potencial galvánico real tras divisor 2x10kOhm (~0.8V base)
  float    lastSampledPassiveVolts; // Última lectura tomada en pausa de 1.5s con P33 = OFF
  uint32_t passiveSampleCount;      // Total de muestras guardadas en LittleFS CSV
  bool     littleFsReady;           // true si LittleFS está montado y operativo

  // TT1 (GPIO 4): Temperatura del Agua en °C (DS18B20 OneWire)
  float    tt1TempC;

  // FT1 (GPIO 18): Caudalímetro Hall (Pulsos, L/min, Totalizador Litros)
  uint32_t ft1Pulses;
  float    ft1FlowLpm;
  float    ft1TotalLiters;

  // Sensores Digitales de Nivel (GPIO 19 / 21 / 22 - INPUT_PULLUP: LOW = Cerrado/Activo)
  bool     lsh01Closed;     // GPIO 19: Nivel Tótem 1
  bool     lshh02Closed;    // GPIO 21: Rebose de tapa (paro/último tercio)
  bool     psl03Closed;     // GPIO 22: Nivel / Presostato Barril RO 20L

  // Posición actual del Servomotor MG996R (GPIO 16)
  int      srv1AngleDeg;
};

// ============================================================================
// API PÚBLICA DE LA CAPA DE ABSTRACCIÓN ELÉCTRICA (HAL)
// ============================================================================
void halInit();
void halUpdate();

// Control de actuadores, SSRs, relés lógicos, MOSFETs Bixolon y buzzer
void halSetActuator(uint8_t id, bool turnOn);
bool halIsActuatorOn(uint8_t id);
void halPulseActuator(uint8_t id, uint32_t durationMs);
void halSetActuatorPolarity(uint8_t id, bool activeLow);
void halTurnOffAllActuators();
void halRingCycleEndBell();

// Control PWM de Válvula de Aguja RO (Servomotor MG996R - GPIO 16)
void halSetServoAngle(int angleDeg);
int  halGetServoAngle();

// Control y Calibración de Sensor de Corriente #1 y Divisor de Tensión (CT1 - GPIO 34)
void  halSetElectrodeMeasurement(bool enabled);
bool  halGetElectrodeMeasurement();
void  halTareElectrodeZero();
float halGetElectrodeZeroVolts();
void  halSetCellDividerFactor(float factor);
float halGetCellDividerFactor();

// Control y Calibración de Alta Resolución del Presostato / Transductor PT1 (100 PSI en Tanque 110 cm)
void  halTarePt1Zero();
bool  halCalibratePt1AtHeightCm(float waterHeightCm);
void  halSetPt1Calibration(float zeroMv, float mvPerCm);
void  halResetPt1Calibration();
float halVolumeFromHeightCm(float heightCm);

// Muestreo de Ánodo Pasivo en GPIO 36 (VP) y Registro CSV en LittleFS
float halSampleAndLogPassiveAnode(uint32_t stageElapsedSec, float coulombs, float feGrams);
bool  halClearPassiveAnodeCsv();

// Caudalímetro Hall FT1 (GPIO 18)
void halResetFlowTotalizer();

// Acceso a tablas de estado y telemetría
const ActuatorChannel*     halGetActuatorTable();
const MultimeterTelemetry& halGetTelemetry();

#endif // HAL_ELECTRIC_H
