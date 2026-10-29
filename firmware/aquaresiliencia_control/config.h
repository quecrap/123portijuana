/**
 * ============================================================================
 * PROYECTO: AquaResiliencia Tijuana — Sistema Automatizado de Gestión Hídrica
 * HARDWARE: ESP8266 (ESP8266MOD / NodeMCU / Generic ESP8266) y ESP32 DevKit
 * ARCHIVO:  config.h
 * ============================================================================
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================================================
// 1. VELOCIDAD SERIAL USB (CH340G @ 9600 BAUDIOS)
// ============================================================================
#define SERIAL_BAUD_RATE            9600

// ============================================================================
// 2. DEFINICIÓN UNIVERSAL DE PINES NODEMCU / WEMOS PARA CUALQUIER ESP8266
// (Evita errores si se compila con "Generic ESP8266 Module" en Arduino IDE)
// ============================================================================
#if defined(ESP8266)
  #ifndef D0
    #define D0 16
  #endif
  #ifndef D1
    #define D1 5
  #endif
  #ifndef D2
    #define D2 4
  #endif
  #ifndef D3
    #define D3 0
  #endif
  #ifndef D4
    #define D4 2
  #endif
  #ifndef D5
    #define D5 14
  #endif
  #ifndef D6
    #define D6 12
  #endif
  #ifndef D7
    #define D7 13
  #endif
  #ifndef D8
    #define D8 15
  #endif
  #ifndef D9
    #define D9 3
  #endif
  #ifndef D10
    #define D10 1
  #endif

  // --------------------------------------------------------------------------
  // ASIGNACIÓN DE PINES HARDWARE ESP8266 (Numeración GPIO directa y segura)
  // --------------------------------------------------------------------------
  // ACTUADORES: D1 (5), D2 (4), D5 (14), D6 (12) -> Pines 100% seguros al arranque
  #define PIN_RELAY_1_PUMP          5    // Pin serigrafiado D1 (GPIO 5)  -> Relé 1: Bomba Pozo
  #define PIN_RELAY_2_OSMOSIS       4    // Pin serigrafiado D2 (GPIO 4)  -> Relé 2: Solenoide Ósmosis
  #define PIN_RELAY_3_GRAVITY       14   // Pin serigrafiado D5 (GPIO 14) -> Relé 3: Solenoide Gravedad
  #define PIN_RELAY_4_AUX           12   // Pin serigrafiado D6 (GPIO 12) -> Relé 4: Canal Auxiliar

  // SENSORES E INSTRUMENTACIÓN:
  #define PIN_P08_FLOW_METER        13   // Pin serigrafiado D7 (GPIO 13) -> [P-08] Caudalímetro (ISR)
  #define PIN_P02_RO_PRESSURE       A0   // Pin serigrafiado A0 (ADC0)    -> [P-02] Presostato / Transductor Ósmosis
  #define PIN_P01_WELL_WATER        16   // Pin serigrafiado D0 (GPIO 16) -> [P-01] Nivel / Presencia Agua Pozo
  #define PIN_P05_TOTEM2_MAX        0    // Pin serigrafiado D3 (GPIO 0)  -> [P-05] Nivel Máximo Tótem 2
  #define PIN_P06_TOTEM1_MIN        3    // Pin serigrafiado RX (GPIO 3)  -> [P-06] Nivel Mínimo Tótem 1
  #define PIN_P07_BARREL_PRESSURE   15   // Pin serigrafiado D8 (GPIO 15) -> [P-07] Presostato Auxiliar Barril

  #define PIN_STATUS_LED            2    // Pin serigrafiado D4 (GPIO 2)  -> LED azul integrado ESP8266MOD

#else
  // --------------------------------------------------------------------------
  // ASIGNACIÓN DE PINES PARA ESP32 WROOM-32
  // --------------------------------------------------------------------------
  #define PIN_RELAY_1_PUMP          26
  #define PIN_RELAY_2_OSMOSIS       25
  #define PIN_RELAY_3_GRAVITY       33
  #define PIN_RELAY_4_AUX           32

  #define PIN_P01_WELL_WATER        35
  #define PIN_P02_RO_PRESSURE       34
  #define PIN_P05_TOTEM2_MAX        14
  #define PIN_P06_TOTEM1_MIN        27
  #define PIN_P07_BARREL_PRESSURE   13
  #define PIN_P08_FLOW_METER        18
  #define PIN_STATUS_LED            2
#endif

// ============================================================================
// 3. PARÁMETROS HIDRÁULICOS Y CALIBRACIÓN
// ============================================================================
// Sensor de pozo [P-01]:
#define WELL_LEVEL_SENSOR_INSTALLED false // false = Sin sensor en el pozo por ahora (no bloquea bomba)
#define P01_WATER_PRESENT_LOGIC     LOW   // LOW = Boya detecta agua en pozo (si se instala)

// Presostato Único Mecánico (Cople D del Barril de Cerveza Sanke D @ 40 PSI):
#define USE_SINGLE_PRESSURE_SWITCH  true  // true = 1 solo presostato en el barril Sanke D
#define P02_USE_ANALOG_TRANSDUCER   false // false = Presostato digital en barril

#define RO_PRESSURE_START_PSI       22.0f // Re-arranque al caer de 22 PSI
#define RO_PRESSURE_STOP_PSI        40.0f // Corte seguro a 40 PSI de aire comprimido

// Transductor de presión (si se usara en el futuro)
#define PRESSURE_SENSOR_VMIN        0.36f
#define PRESSURE_SENSOR_VMAX        3.10f
#define PRESSURE_SENSOR_MAX_PSI     100.0f

#define P05_TANK_FULL_LOGIC         LOW  // LOW = Tótem 2 (Superior) lleno al 100% (corta bomba pozo)
#define P06_LEVEL_OK_LOGIC          LOW  // LOW = Tótem 1 (Inferior) tiene >= 20 Litros
#define P07_CUTOFF_LOGIC            LOW  // LOW = Presostato en Cople D alcanzó 40 PSI (corta bomba Evans)

// Caudalímetro YF-S201
#define FLOW_CALIBRATION_FACTOR     7.5f // Pulsos por segundo por L/min

// ============================================================================
// 4. POLARIDAD DEL MÓDULO DE 4 RELÉS (SRD-05VDC)
// ============================================================================
#define RELAYS_ACTIVE_LOW           true // true = LOW activa relé (estándar optoacoplado)

#if RELAYS_ACTIVE_LOW
  #define RELAY_STATE_ON            LOW
  #define RELAY_STATE_OFF           HIGH
#else
  #define RELAY_STATE_ON            HIGH
  #define RELAY_STATE_OFF           LOW
#endif

// Histéresis y temporizadores de seguridad
#define DEBOUNCE_DELAY_MS           500UL 
#define DEFAULT_FLOC_TARGET_LITERS  25.0f
#define MIN_FLOC_TARGET_LITERS      1.0f
#define MAX_FLOC_TARGET_LITERS      200.0f
#define WS_BROADCAST_INTERVAL_MS    1000UL

// ============================================================================
// 5. CONFIGURACIÓN WI-FI (ROUTER DOMÉSTICO + AP DE RESPALDO)
// ============================================================================
// Conexión a tu red Wi-Fi local:
#define WIFI_STA_SSID               "la ultima compuerta"
#define WIFI_STA_PASSWORD           "6646802560"

// Red Access Point de respaldo emitida por el ESP8266:
#define WIFI_AP_SSID                "AquaResiliencia_AP"
#define WIFI_AP_PASSWORD            "AquaResiliencia2026"
#define WIFI_AP_CHANNEL             1
#define WIFI_AP_MAX_CLIENTS         4

#define AP_LOCAL_IP_OCTET1          192
#define AP_LOCAL_IP_OCTET2          168
#define AP_LOCAL_IP_OCTET3          4
#define AP_LOCAL_IP_OCTET4          1

#endif // CONFIG_H
