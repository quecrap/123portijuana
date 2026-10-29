#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================================================
// AQUARESILIENCIA TIJUANA — NODO CENTINELA POZO 001
// CONFIGURACIÓN DE PINES Y PARÁMETROS HIDRÁULICOS (ESP32 WROOM-32)
// ============================================================================

// ----------------------------------------------------------------------------
// 1. ACTUADORES (Módulo de Relevadores - Activos en LOW)
// ----------------------------------------------------------------------------
#define PIN_RELAY_PUMP_WELL       23   // Relay 1: Bomba 1 Extracción Pozo (24V DC, 1A)
#define PIN_RELAY_RO_BOOSTER      22   // Relay 2: Bomba Booster Solenoide Ósmosis Inversa (~80 PSI)
#define PIN_RELAY_SOLENOID_120V   21   // Relay 3: Electroválvula de línea (120 VAC)
#define PIN_LED_STATUS            2    // LED onboard ESP32 (Diagnóstico de ciclo activo)

// ----------------------------------------------------------------------------
// 2. SENSORES FÍSICOS PRESENTES EN SITIO
// ----------------------------------------------------------------------------
#define PIN_FLOW_SENSOR           18   // Caudalímetro Steren (Efecto Hall, interrupción flanco RISING)
#define PIN_DHT_DATA              4    // Sensor DHT22 / AM2302 (Temperatura y Humedad de cabina)
#define PIN_ONEWIRE_WATER_TEMP    19   // Sensor digital sumergible DS18B20 (Temperatura agua entrada RO / intercambiador)
#define PIN_MAGNETIC_FLOAT_WELL   33   // Contacto magnético tipo alarma de puerta en pozo (NO, con pull-up)
#define PIN_PRESSURE_SWITCH_T1    32   // Presostato mecánico Línea T1 (Corta a 90 PSI / Arranca a 60 PSI)
#define PIN_PRESSURE_SWITCH_T2    25   // Presostato Tanque T2 Agua Limpia (Corta a 40 PSI / Arranca a 20 PSI)

// ----------------------------------------------------------------------------
// 2.1 PROTECCIÓN TÉRMICA DE MEMBRANA ÓSMOSIS INVERSA
// ----------------------------------------------------------------------------
#define TEMP_RO_MAX_SAFE          32.0f // Temperatura máxima permitida (°C) antes de corte de emergencia
#define TEMP_RO_RESUME_HYST       2.0f  // Histéresis de reconexión segura (rearme cuando baje a <= 30.0 °C)

// ----------------------------------------------------------------------------
// 3. MÓDEM GSM SIM800L (Hardware Serial UART2)
// ----------------------------------------------------------------------------
#define PIN_GSM_RX                16   // UART2 RX (conectar al TX del SIM800L)
#define PIN_GSM_TX                17   // UART2 TX (conectar al RX del SIM800L con divisor 3.3V)

// ----------------------------------------------------------------------------
// 4. PARÁMETROS HIDRÁULICOS Y TEMPORIZADORES DEL POZO
// ----------------------------------------------------------------------------
// Pozo: Nivel freático 6.50m | Fondo 7.30m -> Columna útil: 80 cm (~1.6 L en ademe 2")
// Bomba 24V 1A (~3 L/min): Debe operar en pulsos para no agotar el venero
#define PUMP_WORK_DURATION_MS     (30UL * 1000UL) // 30 segundos de bombeo activo
#define PUMP_REST_DURATION_MS     (60UL * 1000UL) // 60 segundos de descanso para recarga freática

// Factor de pulsos por litro para caudalímetro Steren / YF-S201
#define FLOW_CALIBRATION_FACTOR   7.5f

// ----------------------------------------------------------------------------
// 5. RED WI-FI Y CREDENCIALES WEB
// ----------------------------------------------------------------------------
#define WIFI_AP_SSID              "AquaResiliencia_Pozo001"
#define WIFI_AP_PASSWORD          "agua_resiliente_2026"
#define HTTP_AUTH_USER            "admin"
#define HTTP_AUTH_PASS            "pozo001tijuana"
#define EMERGENCY_PHONE_NUMBER    "+526640000000"

#endif // CONFIG_H
