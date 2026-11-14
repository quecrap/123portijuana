#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================================================
// 1. CONFIGURACIÓN DE RED WI-FI (MODO AP + STA CON OTA)
// ============================================================================
#define WIFI_AP_SSID        "AguaDeGia_Lavadora"
#define WIFI_AP_PASS        "agua_resiliente"
#define WIFI_AP_IP_A        192
#define WIFI_AP_IP_B        168
#define WIFI_AP_IP_C        4
#define WIFI_AP_IP_D        1

struct WifiCredential {
  const char* ssid;
  const char* password;
};

static const WifiCredential WIFI_STA_LIST[] = {
  { "Lacasita",            "6646802560" },
  { "la última compuerta", "6646802560" },
  { "la ultima compuerta", "6646802560" }
};
static const uint8_t WIFI_STA_COUNT = sizeof(WIFI_STA_LIST) / sizeof(WIFI_STA_LIST[0]);

// ============================================================================
// 2. MAPA DE PINES OFICIAL ESP-32S — NODO 2 ("AGUA DE GIA")
//    Especificaciones Técnicas Oficiales (SSR Fotek + Relé Lógico + Bixolon)
// ============================================================================

// --- Entradas Analógicas (ADC1 - 12 bits: 0..4095, compatibles con Wi-Fi) ---
static const uint8_t PIN_PT1_NIVEL_CONO    = 32; // GPIO 32 (ADC1_CH4): Transductor de Presión (0.5V - 4.5V)
static const uint8_t PIN_CT1_CELDA_E1      = 34; // GPIO 34 (ADC1_CH6): Sensor de Corriente #1 (Corriente AC en entrada 120V de fuente ATX)
static const uint8_t PIN_CT2_MOTORES_DC    = 35; // GPIO 35 (ADC1_CH7): Sensor de Corriente #2 (Corriente de bombas de diafragma DC)
static const uint8_t PIN_ANODO_PASIVO_VP   = 36; // GPIO 36 (VP / ADC1_CH0): Ánodo Pasivo (Varilla de fierro con divisor 2x10kOhm)
static const uint8_t PIN_REMANENTE_ANODO   = 36; // Alias compatible para el módulo de reactor (GPIO 36 / VP)

// --- Entradas Digitales de Instrumentación y Nivel ---
static const uint8_t PIN_TT1_TEMP_DS18B20  = 4;  // GPIO 4:  Sensor Temperatura Agua TT1 (DS18B20 OneWire)
static const uint8_t PIN_FT1_CAUDAL_HALL   = 18; // GPIO 18: Entrada por interrupción (Digital Interrupt) para Caudalímetro Hall
static const uint8_t PIN_LSH01_TOTEM_ALTO  = 19; // GPIO 19: Sensor de nivel Tótem 1 (INPUT_PULLUP)
static const uint8_t PIN_LSHH02_REBOSE     = 21; // GPIO 21: Sensor de nivel Rebose de tapa (INPUT_PULLUP)
static const uint8_t PIN_PSL03_DEMANDA_RO  = 22; // GPIO 22: Sensor de nivel / Presostato Barril RO 20L (INPUT_PULLUP)

// --- Salidas de Potencia: SSR Fotek, Relés Lógicos y Etapas MOSFET Bixolon ---
static const uint8_t PIN_SSR1_CELDA_40A    = 33; // GPIO 33: SSR Fotek #1 (Fuente ATX 15A - Electrofloculación)
static const uint8_t PIN_K2_SOL_LLENADO    = 27; // GPIO 27: SSR Fotek #2 (Solenoide de Bronce 3/4" 120V AC - Llenado)
static const uint8_t PIN_K3_WHIRLPOOL_M2   = 26; // GPIO 26: Relé Lógico Canal #1 (Bomba Pecera 120V AC - Whirlpool / Lavado)
static const uint8_t PIN_K6_SOL_CASA_60L   = 15; // GPIO 15: Relé Lógico Canal #2 (Solenoide de Bronce 3/4" 120V AC - Casa)
static const uint8_t PIN_K5_MANIFOLD_UV    = 23; // GPIO 23: Relé Lógico Canal #3 (Bomba de Diafragma #2 Manifold 12V DC)
static const uint8_t PIN_K7_SOL_PRE_RO     = 25; // GPIO 25: Bixolon #1 - MOSFET 1 (Solenoide 1/2" Pre-RO 12V DC)
static const uint8_t PIN_K8_SOL_BYPASS_RO  = 13; // GPIO 13: Bixolon #1 - MOSFET 2 (Solenoide 1/2" Bypass RO 12V DC)
static const uint8_t PIN_K1_BOMBA_POZO_M1  = 14; // GPIO 14: Bixolon #2 - MOSFET 2 (Bomba Evans 24V DC / Timer ASL-52)
static const uint8_t PIN_K4_TRASVASE_M3    = 5;  // GPIO 5:  Bixolon #2 - MOSFET 1 (Relé Automotriz 30A / Bomba Diafragma #1 Trasvase)

// --- Actuador PWM y Alarma Acústica ---
static const uint8_t PIN_SRV1_AGUJA_PWM    = 16; // GPIO 16 (PWM 50Hz): Servomotor MG996R (Válvula de aguja de rechazo en Ósmosis Inversa)
static const uint8_t PIN_BZ1_CAMPANA       = 17; // GPIO 17 (Digital):  Buzzer / Alarma acústica de panel

// ============================================================================
// 3. IDENTIFICADORES DE CANALES DE SALIDA (10 CANALES OFICIALES)
// ============================================================================
enum ActuatorId : uint8_t {
  ACT_SSR1_40A   = 0, // GPIO 33 - SSR Fotek #1 (Fuente ATX 15A - Electrofloculación)
  ACT_K1_M1      = 1, // GPIO 14 - Bixolon #2 MOSFET 2 (Bomba Evans 24V DC / Timer ASL-52)
  ACT_K2_YV1     = 2, // GPIO 27 - SSR Fotek #2 (Solenoide Bronce 3/4" 120V AC - Llenado)
  ACT_K3_M2      = 3, // GPIO 26 - Relé Lógico Canal #1 (Bomba Pecera 120V AC - Whirlpool / Lavado)
  ACT_K4_M3      = 4, // GPIO 5  - Bixolon #2 MOSFET 1 (Relé Automotriz 30A / Bomba Diafragma #1 Trasvase)
  ACT_K5_M4_UV1  = 5, // GPIO 23 - Relé Lógico Canal #3 (Bomba de Diafragma #2 Manifold 12V DC)
  ACT_K6_YV2     = 6, // GPIO 15 - Relé Lógico Canal #2 (Solenoide Bronce 3/4" 120V AC - Casa)
  ACT_K7_YV3     = 7, // GPIO 25 - Bixolon #1 MOSFET 1 (Solenoide 1/2" Pre-RO 12V DC)
  ACT_K8_YV4     = 8, // GPIO 13 - Bixolon #1 MOSFET 2 (Solenoide 1/2" Bypass RO 12V DC)
  ACT_BZ1_BELL   = 9, // GPIO 17 - Buzzer / Alarma acústica de panel
  ACT_COUNT      = 10
};

// ============================================================================
// 4. CONSTANTES FÍSICAS, ELECTROQUÍMICAS Y DE CONTROL POR CULOMBIOS
// ============================================================================
// Masa molar del Hierro (Fe) = 55.845 g/mol, Valencia z = 2, F = 96485.33 C/mol
// Tasa teórica de Faraday = 55.845 / (2 * 96485.33) = 0.000289396 g/(A*s)
static const float FARADAY_FE_G_PER_AMP_SEC   = 0.0002894f;

// Meta oficial de disolución de Hierro para 1 BBL (117.3 L): 7.60 g Fe (~26,262 C, ~29-35 min a 15A)
static const float META_FE_GRAMOS_117L        = 7.60f;
static const float VOLUMEN_NOMINAL_1BBL_L     = 117.3f;

// Parámetros eléctricos de la Fuente ATX 15A y sensores
static const float VOLTAJE_FUENTE_CELDA_V     = 12.0f;    // Fuente ATX 12V DC nominal (15A máx)
static const float CORRIENTE_NOMINAL_ATX_DC_A = 15.0f;    // Corriente nominal DC en electrofloculación (15A)
static const float FACTOR_AC_A_DC_FUENTE_ATX  = 8.0f;     // Conversión I_AC (120V) -> I_DC (12V, ~80% eficiencia: 1.875A AC = 15A DC)
static const float K_CELDA_PPM_OHMS           = 12000.0f; // Constante geométrica de celda (PPM * Ohms)
static const float CORRIENTE_TOQUE_ANODO_A    = 0.25f;    // Umbral de detección de agua en ánodo (A)
static const float CORRIENTE_MAX_LLENADO_A    = 15.5f;    // Límite de corriente DC en llenado por etapas (< 15.5A)
static const float CORRIENTE_HISTERESIS_A     = 13.5f;    // Rearranque de válvula YV1 al bajar corriente (A)
static const float CORRIENTE_DEADHEAD_DC_A    = 6.8f;     // Umbral de atasco / dead-head en bombas de diafragma DC (CT2)

// Divisor de tensión en GPIO 34 (Pin 5 - Electrodo Activo 12V con resistencia extra de 10k en serie):
// Calibración con 10k extra añadida a la rama superior: 3.48 + 1.0 = factor 4.48 (nominal 4x10k:1x10k = 5.0)
static const float FACTOR_DIVISOR_CELDA_P34   = 4.48f;    // Factor con resistencia de 10k añadida en serie
static const float R_INTERNA_FUENTE_ATX_OHMS  = 0.20f;    // Resistencia interna fuente + cables (caída 12.2V -> 9.5V @ ~13.5A)
static const float V_VACIO_FUENTE_ATX_V       = 12.2f;    // Voltaje en circuito abierto de la fuente ATX

// Protección de Bomba de Pecera (Whirlpool GPIO 26):
// Solo se enciende cuando el nivel alcanza el último tercio del tanque (>= 66.7%) para cebado por gravedad desde espiga 1/8"
static const float NIVEL_CEBADO_WHIRLPOOL_PCT = 66.7f;

// Muestreo periódico del Ánodo Pasivo (GPIO 36 / VP con divisor 2x10kOhm -> factor 2.0):
// Cada 60 s de electrólisis se apaga la fuente ATX (P33 = OFF) durante 1.5 s, se lee GPIO 36 y se guarda en LittleFS CSV
static const uint32_t PERIODO_MUESTREO_ANODO_MS = 60000UL; // 60 segundos de electrólisis entre lecturas
static const uint32_t PAUSA_SEGURIDAD_ANODO_MS  = 1500UL;  // 1.5 segundos de pausa con fuente ATX apagada (P33 = OFF)
static const uint32_t INSTANTE_LECTURA_VP_MS    = 1200UL;  // Instante de lectura ADC dentro de la pausa de 1.5s
static const float    FACTOR_DIVISOR_VP_2X10K   = 1.0f;    // Conexión directa en GPIO 36 (VP) sin divisor
static const char*    PASSIVE_ANODE_CSV_PATH    = "/anodo_pasivo.csv";

// Transductor de Presión PT1 en GPIO 32 (0.5V - 4.5V, 0 - 100 PSI)
// Geometría real medida en campo: Altura total = 110 cm (Cono inferior = 35 cm + Cilindro = 75 cm)
static const float TANK_TOTAL_HEIGHT_CM       = 110.0f;   // 1.10 m desde el vértice del cono hasta arriba
static const float TANK_CYL_HEIGHT_CM         = 75.0f;    // 75 cm de sección cilíndrica recta
static const float TANK_CONE_HEIGHT_CM        = 35.0f;    // 35 cm de cono inferior (110 cm - 75 cm)
static const float TANK_CYL_LITERS_PER_CM     = 1.35346f; // 117.3 L / (75 + 35/3) cm = 1.35346 L/cm (Ø int = 41.52 cm)
static const float TANK_CONE_VOLUME_LITERS    = 15.7904f; // Volumen exacto del cono (0 a 35 cm) = 15.79 L

// Sensibilidad teórica del transductor ratiométrico 0-100 PSI (0.5V a 4.5V -> 40 mV/PSI):
// 1 cm H2O = 0.0142233 PSI -> 40 mV/PSI * 0.0142233 PSI/cm = 0.56893 mV/cm H2O
// Rango en tanque de 110 cm: 0 cm = 500.00 mV -> 110 cm (1.5646 PSI) = 562.58 mV (Delta = 62.58 mV)
// Calibración adaptada al divisor de voltaje en P32 (cero real ~152 mV):
static const float PT1_DEFAULT_ZERO_MV        = 152.00f;  // Salida base a 0 cm de agua con divisor (~152 mV)
static const float PT1_DEFAULT_MV_PER_CM      = 0.17300f; // mV por cm con divisor (~0.173 mV/cm)
static const float PT1_PSI_PER_CM_H2O         = 0.0142233f; // PSI por cm de columna de agua
static const float PT1_VOLTS_ZERO_05V         = 0.50f;    // Salida base a 0 PSI (0.5V)
static const float PT1_VOLTS_SPAN             = 2.80f;    // Rango máximo seguro en ADC1 (0.5V a 3.3V)

// Volúmenes de "Tamaño de Carga" de la Lavadora (Litros)
enum LoadSizeMode : uint8_t {
  LOAD_SMALL_40L  = 0, // Carga Chica: 40 L
  LOAD_MEDIUM_80L = 1, // Carga Media: 80 L
  LOAD_FULL_117L  = 2  // Tina Llena: 1 BBL / 117.3 L
};

static const float LOAD_VOLUME_LITERS[3] = { 40.0f, 80.0f, 117.3f };

// Tipos de "Jabón Eléctrico" (Dosificación de Hierro Fe2+ por Culombios)
enum SoapTypeMode : uint8_t {
  SOAP_AUTO_MAGIC   = 0, // Meta Oficial 1 BBL: 7.60 g Fe (escalado por volumen)
  SOAP_DELICATE_2G5 = 1, // Agua Ligera (Meta base: 3.80 g Fe)
  SOAP_NORMAL_4G0   = 2, // Agua Media (Meta base: 5.70 g Fe)
  SOAP_HEAVY_5G86   = 3  // Agua Dura / Estándar Oficial Nodo 2 (Meta exacta: 7.60 g Fe)
};

// ============================================================================
// 5. POSICIONES DE LA PERILLA CENTRAL DE LA LAVADORA (7 ETAPAS)
// ============================================================================
enum WasherKnobPos : uint8_t {
  POS_1_APAGADO             = 1, // 12 en punto: APAGADO / LISTA PARA LAVAR (Standby)
  POS_2_LLENANDO_TINA       = 2, // 2 en punto:  LLENANDO TINA (Llenado + Cebado Whirlpool al último tercio)
  POS_3_TALLADO_JABON       = 3, // 4 en punto:  TALLADO Y JABÓN ELÉCTRICO (Integrador Faraday 7.60g Fe + Pausa 1.5s/60s VP)
  POS_4_REMOJO_CALMA        = 4, // 6 en punto:  REMOJO EN CALMA (Sedimentación + Campana Ding!)
  POS_5_ENJUAGUE_FILTROS    = 5, // 8 en punto:  ENJUAGUE TRIPLE FILTRO (Bomba Trasvase GPIO 5 hasta FT1 = 0 L/min x 10s)
  POS_6_EXPRIMIDO_CASA_60   = 6, // 10 en punto: EXPRIMIDO CASA (60 PSI - Solenoide Casa GPIO 15 + Bomba Manifold GPIO 23)
  POS_7_SECADO_SOL_RO_100   = 7  // 11 en punto: SECADO AL SOL Y TENDIDO (100 PSI RO: Pre-RO GPIO 25 + Bypass GPIO 13 + Servo GPIO 16)
};

// Ángulos visuales del dial (0° = 12 en punto)
static const uint16_t KNOB_ANGLE_DEG[8] = {
  0,   // Índice 0 (no usado)
  0,   // Posición 1: 12 en punto (0°)
  60,  // Posición 2: 2 en punto  (60°)
  120, // Posición 3: 4 en punto  (120°)
  180, // Posición 4: 6 en punto  (180°)
  240, // Posición 5: 8 en punto  (240°)
  300, // Posición 6: 10 en punto (300°)
  330  // Posición 7: 11 en punto (330°)
};

// Tiempos de proceso configurables
static const uint32_t TIEMPO_REMOJO_SEGUNDOS_DEF    = 2700;  // 45 min (2700 s) de sedimentación estándar
static const uint32_t TIEMPO_FLUJO_CERO_FIN_MS      = 10000; // 10 s con FT1 = 0 L/min para dar por vaciado el cono
static const uint32_t TIEMPO_EXPRIMIDO_CASA_MS      = 45000; // 45 s de presurización de barril casa 60L
static const uint32_t TIEMPO_FLUSHING_BYPASS_RO_MS  = 15000; // 15 s de lavado de membrana RO con YV4 abierta
static const uint32_t TIEMPO_MAX_SECADO_RO_MS       = 90000; // 90 s máximo en etapa RO antes de concluir o PSL-03

// Ángulos calibrados del Servo SRV1 (MG996R - Válvula de Aguja RO)
static const int SERVO_ANGULO_FLUSHING_ABIERTO      = 20;    // Abierto para arrastrar salmuera en lavado
static const int SERVO_ANGULO_RO_100_PSI            = 135;   // Restricción calibrada para levantar 100 PSI
static const int SERVO_ANGULO_REPOSO                = 90;    // Posición neutral

#endif // CONFIG_H
