/**
 * ============================================================================
 * PROYECTO: AquaResiliencia Tijuana · "Agua de Gía"
 * ARQUITECTURA: Sistema Embebido SCADA IoT & Servidor Web Local en Modo Dual
 * REDES: Modo Dual (AP: AguaDeGia_Config + STA: Lacasita / la última compuerta)
 * ACTUALIZACIONES: Inalámbricas OTA (ArduinoOTA + Web OTA /update)
 * CONTROLADOR: NodeMCU ESP32S v1.1 - 38 Pines (USB Tipo-C)
 * VERSIÓN: 4.5 - Noviembre 2026
 * ============================================================================
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <ArduinoOTA.h>
#include <Update.h>
#include <Preferences.h>
#include <esp_arduino_version.h>
#include <driver/gpio.h>
#include "web_page.h"

// ============================================================================
// 1. CONFIGURACIÓN WI-FI DUAL (PUNTO DE ACCESO + REDES DE CASA)
// ============================================================================
// Access Point Local de Respaldo (Siempre activo)
const char* AP_SSID = "AguaDeGia_Config";
const char* AP_PASS = "agua_resiliente";
IPAddress apIP(192, 168, 4, 1);
IPAddress apGateway(192, 168, 4, 1);
IPAddress apSubnet(255, 255, 255, 0);

// Multi-Wi-Fi para conectar al módem de casa
WiFiMulti wifiMulti;
DNSServer dnsServer;

WebServer server(80);

// ============================================================================
// 2. MAPA FÍSICO DE PINES (GPIOs)
// ============================================================================
// Entradas Analógicas (ADC1)
#define PIN_ADC_I_ELECTRODOS  34  // ADC1_CH6: Corriente 40A electrodos
#define PIN_ADC_I_BOMBAS_DC   35  // ADC1_CH7: Corriente bombas DC (Dead-Head)
#define PIN_ADC_PRESOSTATO    32  // ADC1_CH4: Campana hidrostática cono
#define PIN_ADC_ANODO_PASIVO  36  // ADC1_CH0 (VP): Ánodo Pasivo (Varilla Fe 24" con divisor 2x10kOhm)

// Entradas Digitales
#define PIN_TEMP_DS18B20      4   // Sensor DS18B20 OneWire
#define PIN_FLOW_HALL         18  // Caudalímetro Hall (Interrupción)
#define PIN_NIVEL_ALTO_TOTEM  19  // Electrodo Tótem 1 (PULLUP)
#define PIN_PARO_REBOSE_CONO  21  // Paro rebose 2 hilos cobre (PULLUP)
#define PIN_PRESOSTATO_RO     22  // Presostato demanda 20-40 PSI (PULLUP)

// --- SALIDAS DE POTENCIA Y CONTROL (ACTIVE-HIGH: SSRs, MOSFETs BIXOLON Y RELÉS) ---
#define PIN_RELE_1_POZO_EVANS 14  // P14 -> Actuador 1: Bomba Evans Pozo 24V (Bixolon #2 Ch2)
#define PIN_RELE_2_SV_LLENADO 27  // P27 -> Actuador 2: Solenoide Llenado Cono 120VAC (SSR 2)
#define PIN_RELE_3_POTENCIA40 33  // P33 -> Actuador 3: Fuente ATX Electrofloculación (SSR 1)
#define PIN_RELE_4_BOMBA_PEC  26  // P26 -> Actuador 4: Bomba Pecera Whirlpool 120VAC (SSR 3)
#define PIN_RELE_5_BOMBA_TR   25  // P25 -> Actuador 5: Bomba Trasvase 12V vía Relé 30A (Bixolon #2 Ch1)
#define PIN_RELE_6_MANIFOLD_UV 23 // P23 -> Actuador 6: Bomba Manifold + UV
#define PIN_RELE_7_SV_CASA    15  // P15 -> Actuador 7: Solenoide Vía A Casa 12V
#define PIN_RELE_8_SV_RO      5   // P5  -> Actuador 8: Solenoide Vía B Pre-RO 12V (Bixolon #1 Ch1)
#define PIN_RELE_9_SV_LAVADO  13  // P13 -> Actuador 9: Solenoide Bypass Lavado RO 12V (Bixolon #1 Ch2)

// Control PWM y Audio
#define PIN_SERVO_PWM         16  // P16 -> Servo válvula aguja RO (LEDC)
#define PIN_BUZZER            17  // P17 -> Actuador 10: Buzzer Alarma

// ============================================================================
// 3. POLARIDAD DE ACTUADORES (SSRs / MOSFETs ACTIVE-HIGH) Y BUZZER
// ============================================================================
#define RELAY_ACTIVE_LEVEL     HIGH
#define RELAY_INACTIVE_LEVEL   LOW

#define BUZZER_ACTIVE_LEVEL    HIGH
#define BUZZER_INACTIVE_LEVEL  LOW

// ============================================================================
// 4. PARÁMETROS DEL PROCESO Y LEY DE FARADAY
// ============================================================================
const float VOLUMEN_LOTE_L          = 117.3f; // 1 BBL
const float PESO_ATOMICO_FE         = 55.845f;
const float VALENCIA_FE             = 2.0f;   // Fe -> Fe2+ + 2e-
const float CONSTANTE_FARADAY       = 96485.0f;
const float DOSIS_BASE_MG_L         = 50.0f;
const float PPM_REFERENCIA          = 3000.0f;

const float CALIB_40A_AMPS_PER_VOLT = 15.15f;
const float CALIB_DC_AMPS_PER_VOLT  = 3.33f;
const float I_CORTE_CRITICO_40A     = 38.0f;
const float I_CORTE_CRITICO_BOMBAS  = 7.5f;

// ============================================================================
// 5. MÁQUINA DE ESTADOS AUTOMÁTICA
// ============================================================================
enum EstadoProceso {
  ESTADO_STANDBY = 0,
  ESTADO_LLENADO_ADAPTATIVO = 1,
  ESTADO_CALCULO_FARADAY = 2,
  ESTADO_ELECTROFLOCULACION = 3,
  ESTADO_REPOSO_FORZADO = 4,
  ESTADO_TRASVASE_FILTRADO = 5,
  ESTADO_PRESURIZACION_MANIFOLD = 6,
  ESTADO_OSMOSIS_100PSI = 7
};

EstadoProceso estadoActual = ESTADO_STANDBY;
bool modoAutomatico = false;
bool paroEmergencia = false;
String motivoEmergencia = "";

float tdsEstimadoPPM      = 3000.0f;
float tdsManualPPM        = 0.0f;     // Medición obligatoria ingresada por el usuario con su medidor
bool  tdsManualIngresado  = false;    // Bandera de validación previa al inicio del lote
float gramosFeMeta        = 5.865f;
float gramosFeDisueltos   = 0.0f;
float pctFaraday          = 0.0f;

float corrienteElectrodosA = 0.0f;
float corrienteAcPrimariaA = 0.0f; // Amperes AC medidos por ACS712-05B en primaria 120V
float corrienteEstimadaDcA = 0.0f; // Amperes DC estimados en electrodos por caída de celda y potencia AC
float corrienteBombasDCA   = 0.0f;
float nivelHidrostaticoPct = 0.0f;
float temperaturaCelsius   = 21.5f;

// ============================================================================
// GEOMETRÍA REAL DEL TANQUE (1.10 m TOTAL = 35 cm CONO + 75 cm CILINDRO)
// Y TRANSDUCTOR DE PRESIÓN PT1 (0-100 PSI, 5V, 0.5V-4.5V RATIOMÉTRICO)
// ============================================================================
const float TANK_TOTAL_HEIGHT_CM   = 110.0f;   // 1.10 m de punta de cono a borde superior
const float TANK_CYL_HEIGHT_CM     = 75.0f;    // 75 cm sección cilíndrica recta
const float TANK_CONE_HEIGHT_CM    = 35.0f;    // 35 cm sección cónica inferior (110 - 75)
const float TANK_CYL_LITERS_PER_CM = 1.35346f; // 101.51 L en 75 cm de cilindro (Ø ~41.52 cm)
const float TANK_CONE_VOLUME_L     = 15.7904f; // V_cono = (1/3) * Area_cil * 35 cm

// Calibración adaptada al divisor de voltaje en P32 (cero real ~152 mV):
const float PT1_DEFAULT_ZERO_MV    = 152.00f;
const float PT1_DEFAULT_MV_PER_CM  = 0.17300f;
const float PT1_PSI_PER_CM_H2O     = 0.0142233f;

Preferences pt1Prefs;
uint8_t  pt1PinActual       = PIN_ADC_PRESOSTATO;
uint16_t pt1Raw             = 0;
float    pt1RawFloat        = 0.0f;
float    pt1Mv              = 0.0f;
float    pt1Volts           = 0.0f;
float    pt1Psi             = 0.0f;
float    pt1HeightCm        = 0.0f;
float    pt1Liters          = 0.0f;
float    pt1ZeroMv          = PT1_DEFAULT_ZERO_MV;
float    pt1MvPerCm         = PT1_DEFAULT_MV_PER_CM;
bool     pt1Connected       = false;
bool     pt1EmaInit         = false;

// Telemetría de Celda y Ánodo Pasivo (GPIO 36 / VP)
// Calibración dinámica NVS para voltaje (1:1, 2:1 o 4:1) y amperaje
Preferences elecPrefs;
float calibVScale            = 2.0f;   // Ratio divisor P36 (2.0f actual con 2 resistencias, 4.0f con 4 resistencias)
int   calibIMode             = 0;      // 0 = Amperaje Nominal Fijo (Exacto y Recomendado), 1 = Sensor ACS712/Sag
float calibINominal          = 11.0f;  // Amperes nominales de celda con SSR 1 (P33) encendido
float calibISagFactor        = 1.0f;   // Factor de calibración fina para modo sensor
float calibDosisMgL          = 80.0f;  // Dosis objetivo Fe en mg/L (80 = Agotamiento Total)
uint32_t calibTiempoMaxMin   = 50;     // Minutos máximos de seguridad en floculación
float calibVolumenL          = 117.3f; // Volumen lote en litros (1 BBL)

// Cronometraje de Faraday y pronóstico en tiempo real
uint32_t tInicioFloculacionMs   = 0;
uint32_t tTranscurridoFlocSeg   = 0;
uint32_t tEstimadoTotalFlocSeg  = 2940; // ~49 min a 11A para 80 mg/L en 117 L
uint32_t tRestanteFlocSeg       = 2940;
float    tasaDisolucionFeGps    = 0.00318f; // ~0.191 g Fe / min a 11A

const float DIVISOR_ANODO_PASIVO = 2.0f; // Ratio base (actualmente 2:1 hasta instalar 4 resistencias)
uint32_t rawAnodoPasivo      = 0;
uint32_t ruidoPpAnodoPasivo  = 0;     // Ruido pico a pico (60Hz) para detectar pin al aire
bool  pinP36AlAire           = true;  // True si P36 no tiene conectado el divisor 10k a GND
float vPinAnodoPasivoV       = 0.0f;  // Voltaje directo en GPIO 36 (0 - 3.5V max)
float vVarillaAnodoPasivoV   = 0.0f;  // Voltaje real en la celda/varilla (vPin * calibVScale)
float vAnodoConFuenteOnV     = 0.0f;  // Último voltaje en varilla con SSR 1 (P33) ENCENDIDO
float vAnodoPausa50msV       = 0.0f;  // Voltaje en varilla a los 50 ms de apagar SSR 1 (P33)
float vAnodoPausa1400msV     = 0.0f;  // Voltaje en varilla a los 1400 ms de apagar SSR 1 (galvánico/remanente)
float deltaVIrOhmicaV        = 0.0f;  // Caída óhmica instantánea: V_ON - V_OFF(50ms)
bool  pausaMuestreoActiva    = false; // True durante la ventana de 1.5s con P33 = OFF
bool  ssr1EstabaEncendido    = false;
bool  tomoMuestra50ms        = false;
bool  tomoMuestra1400ms      = false;
uint32_t tInicioPausaAnodoMs = 0;
uint32_t ultimoMuestreo60sMs = 0;
uint32_t contadorMuestrasP36 = 0;

const char* getNombreEstado(EstadoProceso st) {
  switch (st) {
    case ESTADO_STANDBY: return "Standby";
    case ESTADO_LLENADO_ADAPTATIVO: return "Llenado Cono";
    case ESTADO_CALCULO_FARADAY: return "Faraday";
    case ESTADO_ELECTROFLOCULACION: return "Electrofloculación";
    case ESTADO_REPOSO_FORZADO: return "Reposo 45m";
    case ESTADO_TRASVASE_FILTRADO: return "Trasvase";
    case ESTADO_PRESURIZACION_MANIFOLD: return "Presurización";
    case ESTADO_OSMOSIS_100PSI: return "Ósmosis Inversa";
    default: return "Desconocido";
  }
}

// ============================================================================
// BITÁCORA FORENSE DE CAMPO (CAJA NEGRA) Y PUENTE MÓVIL <-> ANTIGRAVITY
// ============================================================================
struct RegistroBitacora {
  uint32_t id;
  uint32_t uptime_s;
  uint8_t  etapa_id;
  char     etapa_nombre[24];
  bool     ssr1_on;
  float    nivel_cm;
  float    litros;
  float    psi;
  float    v_multimetro;
  float    i_pinza_ac;
  float    v_p36_pin;
  float    v_p36_rod;
  float    v_on;
  float    v_off_50ms;
  float    v_off_1400ms;
  float    dv_ir;
  float    i_dc_est;
  float    i_ac_prim;
  float    tds_ppm;
  char     actuadores_str[24];
  char     nota[200];
};

#define MAX_BITACORA_ROWS 80
RegistroBitacora bitacoraCampo[MAX_BITACORA_ROWS];
uint16_t totalBitacoraRows = 0;
String mensajeAntigravity = "Listo en linea. Pinout actualizado con divisor 4:1 en Pin 03 y ACS712 en AC primaria. Puedes saltar a cualquier etapa o registrar datos.";
uint32_t msgAntigravityTs = 0;

String jsonEscape(const String& s) {
  String out = "";
  for (size_t i = 0; i < s.length(); i++) {
    char c = s[i];
    if (c == '\"') out += "\\\"";
    else if (c == '\\') out += "\\\\";
    else if (c == '\n' || c == '\r') out += " ";
    else out += c;
  }
  return out;
}

volatile uint32_t pulsosHall = 0;
float caudalLpm = 0.0f;
float litrosTotalizados = 0.0f;
uint32_t ultimaMedicionFlujoMs = 0;

uint32_t tiempoInicioEstadoMs = 0;
uint32_t tiempoBarridoPeceraMs = 0;
bool barridoPeceraActivo = false;
uint32_t tiempoPulsoLlenadoMs = 0;
bool solenoideLlenadoPulsando = false;

// ============================================================================
// 6. ESTRUCTURA DE ACTUADORES Y MOTOR DE PULSOS (3 SEGUNDOS)
// ============================================================================
struct Actuador {
  uint8_t id;
  uint8_t pin;
  const char* nombre;
  const char* desc;
  bool esBuzzer;
  bool estado;
  uint32_t pulsoFinMs;
};

#define TOTAL_ACTUADORES 10
Actuador actuadores[TOTAL_ACTUADORES] = {
  {1,  PIN_RELE_1_POZO_EVANS, "Bomba Evans Pozo 24V (Bixolon #2 Ch2)",           "Bomba succión pozo 24V (Bixolon #2 Ch2)", false, false, 0},
  {2,  PIN_RELE_2_SV_LLENADO, "Solenoide Llenado Cono 120VAC (SSR 2)",           "Válvula 120VAC llenado 1 BBL (SSR 2)", false, false, 0},
  {3,  PIN_RELE_3_POTENCIA40, "Fuente ATX Electrofloculación (SSR 1)",           "Ánodo Fe / Cátodo 12V ATX (SSR 1)", false, false, 0},
  {4,  PIN_RELE_4_BOMBA_PEC,  "Bomba Pecera Whirlpool 120VAC (SSR 3)",           "Agitación Whirlpool y barrido H2 (SSR 3)", false, false, 0},
  {5,  PIN_RELE_5_BOMBA_TR,   "Bomba Trasvase 12V vía Relé 30A (Bixolon #2 Ch1)","Post-racking a filtros 12V (Bixolon #2 Ch1)", false, false, 0},
  {6,  PIN_RELE_6_MANIFOLD_UV,"Bomba Manifold + UV",                             "Presurización Manifold 50 PSI + UV", false, false, 0},
  {7,  PIN_RELE_7_SV_CASA,    "Solenoide Vía A Casa 12V",                        "Barril casa 60L (12V DC)", false, false, 0},
  {8,  PIN_RELE_8_SV_RO,      "Solenoide Vía B Pre-RO 12V (Bixolon #1 Ch1)",     "Alimentación membrana RO (Bixolon #1 Ch1)", false, false, 0},
  {9,  PIN_RELE_9_SV_LAVADO,  "Solenoide Bypass Lavado RO 12V (Bixolon #1 Ch2)", "Purga rápida RO 1/2\" (Bixolon #1 Ch2)", false, false, 0},
  {10, PIN_BUZZER,            "Buzzer Alarma",                                   "Zumbador piezoeléctrico de alarma", true, false, 0}
};

// ============================================================================
// 7. ACTUADOR SERVO VÁLVULA DE AGUJA (LEDC NATIVO)
// ============================================================================
#define SERVO_LEDC_FREQ   50
#define SERVO_LEDC_RES    14
#define SERVO_LEDC_CHAN   0
int servoAnguloActual = 90;

void setupServoLEDC() {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  ledcAttach(PIN_SERVO_PWM, SERVO_LEDC_FREQ, SERVO_LEDC_RES);
#else
  ledcSetup(SERVO_LEDC_CHAN, SERVO_LEDC_FREQ, SERVO_LEDC_RES);
  ledcAttachPin(PIN_SERVO_PWM, SERVO_LEDC_CHAN);
#endif
}

void escribirServoAngulo(int angulo) {
  angulo = constrain(angulo, 0, 180);
  servoAnguloActual = angulo;
  uint32_t duty = map(angulo, 0, 180, 409, 2048);

#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  ledcWrite(PIN_SERVO_PWM, duty);
#else
  ledcWrite(SERVO_LEDC_CHAN, duty);
#endif
}

// ============================================================================
// 8. INTERRUPCIÓN DEL CAUDALÍMETRO HALL (GPIO 18)
// ============================================================================
void IRAM_ATTR isrCaudalimetro() {
  pulsosHall++;
}

// ============================================================================
// 9. FUNCIONES DE CONTROL DE RELÉS Y PULSOS
// ============================================================================
void fijarActuador(uint8_t id, bool encender) {
  for (int i = 0; i < TOTAL_ACTUADORES; i++) {
    if (actuadores[i].id == id) {
      actuadores[i].estado = encender;
      actuadores[i].pulsoFinMs = 0;
      if (actuadores[i].esBuzzer) {
        digitalWrite(actuadores[i].pin, encender ? BUZZER_ACTIVE_LEVEL : BUZZER_INACTIVE_LEVEL);
      } else {
        digitalWrite(actuadores[i].pin, encender ? RELAY_ACTIVE_LEVEL : RELAY_INACTIVE_LEVEL);
      }
      return;
    }
  }
}

void lanzarPulsoActuador(uint8_t id, uint32_t duracionMs) {
  for (int i = 0; i < TOTAL_ACTUADORES; i++) {
    if (actuadores[i].id == id) {
      actuadores[i].estado = true;
      actuadores[i].pulsoFinMs = millis() + duracionMs;
      if (actuadores[i].esBuzzer) {
        digitalWrite(actuadores[i].pin, BUZZER_ACTIVE_LEVEL);
      } else {
        digitalWrite(actuadores[i].pin, RELAY_ACTIVE_LEVEL);
      }
      return;
    }
  }
}

void apagarTodosLosActuadores() {
  for (int i = 0; i < TOTAL_ACTUADORES; i++) {
    actuadores[i].estado = false;
    actuadores[i].pulsoFinMs = 0;
    if (actuadores[i].esBuzzer) {
      digitalWrite(actuadores[i].pin, BUZZER_INACTIVE_LEVEL);
    } else {
      digitalWrite(actuadores[i].pin, RELAY_INACTIVE_LEVEL);
    }
  }
}

void procesarTemporizadoresPulsos() {
  uint32_t ahora = millis();
  for (int i = 0; i < TOTAL_ACTUADORES; i++) {
    if (actuadores[i].pulsoFinMs > 0 && ahora >= actuadores[i].pulsoFinMs) {
      actuadores[i].estado = false;
      actuadores[i].pulsoFinMs = 0;
      if (actuadores[i].esBuzzer) {
        digitalWrite(actuadores[i].pin, BUZZER_INACTIVE_LEVEL);
      } else {
        digitalWrite(actuadores[i].pin, RELAY_INACTIVE_LEVEL);
      }
    }
  }
}

// ============================================================================
// 10. LECTURA DE SENSORES, PT1 ALTA RESOLUCIÓN (ADC_0db 256x), ÁNODO PASIVO Y FAILSAFES
// ============================================================================
float calcularVolumenPorAlturaCm(float heightCm) {
  if (heightCm <= 0.0f) return 0.0f;
  if (heightCm <= TANK_CONE_HEIGHT_CM) {
    float ratio = heightCm / TANK_CONE_HEIGHT_CM;
    return TANK_CONE_VOLUME_L * (ratio * ratio * ratio);
  }
  float cylH = heightCm - TANK_CONE_HEIGHT_CM;
  if (cylH > TANK_CYL_HEIGHT_CM) cylH = TANK_CYL_HEIGHT_CM;
  return TANK_CONE_VOLUME_L + (cylH * TANK_CYL_LITERS_PER_CM);
}

void guardarCalibracionPt1Nvs() {
  if (pt1Prefs.begin("pt1cal", false)) {
    pt1Prefs.putFloat("zeroMv", pt1ZeroMv);
    pt1Prefs.putFloat("mvPerCm", pt1MvPerCm);
    pt1Prefs.end();
  }
}

void cargarCalibracionPt1Nvs() {
  if (pt1Prefs.begin("pt1cal", true)) {
    pt1ZeroMv  = pt1Prefs.getFloat("zeroMv", PT1_DEFAULT_ZERO_MV);
    pt1MvPerCm = pt1Prefs.getFloat("mvPerCm", PT1_DEFAULT_MV_PER_CM);
    pt1Prefs.end();
  }
  if (isnan(pt1ZeroMv) || pt1ZeroMv < 60.0f || pt1ZeroMv > 2500.0f) {
    pt1ZeroMv = PT1_DEFAULT_ZERO_MV;
  }
  if (isnan(pt1MvPerCm) || pt1MvPerCm < 0.02f || pt1MvPerCm > 25.0f) {
    pt1MvPerCm = PT1_DEFAULT_MV_PER_CM;
  }
}

void guardarCalibracionElectricaNvs() {
  if (elecPrefs.begin("eleccal", false)) {
    elecPrefs.putFloat("vScale", calibVScale);
    elecPrefs.putInt("iMode", calibIMode);
    elecPrefs.putFloat("iNom", calibINominal);
    elecPrefs.putFloat("iMult", calibISagFactor);
    elecPrefs.putFloat("dosis", calibDosisMgL);
    elecPrefs.putUInt("tMaxMin", calibTiempoMaxMin);
    elecPrefs.putFloat("volL", calibVolumenL);
    elecPrefs.end();
  }
}

void cargarCalibracionElectricaNvs() {
  if (elecPrefs.begin("eleccal", true)) {
    calibVScale       = elecPrefs.getFloat("vScale", 2.0f);
    calibIMode        = elecPrefs.getInt("iMode", 0);
    calibINominal     = elecPrefs.getFloat("iNom", 11.0f);
    calibISagFactor   = elecPrefs.getFloat("iMult", 1.0f);
    calibDosisMgL     = elecPrefs.getFloat("dosis", 80.0f);
    calibTiempoMaxMin = elecPrefs.getUInt("tMaxMin", 50);
    calibVolumenL     = elecPrefs.getFloat("volL", 117.3f);
    elecPrefs.end();
  }
  if (isnan(calibVScale) || calibVScale < 0.5f || calibVScale > 10.0f) calibVScale = 2.0f;
  if (calibIMode < 0 || calibIMode > 1) calibIMode = 0;
  if (isnan(calibINominal) || calibINominal < 1.0f || calibINominal > 40.0f) calibINominal = 11.0f;
  if (isnan(calibISagFactor) || calibISagFactor < 0.1f || calibISagFactor > 10.0f) calibISagFactor = 1.0f;
  if (isnan(calibDosisMgL) || calibDosisMgL < 10.0f || calibDosisMgL > 150.0f) calibDosisMgL = 80.0f;
  if (calibTiempoMaxMin < 5 || calibTiempoMaxMin > 180) calibTiempoMaxMin = 50;
  if (isnan(calibVolumenL) || calibVolumenL < 10.0f || calibVolumenL > 250.0f) calibVolumenL = 117.3f;
}

static void muestrearPinAdcAltaRes(uint8_t pin, float* outRawAvg, float* outMvCal, bool* outConnected) {
  const int N = 256;
  uint32_t sumRaw = 0;
  uint32_t sumMv  = 0;
  uint16_t minR   = 4095;
  uint16_t maxR   = 0;

  (void)analogRead(pin);
  (void)analogRead(pin);

  for (int i = 0; i < N; i++) {
    uint16_t r = analogRead(pin);
    sumRaw += r;
    if (r < minR) minR = r;
    if (r > maxR) maxR = r;
    if ((i & 7) == 0) {
      sumMv += analogReadMilliVolts(pin); // 32 muestras calibradas por eFuse
    }
    if ((i & 31) == 0) {
      delayMicroseconds(120);
    }
  }

  float rawAvg   = (float)sumRaw / (float)N;
  float mvAnchor = (float)sumMv / 32.0f;
  float mvHighRes = mvAnchor;
  if (rawAvg > 20.0f && mvAnchor > 20.0f) {
    float anchorRaw = roundf(rawAvg);
    if (anchorRaw > 10.0f) {
      mvHighRes = mvAnchor * (rawAvg / anchorRaw);
    }
  }
  uint16_t pp = (maxR >= minR) ? (maxR - minR) : 0;
  // Con divisor de voltaje en P32, 500 mV baja a ~150 mV; umbral de conexion >= 80 mV
  bool connected = (mvHighRes >= 80.0f && mvHighRes <= 4600.0f && pp < 450);

  *outRawAvg    = rawAvg;
  *outMvCal     = mvHighRes;
  *outConnected = connected;
}

static void muestrearPt1AltaResolucion(float* outRawAvg, float* outMvCal, uint8_t* outPin, bool* outConnected) {
  float raw32 = 0.0f, mv32 = 0.0f;
  bool conn32 = false;
  muestrearPinAdcAltaRes(PIN_ADC_PRESOSTATO, &raw32, &mv32, &conn32);
  *outRawAvg    = raw32;
  *outMvCal     = mv32;
  *outPin       = PIN_ADC_PRESOSTATO;
  *outConnected = conn32;
}

void tararPt1Cero() {
  float rawAvg = 0.0f, mvCal = 0.0f;
  uint8_t usedPin = PIN_ADC_PRESOSTATO;
  bool conn = false;
  muestrearPt1AltaResolucion(&rawAvg, &mvCal, &usedPin, &conn);
  if (mvCal >= 80.0f && mvCal <= 2500.0f) {
    pt1ZeroMv  = mvCal;
    pt1Mv      = mvCal;
    pt1EmaInit = true;
    guardarCalibracionPt1Nvs();
    Serial.printf("[PT1 CAL] Tara 0 cm guardada: zeroMv = %.2f mV (Pin P%u, Raw=%.2f)\n", pt1ZeroMv, usedPin, rawAvg);
  }
}

void calibrarPt1AlturaCm(float alturaAguaCm) {
  if (alturaAguaCm <= 0.5f) {
    tararPt1Cero();
    return;
  }
  float rawAvg = 0.0f, mvCal = 0.0f;
  uint8_t usedPin = PIN_ADC_PRESOSTATO;
  bool conn = false;
  muestrearPt1AltaResolucion(&rawAvg, &mvCal, &usedPin, &conn);
  float deltaMv = mvCal - pt1ZeroMv;
  if (deltaMv > 1.5f) {
    pt1MvPerCm = deltaMv / alturaAguaCm;
    pt1Mv      = mvCal;
    pt1EmaInit = true;
    guardarCalibracionPt1Nvs();
    Serial.printf("[PT1 CAL] Calibrado a %.1f cm: V_actual=%.2f mV, Zero=%.2f mV -> Sensibilidad=%.5f mV/cm\n",
                  alturaAguaCm, mvCal, pt1ZeroMv, pt1MvPerCm);
  }
}

void resetCalibracionPt1() {
  pt1ZeroMv  = PT1_DEFAULT_ZERO_MV;
  pt1MvPerCm = PT1_DEFAULT_MV_PER_CM;
  guardarCalibracionPt1Nvs();
}

// Muestrea ciclo en GPIO 34 (ACS712-05B en entrada 120V AC de ATX o prueba DC directa)
// Entrega Amperes AC RMS directos (0.0A a 5.0A AC) sin riesgo de sobrecargar el sensor de 5A
static float muestrearCorrienteAcs712(uint8_t pin) {
  const int N = 64;
  uint16_t rawBuf[N];
  uint32_t sumRaw = 0;
  for (int i = 0; i < N; i++) {
    rawBuf[i] = analogRead(pin);
    sumRaw += rawBuf[i];
    delayMicroseconds(250); // 64 * 250us = 16ms = ciclo completo de 60Hz
  }
  float meanRaw = (float)sumRaw / (float)N;
  float meanV   = (meanRaw / 4095.0f) * 3.30f;

  float sumSqV = 0.0f;
  for (int i = 0; i < N; i++) {
    float v = ((float)rawBuf[i] / 4095.0f) * 3.30f;
    float dv = v - meanV;
    sumSqV += dv * dv;
  }
  float acRmsV = sqrtf(sumSqV / (float)N);

  // Auto-detección del divisor de voltaje (10k + 10k -> V_reposo ~ 1.17V - 1.25V; sin divisor -> ~ 2.50V)
  bool tieneDivisor = (meanV < 1.85f);
  float zeroVolts   = tieneDivisor ? 1.17f : 2.50f;
  float sensVperA   = tieneDivisor ? (0.185f * 0.50f) : 0.185f;

  float dcDeltaV    = fabsf(meanV - zeroVolts);
  float effectiveV  = (acRmsV > 0.008f) ? acRmsV : ((dcDeltaV > 0.015f) ? dcDeltaV : 0.0f);

  if (effectiveV < 0.008f) return 0.0f; // Filtro de ruido en reposo (menor a 8 mV)

  float iMedida = effectiveV / sensVperA;
  return constrain(iMedida, 0.0f, 5.0f); // Sensor físico ACS712-05B: límite seguro 5A
}

uint32_t leerAdc1ConDescargaSH(uint8_t pin, uint32_t* outMvPromedio, uint32_t* outPpNoise, bool* outFlotando) {
  uint32_t accRaw = 0;
  uint32_t accMv  = 0;
  uint32_t minRaw = 4095;
  uint32_t maxRaw = 0;

  (void)analogRead(pin);
  // Muestrear a lo largo de un ciclo completo de 60Hz (~16.6ms) para cancelar el zumbido de red
  for (int i = 0; i < 32; i++) {
    uint32_t r  = analogRead(pin);
    uint32_t mv = analogReadMilliVolts(pin);
    accRaw += r;
    accMv  += mv;
    if (r < minRaw) minRaw = r;
    if (r > maxRaw) maxRaw = r;
    delayMicroseconds(520); // 32 * 520us = 16.64ms (1 ciclo de 60Hz)
  }

  uint32_t avgRaw = accRaw / 32;
  uint32_t avgMv  = accMv / 32;
  uint32_t pp     = (maxRaw >= minRaw) ? (maxRaw - minRaw) : 0;

  // Solo se marca flotando si el voltaje promedio es prácticamente nulo (<10 mV) con oscilación salvaje
  bool flotando = (avgMv < 10 && pp > 400);

  // SIEMPRE entregamos la lectura física real medida sin forzarla a cero
  if (outMvPromedio) *outMvPromedio = avgMv;
  if (outPpNoise)    *outPpNoise    = pp;
  if (outFlotando)   *outFlotando   = flotando;
  return avgRaw;
}

float leerAnodoPasivoVarillaV() {
  uint32_t avgMv = 0;
  uint32_t pp = 0;
  bool flotando = false;
  rawAnodoPasivo = leerAdc1ConDescargaSH(PIN_ADC_ANODO_PASIVO, &avgMv, &pp, &flotando);
  ruidoPpAnodoPasivo = pp;
  pinP36AlAire = flotando;
  vPinAnodoPasivoV = avgMv / 1000.0f;
  vVarillaAnodoPasivoV = vPinAnodoPasivoV * calibVScale;
  return vVarillaAnodoPasivoV;
}

void iniciarPausaMuestreoAnodoPasivo() {
  if (pausaMuestreoActiva) return;
  ssr1EstabaEncendido = actuadores[2].estado; // Actuador 3 (índice 2) = SSR 1 (P33)
  if (ssr1EstabaEncendido) {
    vAnodoConFuenteOnV = leerAnodoPasivoVarillaV();
  }
  // Apagar SSR 1 (P33 = LOW) durante 1.5 segundos para medir el potencial sin la fuente ATX
  fijarActuador(3, false);
  pausaMuestreoActiva = true;
  tomoMuestra50ms = false;
  tomoMuestra1400ms = false;
  tInicioPausaAnodoMs = millis();
  Serial.printf("[P36 MUESTREO] Iniciando pausa de 1.5s (P33=OFF). V_ON previo: %.3f V\n", vAnodoConFuenteOnV);
}

void procesarMuestreoAnodoPasivo() {
  if (!pausaMuestreoActiva) return;
  uint32_t dt = millis() - tInicioPausaAnodoMs;

  // Asegurar que P33 permanezca apagado durante toda la ventana de 1.5s
  if (actuadores[2].estado) {
    fijarActuador(3, false);
  }

  if (!tomoMuestra50ms && dt >= 50) {
    vAnodoPausa50msV = leerAnodoPasivoVarillaV();
    if (vAnodoConFuenteOnV > 0.05f) {
      deltaVIrOhmicaV = max(0.0f, vAnodoConFuenteOnV - vAnodoPausa50msV);
    }
    tomoMuestra50ms = true;
  }

  if (!tomoMuestra1400ms && dt >= 1400) {
    vAnodoPausa1400msV = leerAnodoPasivoVarillaV();
    tomoMuestra1400ms = true;
  }

  if (dt >= 1500) {
    pausaMuestreoActiva = false;
    contadorMuestrasP36++;
    if (ssr1EstabaEncendido && !paroEmergencia) {
      fijarActuador(3, true); // Reanuda SSR 1 (P33) tras los 1.5s
    }
    Serial.printf("[P36 MUESTREO #%u] V_ON: %.3f V | V_50ms: %.3f V | V_1.4s: %.3f V | dV_IR: %.3f V\n",
                  contadorMuestrasP36, vAnodoConFuenteOnV, vAnodoPausa50msV, vAnodoPausa1400msV, deltaVIrOhmicaV);
  }
}

void actualizarLecturasSensores() {
  // 1) Lectura de alta resolución del Transductor de Presión PT1 (0-100 PSI en 1.10 m)
  float rawPt1Avg = 0.0f, mvPt1Inst = 0.0f;
  uint8_t usedPt1Pin = PIN_ADC_PRESOSTATO;
  bool connPt1 = false;
  muestrearPt1AltaResolucion(&rawPt1Avg, &mvPt1Inst, &usedPt1Pin, &connPt1);

  pt1PinActual = usedPt1Pin;
  pt1RawFloat  = rawPt1Avg;
  pt1Raw       = (uint16_t)roundf(rawPt1Avg);
  pt1Connected = connPt1;

  // Filtro adaptativo PT1: si hay burbujas de floculación o agitación de bomba pecera, usa filtrado pesado
  float alphaPt1 = (actuadores[2].estado || actuadores[3].estado) ? 0.05f : 0.22f;
  if (!pt1EmaInit) {
    pt1Mv      = mvPt1Inst;
    pt1EmaInit = true;
  } else {
    pt1Mv = (alphaPt1 * mvPt1Inst) + ((1.0f - alphaPt1) * pt1Mv);
  }
  pt1Volts = pt1Mv / 1000.0f;

  if (pt1Connected && pt1MvPerCm > 0.01f) {
    float deltaMv = pt1Mv - pt1ZeroMv;
    if (fabsf(deltaMv) < 0.15f) deltaMv = 0.0f;
    float hCm = deltaMv / pt1MvPerCm;
    if (hCm < 0.0f) hCm = 0.0f;
    if (hCm > 130.0f) hCm = 130.0f;

    // Amortiguador de oleaje y microburbujas en el cono (max 0.6cm de cambio brusco por ciclo)
    if (pt1HeightCm > 0.1f && fabsf(hCm - pt1HeightCm) > 0.6f) {
      hCm = pt1HeightCm + ((hCm > pt1HeightCm) ? 0.6f : -0.6f);
    }

    pt1HeightCm = hCm;
    pt1Psi      = hCm * PT1_PSI_PER_CM_H2O;
    nivelHidrostaticoPct = constrain((hCm / TANK_TOTAL_HEIGHT_CM) * 100.0f, 0.0f, 100.0f);
    pt1Liters   = calcularVolumenPorAlturaCm(hCm);
  } else {
    pt1HeightCm = 0.0f;
    pt1Psi      = 0.0f;
    nivelHidrostaticoPct = 0.0f;
    pt1Liters   = 0.0f;
  }

  // 2) Muestreo sincrónico de corriente: ACS712-05B en GPIO 34 (Primaria AC 120V)
  corrienteAcPrimariaA = muestrearCorrienteAcs712(PIN_ADC_I_ELECTRODOS);

  // Lectura continua de alta precisión de Celda con divisor 4:1 (GPIO 36 / VP)
  leerAnodoPasivoVarillaV();
  if (actuadores[2].estado && !pausaMuestreoActiva) {
    vAnodoConFuenteOnV = vVarillaAnodoPasivoV;
  }

  // Estimación de Corriente Real en Electrodos (Nominal Fijo o Sensor Dinámico):
  if (!actuadores[2].estado) {
    // Si la Fuente ATX (SSR 1 - GPIO 33) está APAGADA, la corriente es CERO absoluto
    corrienteEstimadaDcA = 0.0f;
    corrienteElectrodosA = 0.0f;
  } else {
    if (calibIMode == 0) {
      // MODO NOMINAL FIJO: Alta estabilidad matemática para integración de Faraday
      corrienteEstimadaDcA = calibINominal;
      corrienteElectrodosA = calibINominal;
    } else {
      // MODO SENSOR DINÁMICO: Sag de celda + ACS712 ajustado con factor fino
      float iSag = 0.0f;
      if (vVarillaAnodoPasivoV > 4.0f && vVarillaAnodoPasivoV < 12.20f) {
        iSag = ((12.20f - vVarillaAnodoPasivoV) / 0.22f) * calibISagFactor;
        if (iSag < 0.0f) iSag = 0.0f;
        if (iSag > 25.0f) iSag = 25.0f;
      }

      float iAcNeta = (corrienteAcPrimariaA > 0.18f) ? (corrienteAcPrimariaA - 0.18f) : 0.0f;
      float iDcAc   = iAcNeta * 5.25f * calibISagFactor;

      if (iDcAc > 0.3f && iSag > 0.3f) {
        corrienteEstimadaDcA = (0.5f * iDcAc) + (0.5f * iSag);
      } else if (iDcAc > 0.3f) {
        corrienteEstimadaDcA = iDcAc;
      } else if (iSag > 0.3f) {
        corrienteEstimadaDcA = iSag;
      } else {
        corrienteEstimadaDcA = calibINominal; // Fallback seguro a corriente nominal
      }
      corrienteElectrodosA = corrienteEstimadaDcA;
    }
  }

  bool flotDC = false;
  uint32_t rawDC  = leerAdc1ConDescargaSH(PIN_ADC_I_BOMBAS_DC, nullptr, nullptr, &flotDC);
  float vDC  = (rawDC / 4095.0f) * 3.3f;
  corrienteBombasDCA   = vDC * CALIB_DC_AMPS_PER_VOLT;

  uint32_t ahora = millis();
  if (ahora - ultimaMedicionFlujoMs >= 1000) {
    uint32_t dtMs = ahora - ultimaMedicionFlujoMs;
    ultimaMedicionFlujoMs = ahora;

    noInterrupts();
    uint32_t pulsos = pulsosHall;
    pulsosHall = 0;
    interrupts();

    caudalLpm = (pulsos * 1000.0f / dtMs) / 7.5f;
    litrosTotalizados += (caudalLpm / 60.0f) * (dtMs / 1000.0f);
  }

  // Paro de Rebose (P21): 2 cables de cobre tocando agua
  if (digitalRead(PIN_PARO_REBOSE_CONO) == LOW) {
    if (!paroEmergencia) {
      paroEmergencia = true;
      motivoEmergencia = "¡REBOSE CRÍTICO EN TAPA DE CONO! (P21)";
      apagarTodosLosActuadores();
      fijarActuador(10, true);
    }
  }

  // Protección por Sobrecorriente Electrodos (> 38A)
  if (corrienteElectrodosA > I_CORTE_CRITICO_40A) {
    if (!paroEmergencia) {
      paroEmergencia = true;
      motivoEmergencia = "¡SOBRECORRIENTE ELECTRODOS > 38A! (P34)";
      apagarTodosLosActuadores();
      fijarActuador(10, true);
    }
  }

  // Protección Deadhead Bombas (> 7.5A)
  if (corrienteBombasDCA > I_CORTE_CRITICO_BOMBAS) {
    fijarActuador(5, false);
    fijarActuador(1, false);
  }
}

// ============================================================================
// 11. AUTÓMATA DE CONTROL INTEGRAL "AGUA DE GÍA" (CORREGIDO)
// ============================================================================
const float VOLTAJE_FUENTE_EF     = 12.0f;    // Fuente ATX de electrofloculación riel 12V (SSR 1)
const float K_CELDA_PPM           = 12000.0f; // Constante geométrica (calibrable en campo: PPM_real * R0)
const float UMBRAL_TOQUE_ANODO_A  = 0.25f;    // Amperaje mínimo al tocar la punta del ánodo
const float I_MODULACION_MAX_A    = 12.5f;    // Cierra solenoide B si sube a 12.5 A
const float I_MODULACION_MIN_A    = 11.0f;    // Reabre solenoide B cuando baja a 11.0 A

uint32_t tToqueAnodoMs = 0;
uint32_t tInicioLavadoRoMs = 0;
bool lavadoRoEnCurso = false;
bool conoLleno1BBL = false;

// Control autónomo del Tótem Superior (Etapa 1)
void controlarTotemSuperior() {
  if (!modoAutomatico || paroEmergencia) return;

  // INPUT_PULLUP en GPIO 19: HIGH = Seco (falta agua), LOW = Agua tocó el electrodo superior (Lleno)
  bool totemLleno = (digitalRead(PIN_NIVEL_ALTO_TOTEM) == LOW);
  if (!totemLleno) {
    fijarActuador(1, true);  // Enciende Bomba Evans Pozo 24V - Bixolon #2 Ch2 (Actuador 1 - GPIO 14 / P14)
  } else {
    fijarActuador(1, false); // Corta Bomba Evans Pozo al llenarse el Tótem (Actuador 1 - GPIO 14 / P14)
  }
}

void registrarBitacoraAuto(const char* motivo) {
  uint16_t idx = totalBitacoraRows % MAX_BITACORA_ROWS;
  RegistroBitacora& r = bitacoraCampo[idx];
  r.id           = totalBitacoraRows + 1;
  r.uptime_s     = millis() / 1000;
  r.etapa_id     = (uint8_t)estadoActual;
  strncpy(r.etapa_nombre, getNombreEstado(estadoActual), sizeof(r.etapa_nombre));
  r.ssr1_on      = actuadores[2].estado;
  r.nivel_cm     = pt1HeightCm;
  r.litros       = pt1Liters;
  r.psi          = pt1Psi;
  r.v_multimetro = 0.0f;
  r.i_pinza_ac   = 0.0f;
  r.v_p36_pin    = vPinAnodoPasivoV;
  r.v_p36_rod    = vVarillaAnodoPasivoV;
  r.v_on         = vAnodoConFuenteOnV;
  r.v_off_50ms   = vAnodoPausa50msV;
  r.v_off_1400ms = vAnodoPausa1400msV;
  r.dv_ir        = deltaVIrOhmicaV;
  r.i_dc_est     = corrienteElectrodosA;
  r.i_ac_prim    = corrienteAcPrimariaA;
  r.tds_ppm      = (tdsManualIngresado ? tdsManualPPM : tdsEstimadoPPM);

  String acts = "";
  for (int i = 0; i < TOTAL_ACTUADORES; i++) {
    if (actuadores[i].estado) acts += "A" + String(actuadores[i].id) + " ";
  }
  acts.trim();
  strncpy(r.actuadores_str, acts.c_str(), sizeof(r.actuadores_str));

  if (motivo) {
    strncpy(r.nota, motivo, sizeof(r.nota));
  } else {
    r.nota[0] = '\0';
  }

  totalBitacoraRows++;
  Serial.printf("[BITACORA AUTO #%u] %s | Etapa=%s | Nivel=%.1fcm (%.1fL) | V_celda=%.2fV | I_DC=%.1fA | I_AC=%.2fA\n",
                r.id, motivo ? motivo : "", r.etapa_nombre, r.nivel_cm, r.litros, r.v_p36_rod, r.i_dc_est, r.i_ac_prim);
}

void cambiarEstado(EstadoProceso nuevoEstado) {
  estadoActual = nuevoEstado;
  tiempoInicioEstadoMs = millis();
  tiempoBarridoPeceraMs = millis();
  barridoPeceraActivo = false;
  tiempoPulsoLlenadoMs = millis();
  solenoideLlenadoPulsando = false;
  tToqueAnodoMs = 0;
  lavadoRoEnCurso = false;
  if (nuevoEstado != ESTADO_ELECTROFLOCULACION) {
    tInicioFloculacionMs = 0;
  }

  Serial.printf("[AUTO] Cambio a Estado: %d (%s)\n", nuevoEstado, getNombreEstado(nuevoEstado));
  String mot = "Cambio a Etapa " + String((int)nuevoEstado) + ": " + String(getNombreEstado(nuevoEstado));
  registrarBitacoraAuto(mot.c_str());
}

void procesarAutomataMaestro() {
  if (!modoAutomatico || paroEmergencia) return;

  // 1. Mantiene vigilado el nivel del Tótem superior en todo momento
  controlarTotemSuperior();

  // Muestreo forense periódico cada 60s durante cualquier etapa activa
  static uint32_t ultimoLogAutoMs = 0;
  if (millis() - ultimoLogAutoMs >= 60000) {
    ultimoLogAutoMs = millis();
    if (estadoActual != ESTADO_STANDBY) {
      registrarBitacoraAuto("Muestreo periodico 60s");
    }
  }

  uint32_t tiempoEnEstadoMs = millis() - tiempoInicioEstadoMs;

  switch (estadoActual) {

    // ------------------------------------------------------------------------
    // ETAPA 0: STANDBY / REPOSO
    // ------------------------------------------------------------------------
    case ESTADO_STANDBY:
      apagarTodosLosActuadores();
      break;

    // ------------------------------------------------------------------------
    // ETAPA 1: PRE-ENERGIZADO Y BAJADA POR GRAVEDAD HASTA TOCAR EL ÁNODO
    // ------------------------------------------------------------------------
    case ESTADO_LLENADO_ADAPTATIVO: {
      // Prende PRIMERO Fuente ATX Electrofloculación - SSR 1 (Actuador 3 - GPIO 33 / P33) y abre Solenoide Llenado Cono 120VAC - SSR 2 (Actuador 2 - GPIO 27 / P27)
      fijarActuador(3, true);
      fijarActuador(2, true);
      conoLleno1BBL = false;

      // Cuando el agua sube y toca la punta del ánodo, empieza a circular corriente
      if (corrienteElectrodosA >= UMBRAL_TOQUE_ANODO_A) {
        if (tToqueAnodoMs == 0) {
          tToqueAnodoMs = millis(); // Ventana de 800 ms para estabilizar el contacto
        } else if (millis() - tToqueAnodoMs >= 800) {
          cambiarEstado(ESTADO_CALCULO_FARADAY);
        }
      } else {
        tToqueAnodoMs = 0;
      }
      break;
    }

    // ------------------------------------------------------------------------
    // ETAPA 2: CÁLCULO DE RESISTENCIA INICIAL (R0), PPM Y META FARADAY
    // ------------------------------------------------------------------------
    case ESTADO_CALCULO_FARADAY: {
      // Prioriza la medición única del medidor físico ingresada al inicio del lote (o 3000 ppm por defecto)
      if (tdsManualIngresado && tdsManualPPM >= 50.0f) {
        tdsEstimadoPPM = tdsManualPPM;
      } else {
        tdsEstimadoPPM = 3000.0f;
        tdsManualPPM   = 3000.0f;
        tdsManualIngresado = true;
      }

      // Volumen real del lote (PT1 litros si hay lectura válida > 50L, o volumen nominal configurado)
      float volReal = (pt1Liters > 50.0f) ? pt1Liters : calibVolumenL;

      // Meta de hierro (g) = (dosis_mg_L * volumen_L) / 1000
      gramosFeMeta = (calibDosisMgL * volReal) / 1000.0f;
      gramosFeDisueltos = 0.0f;
      pctFaraday = 0.0f;

      // Cálculo de tasa de disolución y tiempo estimado a la corriente operativa
      float iOperacion = (calibIMode == 0) ? calibINominal : max(corrienteElectrodosA, 10.0f);
      tasaDisolucionFeGps = (iOperacion * PESO_ATOMICO_FE) / (VALENCIA_FE * CONSTANTE_FARADAY);
      if (tasaDisolucionFeGps > 0.0001f) {
        tEstimadoTotalFlocSeg = (uint32_t)ceilf(gramosFeMeta / tasaDisolucionFeGps);
      } else {
        tEstimadoTotalFlocSeg = 2940; // ~49 minutos a 11A
      }
      tRestanteFlocSeg = tEstimadoTotalFlocSeg;
      tTranscurridoFlocSeg = 0;
      tInicioFloculacionMs = millis();

      // Enciende la Bomba Pecera Whirlpool 120VAC - SSR 3 (Actuador 4 - GPIO 26 / P26)
      fijarActuador(4, true);

      Serial.printf("[FARADAY PROCESO] TDS: %.0f PPM | Dosis: %.1f mg/L | Vol: %.1f L | Meta Fe: %.3f g | T_Est: %u s (%.1f min)\n",
                    tdsEstimadoPPM, calibDosisMgL, volReal, gramosFeMeta, tEstimadoTotalFlocSeg, tEstimadoTotalFlocSeg / 60.0f);

      cambiarEstado(ESTADO_ELECTROFLOCULACION);
      break;
    }

    // ------------------------------------------------------------------------
    // ETAPA 3: LLENADO MODULADO (11.0A - 12.5A) Y ELECTROFLOCULACIÓN ACTIVA
    // ------------------------------------------------------------------------
    case ESTADO_ELECTROFLOCULACION: {
      if (tInicioFloculacionMs == 0) {
        tInicioFloculacionMs = millis();
      }

      // Muestreo automático cada 60 segundos: pausa de 1.5s con P33 = OFF para leer Ánodo Pasivo (P36)
      if (!pausaMuestreoActiva && (millis() - ultimoMuestreo60sMs >= 60000)) {
        ultimoMuestreo60sMs = millis();
        iniciarPausaMuestreoAnodoPasivo();
      }
      if (!pausaMuestreoActiva) {
        fijarActuador(3, true); // SSR 1 (P33) = ON (Fuente ATX 12V)
      }
      fijarActuador(4, true); // SSR 3 (P26) = ON (Bomba Pecera recirculación)

      // Reconocimiento de tanque lleno (por caudalímetro, altura o volumen > 80L)
      if (litrosTotalizados >= calibVolumenL || nivelHidrostaticoPct >= 80.0f || pt1Liters >= 80.0f) {
        conoLleno1BBL = true;
      }

      // Modulación de Solenoide Llenado (GPIO 27) si aún no está lleno
      if (!conoLleno1BBL) {
        if (corrienteElectrodosA >= I_MODULACION_MAX_A) {
          fijarActuador(2, false);
        } else if (corrienteElectrodosA <= I_MODULACION_MIN_A) {
          fijarActuador(2, true);
        }
      } else {
        fijarActuador(2, false);
      }

      // Odómetro de Culombios y Cronómetro de Cuenta Regresiva (segundo a segundo)
      static uint32_t ultimoSegundoFaradayMs = 0;
      if (millis() - ultimoSegundoFaradayMs >= 1000) {
        uint32_t dtMs = millis() - ultimoSegundoFaradayMs;
        ultimoSegundoFaradayMs = millis();
        float dtSeg = dtMs / 1000.0f;

        float d_mFe = (corrienteElectrodosA * dtSeg * PESO_ATOMICO_FE) / (VALENCIA_FE * CONSTANTE_FARADAY);
        gramosFeDisueltos += d_mFe;
        pctFaraday = constrain((gramosFeDisueltos / gramosFeMeta) * 100.0f, 0.0f, 100.0f);

        tTranscurridoFlocSeg = (millis() - tInicioFloculacionMs) / 1000;
        float iOp = max(corrienteElectrodosA, 0.5f);
        tasaDisolucionFeGps = (iOp * PESO_ATOMICO_FE) / (VALENCIA_FE * CONSTANTE_FARADAY);
        if (gramosFeMeta > gramosFeDisueltos && tasaDisolucionFeGps > 0.0001f) {
          tRestanteFlocSeg = (uint32_t)ceilf((gramosFeMeta - gramosFeDisueltos) / tasaDisolucionFeGps);
        } else {
          tRestanteFlocSeg = 0;
        }
      }

      // Condición de Paro Automático Seguro:
      // 1) Se alcanzó la masa de Fe requerida (gramosFeDisueltos >= gramosFeMeta)
      // 2) O expiró el tiempo máximo de seguridad configurado por el usuario (ej. 50-60 min)
      uint32_t tSegundosMaximos = calibTiempoMaxMin * 60;
      bool metaAlcanzada = (gramosFeDisueltos >= gramosFeMeta && gramosFeMeta > 0.1f);
      bool tiempoAgotado = (tTranscurridoFlocSeg >= tSegundosMaximos);

      if (metaAlcanzada || tiempoAgotado) {
        Serial.printf("[FARADAY FIN] Fe: %.3f/%.3f g | Tiempo: %u s (Max %u s) | Paro: %s\n",
                      gramosFeDisueltos, gramosFeMeta, tTranscurridoFlocSeg, tSegundosMaximos,
                      metaAlcanzada ? "Meta Faraday Alcanzada" : "Tiempo Limite de Seguridad");
        fijarActuador(2, false);
        fijarActuador(3, false);
        fijarActuador(4, false);
        tInicioFloculacionMs = 0;
        cambiarEstado(ESTADO_REPOSO_FORZADO);
      }
      break;
    }

    // ------------------------------------------------------------------------
    // ETAPA 4: REPOSO FORZADO (DECANTACIÓN DE FLÓCULOS)
    // ------------------------------------------------------------------------
    case ESTADO_REPOSO_FORZADO: {
      fijarActuador(2, false);
      fijarActuador(3, false);
      fijarActuador(4, false);

      if (tiempoEnEstadoMs >= 2700000) {
        cambiarEstado(ESTADO_TRASVASE_FILTRADO);
      }
      break;
    }

    // ------------------------------------------------------------------------
    // ETAPA 5: TRASVASE Y FILTRADO MULTIETAPA (80um -> Bomba -> 50um -> Carbón)
    // ------------------------------------------------------------------------
    case ESTADO_TRASVASE_FILTRADO: {
      fijarActuador(5, true); // Bomba Trasvase 12V vía Relé 30A - Bixolon #2 Ch1 (Actuador 5 - GPIO 25 / P25)

      if (tiempoEnEstadoMs > 20000 && caudalLpm < 0.2f) {
        fijarActuador(5, false);
        cambiarEstado(ESTADO_PRESURIZACION_MANIFOLD);
      }
      break;
    }

    // ------------------------------------------------------------------------
    // ETAPA 6: PRESURIZACIÓN MANIFOLD (MODO CASA)
    // ------------------------------------------------------------------------
    case ESTADO_PRESURIZACION_MANIFOLD: {
      fijarActuador(6, true); // Bomba Manifold + UV (Actuador 6 - GPIO 23 / P23)
      fijarActuador(7, true); // Solenoide Vía A Casa 12V (Actuador 7 - GPIO 15 / P15)

      if (digitalRead(PIN_PRESOSTATO_RO) == LOW) {
        cambiarEstado(ESTADO_OSMOSIS_100PSI);
      }
      break;
    }

    // ------------------------------------------------------------------------
    // ETAPA 7: ÓSMOSIS INVERSA 100 PSI (CON LAVADO NO BLOQUEANTE)
    // ------------------------------------------------------------------------
    case ESTADO_OSMOSIS_100PSI: {
      if (!lavadoRoEnCurso) {
        fijarActuador(6, true);  // Bomba Manifold + UV (Actuador 6 - GPIO 23 / P23)
        fijarActuador(8, true);  // Solenoide Vía B Pre-RO 12V - Bixolon #1 Ch1 (Actuador 8 - GPIO 5 / P5)
        fijarActuador(7, false); // Aísla Solenoide Vía A Casa 12V (Actuador 7 - GPIO 15 / P15)
        escribirServoAngulo(90); // Posición de trabajo en válvula de aguja (GPIO 16 / P16)

        // Cuando el barril de 20L se llena y abre el presostato (HIGH)
        if (digitalRead(PIN_PRESOSTATO_RO) == HIGH && tiempoEnEstadoMs > 30000) {
          lavadoRoEnCurso = true;
          tInicioLavadoRoMs = millis();
          fijarActuador(9, true); // Abre Solenoide Bypass Lavado RO 12V - Bixolon #1 Ch2 (Actuador 9 - GPIO 13 / P13)
        }
      } else {
        // Espera 15 segundos de lavado sin congelar el ESP32
        if (millis() - tInicioLavadoRoMs >= 15000) {
          fijarActuador(9, false);
          apagarTodosLosActuadores();
          cambiarEstado(ESTADO_STANDBY);
        }
      }
      break;
    }
  }
}

// ============================================================================
// 12. RUTAS HTTP API REST & WEB OTA
// ============================================================================
void handleRoot() {
  server.send_P(200, "text/html", INDEX_HTML);
}

void handleApiStatus() {
  String json = "{";
  json += "\"mode\":\"" + String(modoAutomatico ? "AUTO" : "MANUAL") + "\",";
  json += "\"stage\":" + String((int)estadoActual) + ",";
  json += "\"emergency\":" + String(paroEmergencia ? "true" : "false") + ",";
  json += "\"emergencyReason\":\"" + motivoEmergencia + "\",";
  json += "\"uptime_s\":" + String(millis() / 1000) + ",";

  // Wi-Fi Status
  json += "\"wifi\":{";
  json += "\"ap_ssid\":\"" + String(AP_SSID) + "\",";
  json += "\"ap_ip\":\"" + WiFi.softAPIP().toString() + "\",";
  json += "\"ap_channel\":" + String(WiFi.channel()) + ",";
  json += "\"ap_clients\":" + String(WiFi.softAPgetStationNum()) + ",";
  json += "\"sta_connected\":" + String(WiFi.status() == WL_CONNECTED ? "true" : "false") + ",";
  json += "\"sta_ssid\":\"" + String(WiFi.SSID()) + "\",";
  json += "\"sta_ip\":\"" + WiFi.localIP().toString() + "\",";
  json += "\"sta_rssi\":" + String(WiFi.RSSI());
  json += "},";

  json += "\"relays\":[";
  for (int i = 0; i < TOTAL_ACTUADORES; i++) {
    json += "{";
    json += "\"id\":" + String(actuadores[i].id) + ",";
    json += "\"pin\":" + String(actuadores[i].pin) + ",";
    json += "\"name\":\"" + String(actuadores[i].nombre) + "\",";
    json += "\"state\":" + String(actuadores[i].estado ? "true" : "false");
    json += "}";
    if (i < TOTAL_ACTUADORES - 1) json += ",";
  }
  json += "],";

  json += "\"inputs\":{";
  json += "\"totem_level\":{\"pin\":19,\"active\":" + String(digitalRead(PIN_NIVEL_ALTO_TOTEM) == LOW ? "true" : "false") + "},";
  json += "\"overflow_stop\":{\"pin\":21,\"active\":" + String(digitalRead(PIN_PARO_REBOSE_CONO) == LOW ? "true" : "false") + "},";
  json += "\"ro_pressure_sw\":{\"pin\":22,\"active\":" + String(digitalRead(PIN_PRESOSTATO_RO) == LOW ? "true" : "false") + "},";
  json += "\"adc_i_electrodes\":{\"pin\":34,\"raw\":" + String(analogRead(PIN_ADC_I_ELECTRODOS)) + ",\"current_A\":" + String(corrienteElectrodosA, 2) + "},";
  json += "\"adc_i_dc_pumps\":{\"pin\":35,\"raw\":" + String(analogRead(PIN_ADC_I_BOMBAS_DC)) + ",\"current_A\":" + String(corrienteBombasDCA, 2) + "},";
  json += "\"adc_hydrostatic\":{";
  json += "\"pin\":" + String(pt1PinActual) + ",";
  json += "\"raw\":" + String(pt1Raw) + ",";
  json += "\"raw_float\":" + String(pt1RawFloat, 2) + ",";
  json += "\"mv\":" + String(pt1Mv, 2) + ",";
  json += "\"volts\":" + String(pt1Volts, 4) + ",";
  json += "\"psi\":" + String(pt1Psi, 3) + ",";
  json += "\"height_cm\":" + String(pt1HeightCm, 1) + ",";
  json += "\"level_pct\":" + String(nivelHidrostaticoPct, 1) + ",";
  json += "\"liters\":" + String(pt1Liters, 1) + ",";
  json += "\"zero_mv\":" + String(pt1ZeroMv, 2) + ",";
  json += "\"mv_per_cm\":" + String(pt1MvPerCm, 5) + ",";
  json += "\"connected\":" + String(pt1Connected ? "true" : "false");
  json += "},";
  json += "\"passive_anode\":{";
  json += "\"pin\":36,";
  json += "\"floating\":" + String(pinP36AlAire ? "true" : "false") + ",";
  json += "\"noise_pp\":" + String(ruidoPpAnodoPasivo) + ",";
  json += "\"raw\":" + String(rawAnodoPasivo) + ",";
  json += "\"v_pin\":" + String(vPinAnodoPasivoV, 3) + ",";
  json += "\"v_rod\":" + String(vVarillaAnodoPasivoV, 3) + ",";
  json += "\"v_on\":" + String(vAnodoConFuenteOnV, 3) + ",";
  json += "\"v_off_50ms\":" + String(vAnodoPausa50msV, 3) + ",";
  json += "\"v_off_1400ms\":" + String(vAnodoPausa1400msV, 3) + ",";
  json += "\"delta_v_ir\":" + String(deltaVIrOhmicaV, 3) + ",";
  json += "\"sampling_pause\":" + String(pausaMuestreoActiva ? "true" : "false") + ",";
  json += "\"sample_count\":" + String(contadorMuestrasP36);
  json += "},";
  json += "\"flow_hall\":{\"pin\":18,\"lpm\":" + String(caudalLpm, 2) + ",\"total_liters\":" + String(litrosTotalizados, 2) + ",\"hz\":" + String((int)(caudalLpm * 7.5f)) + "},";
  json += "\"temp_c\":" + String(temperaturaCelsius, 1);
  json += "},";

  json += "\"servo\":{\"pin\":16,\"angle\":" + String(servoAnguloActual) + "},";

  json += "\"faraday\":{";
  json += "\"fe_target_g\":" + String(gramosFeMeta, 3) + ",";
  json += "\"fe_dissolved_g\":" + String(gramosFeDisueltos, 3) + ",";
  json += "\"pct\":" + String(pctFaraday, 1) + ",";
  json += "\"rate_gps\":" + String(tasaDisolucionFeGps, 5) + ",";
  json += "\"time_elapsed_s\":" + String(tTranscurridoFlocSeg) + ",";
  json += "\"time_total_s\":" + String(tEstimadoTotalFlocSeg) + ",";
  json += "\"time_remaining_s\":" + String(tRestanteFlocSeg) + ",";
  json += "\"dosis_mgl\":" + String(calibDosisMgL, 1) + ",";
  json += "\"tds_estimated_ppm\":" + String(tdsEstimadoPPM, 0) + ",";
  json += "\"tds_manual_ppm\":" + String(tdsManualPPM, 0) + ",";
  json += "\"tds_manual_set\":" + String(tdsManualIngresado ? "true" : "false") + ",";
  json += "\"tds_projected_post_floc\":" + String((int)(tdsEstimadoPPM * 0.80f)) + ",";
  json += "\"tds_projected_ro\":120";
  json += "},";

  json += "\"calib\":{";
  json += "\"v_scale\":" + String(calibVScale, 3) + ",";
  json += "\"v_pin\":" + String(vPinAnodoPasivoV, 3) + ",";
  json += "\"v_rod\":" + String(vVarillaAnodoPasivoV, 3) + ",";
  json += "\"i_mode\":" + String(calibIMode) + ",";
  json += "\"i_mode_str\":\"" + String(calibIMode == 0 ? "NOMINAL_FIJO" : "SENSOR_DINAMICO") + "\",";
  json += "\"i_nom\":" + String(calibINominal, 2) + ",";
  json += "\"i_mult\":" + String(calibISagFactor, 3) + ",";
  json += "\"dosis_mgl\":" + String(calibDosisMgL, 1) + ",";
  json += "\"t_max_min\":" + String(calibTiempoMaxMin) + ",";
  json += "\"vol_l\":" + String(calibVolumenL, 1);
  json += "},";

  json += "\"antigravity\":{";
  json += "\"msg\":\"" + jsonEscape(mensajeAntigravity) + "\",";
  json += "\"ts\":" + String(msgAntigravityTs) + ",";
  json += "\"log_count\":" + String(totalBitacoraRows);
  json += "}";

  json += "}";
  server.send(200, "application/json", json);
}

void handleApiRelay() {
  if (server.hasArg("id") && server.hasArg("state")) {
    fijarActuador(server.arg("id").toInt(), server.arg("state").toInt() == 1);
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Faltan parametros");
  }
}

void handleApiPulse() {
  if (server.hasArg("id")) {
    uint32_t dur = server.hasArg("duration") ? server.arg("duration").toInt() : 3000;
    lanzarPulsoActuador(server.arg("id").toInt(), dur);
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Falta id");
  }
}

void handleApiServo() {
  if (server.hasArg("angle")) {
    escribirServoAngulo(server.arg("angle").toInt());
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Falta angle");
  }
}

void handleApiAuto() {
  if (server.hasArg("cmd")) {
    String cmd = server.arg("cmd");
    if (server.hasArg("tds")) {
      float val = server.arg("tds").toFloat();
      if (val >= 50.0f && val <= 10000.0f) {
        tdsManualPPM = val;
        tdsManualIngresado = true;
      }
    }
    if (cmd == "start") {
      if (!tdsManualIngresado || tdsManualPPM < 50.0f) {
        tdsManualPPM = 3000.0f; // Default seguro si aún no se ha ingresado
        tdsManualIngresado = true;
      }
      modoAutomatico = true;
      paroEmergencia = false;
      cambiarEstado(ESTADO_LLENADO_ADAPTATIVO);
    } else if (cmd == "pause") {
      modoAutomatico = false;
      apagarTodosLosActuadores();
    } else if (cmd == "stop") {
      modoAutomatico = false;
      apagarTodosLosActuadores();
      cambiarEstado(ESTADO_STANDBY);
    } else if (cmd == "set_stage") {
      if (server.hasArg("stage")) {
        int st = server.arg("stage").toInt();
        if (st >= 0 && st <= 7) {
          paroEmergencia = false;
          apagarTodosLosActuadores();
          if (st > 0) modoAutomatico = true;
          else modoAutomatico = false;
          cambiarEstado((EstadoProceso)st);
        }
      }
    }
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Falta cmd");
  }
}

void handleApiEmergency() {
  if (server.hasArg("active")) {
    paroEmergencia = server.arg("active").toInt() == 1;
    if (!paroEmergencia) {
      motivoEmergencia = "";
      fijarActuador(10, false);
    } else {
      motivoEmergencia = "Activado desde Web";
      apagarTodosLosActuadores();
      fijarActuador(10, true);
    }
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Falta active");
  }
}

void handleApiSampleAnode() {
  iniciarPausaMuestreoAnodoPasivo();
  server.send(200, "text/plain", "OK");
}

void handleApiPt1Cal() {
  String cmd = server.hasArg("cmd") ? server.arg("cmd") : "";
  if (cmd == "tare") {
    tararPt1Cero();
  } else if (cmd == "height") {
    float cm = server.hasArg("cm") ? server.arg("cm").toFloat() : 0.0f;
    calibrarPt1AlturaCm(cm);
  } else if (cmd == "set") {
    float z  = server.hasArg("zero_mv") ? server.arg("zero_mv").toFloat() : pt1ZeroMv;
    float s  = server.hasArg("mv_per_cm") ? server.arg("mv_per_cm").toFloat() : pt1MvPerCm;
    if (z >= 100.0f && z <= 2500.0f) pt1ZeroMv = z;
    if (s >= 0.05f && s <= 25.0f) pt1MvPerCm = s;
    guardarCalibracionPt1Nvs();
  } else if (cmd == "reset") {
    resetCalibracionPt1();
  }
  actualizarLecturasSensores();
  String out = "{\"ok\":true,\"zero_mv\":" + String(pt1ZeroMv, 2) +
               ",\"mv_per_cm\":" + String(pt1MvPerCm, 5) +
               ",\"mv\":" + String(pt1Mv, 2) +
               ",\"height_cm\":" + String(pt1HeightCm, 1) +
               ",\"liters\":" + String(pt1Liters, 1) + "}";
  server.send(200, "application/json", out);
}

void handleApiCalib() {
  bool changed = false;

  // Calibración de Voltaje Celda / Varilla (P36)
  if (server.hasArg("v_scale")) {
    float vs = server.arg("v_scale").toFloat();
    if (vs >= 0.5f && vs <= 10.0f) {
      calibVScale = vs;
      changed = true;
    }
  }
  if (server.hasArg("v_real")) {
    float vr = server.arg("v_real").toFloat();
    if (vr > 0.5f && vPinAnodoPasivoV > 0.05f) {
      calibVScale = vr / vPinAnodoPasivoV;
      changed = true;
    }
  }

  // Calibración de Amperaje
  if (server.hasArg("i_mode")) {
    int m = server.arg("i_mode").toInt();
    if (m >= 0 && m <= 1) {
      calibIMode = m;
      changed = true;
    }
  }
  if (server.hasArg("i_nom")) {
    float inom = server.arg("i_nom").toFloat();
    if (inom >= 1.0f && inom <= 40.0f) {
      calibINominal = inom;
      changed = true;
    }
  }
  if (server.hasArg("i_mult")) {
    float im = server.arg("i_mult").toFloat();
    if (im >= 0.1f && im <= 10.0f) {
      calibISagFactor = im;
      changed = true;
    }
  }

  // Ajuste de Dosis y Temporizador
  if (server.hasArg("dosis_mgl")) {
    float d = server.arg("dosis_mgl").toFloat();
    if (d >= 10.0f && d <= 150.0f) {
      calibDosisMgL = d;
      changed = true;
    }
  }
  if (server.hasArg("t_max_min")) {
    uint32_t tm = server.arg("t_max_min").toInt();
    if (tm >= 5 && tm <= 180) {
      calibTiempoMaxMin = tm;
      changed = true;
    }
  }
  if (server.hasArg("vol_l")) {
    float vl = server.arg("vol_l").toFloat();
    if (vl >= 10.0f && vl <= 250.0f) {
      calibVolumenL = vl;
      changed = true;
    }
  }

  if (changed) {
    guardarCalibracionElectricaNvs();
    vVarillaAnodoPasivoV = vPinAnodoPasivoV * calibVScale;
    if (actuadores[2].estado) {
      corrienteElectrodosA = (calibIMode == 0) ? calibINominal : corrienteEstimadaDcA;
    }
  }

  String json = "{";
  json += "\"ok\":true,";
  json += "\"v_scale\":" + String(calibVScale, 3) + ",";
  json += "\"v_pin\":" + String(vPinAnodoPasivoV, 3) + ",";
  json += "\"v_rod\":" + String(vVarillaAnodoPasivoV, 3) + ",";
  json += "\"i_mode\":" + String(calibIMode) + ",";
  json += "\"i_mode_str\":\"" + String(calibIMode == 0 ? "NOMINAL_FIJO" : "SENSOR_DINAMICO") + "\",";
  json += "\"i_nom\":" + String(calibINominal, 2) + ",";
  json += "\"i_mult\":" + String(calibISagFactor, 3) + ",";
  json += "\"i_electrodos\":" + String(corrienteElectrodosA, 2) + ",";
  json += "\"dosis_mgl\":" + String(calibDosisMgL, 1) + ",";
  json += "\"t_max_min\":" + String(calibTiempoMaxMin) + ",";
  json += "\"vol_l\":" + String(calibVolumenL, 1);
  json += "}";
  server.send(200, "application/json", json);
}

void handleApiBitacoraAdd() {
  uint16_t idx = totalBitacoraRows % MAX_BITACORA_ROWS;
  RegistroBitacora& r = bitacoraCampo[idx];
  r.id           = totalBitacoraRows + 1;
  r.uptime_s     = millis() / 1000;
  r.etapa_id     = (uint8_t)estadoActual;
  strncpy(r.etapa_nombre, getNombreEstado(estadoActual), sizeof(r.etapa_nombre));
  r.ssr1_on      = actuadores[2].estado;
  r.nivel_cm     = pt1HeightCm;
  r.litros       = pt1Liters;
  r.psi          = pt1Psi;
  r.v_multimetro = server.hasArg("v_mult") ? server.arg("v_mult").toFloat() : 0.0f;
  r.i_pinza_ac   = server.hasArg("i_ac") ? server.arg("i_ac").toFloat() : 0.0f;
  r.v_p36_pin    = vPinAnodoPasivoV;
  r.v_p36_rod    = vVarillaAnodoPasivoV;
  r.v_on         = vAnodoConFuenteOnV;
  r.v_off_50ms   = vAnodoPausa50msV;
  r.v_off_1400ms = vAnodoPausa1400msV;
  r.dv_ir        = deltaVIrOhmicaV;
  r.i_dc_est     = corrienteElectrodosA;
  r.i_ac_prim    = corrienteAcPrimariaA;
  r.tds_ppm      = (tdsManualIngresado ? tdsManualPPM : tdsEstimadoPPM);

  String acts = "";
  for (int i = 0; i < TOTAL_ACTUADORES; i++) {
    if (actuadores[i].estado) acts += "A" + String(actuadores[i].id) + " ";
  }
  acts.trim();
  strncpy(r.actuadores_str, acts.c_str(), sizeof(r.actuadores_str));

  String notStr = server.hasArg("nota") ? server.arg("nota") : "";
  strncpy(r.nota, notStr.c_str(), sizeof(r.nota));

  totalBitacoraRows++;
  Serial.printf("[BITACORA #%u] Etapa=%s | Nivel=%.1fcm | Mult=%.3fV | P36_Rod=%.3fV | I_DC=%.1fA | Nota=%s\n",
                r.id, r.etapa_nombre, r.nivel_cm, r.v_multimetro, r.v_p36_rod, r.i_dc_est, r.nota);
  server.send(200, "application/json", "{\"ok\":true,\"id\":" + String(r.id) + "}");
}

void handleApiBitacoraGet() {
  String json = "{\"ag_msg\":\"" + jsonEscape(mensajeAntigravity) + "\",\"count\":" + String(totalBitacoraRows) + ",\"rows\":[";
  uint16_t count = min((uint16_t)MAX_BITACORA_ROWS, totalBitacoraRows);
  for (uint16_t i = 0; i < count; i++) {
    const RegistroBitacora& r = bitacoraCampo[i];
    if (i > 0) json += ",";
    json += "{";
    json += "\"id\":" + String(r.id) + ",";
    json += "\"uptime_s\":" + String(r.uptime_s) + ",";
    json += "\"etapa_id\":" + String(r.etapa_id) + ",";
    json += "\"etapa\":\"" + jsonEscape(String(r.etapa_nombre)) + "\",";
    json += "\"ssr1_on\":" + String(r.ssr1_on ? "true" : "false") + ",";
    json += "\"nivel_cm\":" + String(r.nivel_cm, 1) + ",";
    json += "\"litros\":" + String(r.litros, 1) + ",";
    json += "\"psi\":" + String(r.psi, 2) + ",";
    json += "\"v_mult\":" + String(r.v_multimetro, 3) + ",";
    json += "\"i_ac\":" + String(r.i_pinza_ac, 2) + ",";
    json += "\"v_pin\":" + String(r.v_p36_pin, 3) + ",";
    json += "\"v_rod\":" + String(r.v_p36_rod, 3) + ",";
    json += "\"v_on\":" + String(r.v_on, 3) + ",";
    json += "\"v_off_50ms\":" + String(r.v_off_50ms, 3) + ",";
    json += "\"v_off_1400ms\":" + String(r.v_off_1400ms, 3) + ",";
    json += "\"dv_ir\":" + String(r.dv_ir, 3) + ",";
    json += "\"i_dc\":" + String(r.i_dc_est, 1) + ",";
    json += "\"i_ac_prim\":" + String(r.i_ac_prim, 2) + ",";
    json += "\"tds\":" + String(r.tds_ppm, 0) + ",";
    json += "\"actuadores\":\"" + jsonEscape(String(r.actuadores_str)) + "\",";
    json += "\"nota\":\"" + jsonEscape(String(r.nota)) + "\"";
    json += "}";
  }
  json += "]}";
  server.send(200, "application/json", json);
}

void handleApiAgReply() {
  if (server.hasArg("msg")) {
    mensajeAntigravity = server.arg("msg");
    msgAntigravityTs = millis() / 1000;
    // Emitir doble bip corto en el buzzer para avisar en campo si se pide con beep=1
    if (server.hasArg("beep") && server.arg("beep").toInt() == 1) {
      lanzarPulsoActuador(10, 120);
    }
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Falta msg");
  }
}

void handleApiBitacoraCsv() {
  String csv = "id,uptime_s,etapa_id,etapa_nombre,ssr1_on,nivel_cm,litros,psi,v_multimetro_V,i_pinza_AC_A,v_p36_pin_V,v_p36_rod_V,v_on_V,v_off_50ms_V,v_off_1400ms_V,dv_ir_V,i_dc_est_A,i_ac_prim_A,tds_ppm,actuadores,nota\n";
  uint16_t count = min((uint16_t)MAX_BITACORA_ROWS, totalBitacoraRows);
  for (uint16_t i = 0; i < count; i++) {
    const RegistroBitacora& r = bitacoraCampo[i];
    csv += String(r.id) + "," + String(r.uptime_s) + "," + String(r.etapa_id) + ",\"" + String(r.etapa_nombre) + "\",";
    csv += String(r.ssr1_on ? "ON" : "OFF") + "," + String(r.nivel_cm, 1) + "," + String(r.litros, 1) + "," + String(r.psi, 2) + ",";
    csv += String(r.v_multimetro, 3) + "," + String(r.i_pinza_ac, 2) + "," + String(r.v_p36_pin, 3) + "," + String(r.v_p36_rod, 3) + ",";
    csv += String(r.v_on, 3) + "," + String(r.v_off_50ms, 3) + "," + String(r.v_off_1400ms, 3) + "," + String(r.dv_ir, 3) + ",";
    csv += String(r.i_dc_est, 1) + "," + String(r.i_ac_prim, 2) + "," + String(r.tds_ppm, 0) + ",\"" + String(r.actuadores_str) + "\",\"" + String(r.nota) + "\"\n";
  }
  server.send(200, "text/csv", csv);
}

// Configuración de Web OTA (/update)
void setupWebOTA() {
  server.on("/update", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html",
      "<!DOCTYPE html><html lang='es'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'>"
      "<title>Agua de Gía - Actualización OTA</title>"
      "<style>body{background:#090d16;color:#fff;font-family:sans-serif;padding:24px;text-align:center;}"
      ".box{max-width:440px;margin:40px auto;background:#131b2e;padding:26px;border-radius:14px;border:1px solid #1e293b;box-shadow:0 8px 30px rgba(0,0,0,0.5);}"
      "h2{color:#06b6d4;margin-bottom:8px;font-size:20px;}"
      "p{color:#94a3b8;font-size:13px;line-height:1.5;margin-bottom:20px;}"
      "input[type=file]{margin:16px 0;background:#1e293b;color:#e2e8f0;padding:12px;border-radius:8px;width:100%;box-sizing:border-box;border:1px dashed #334155;}"
      "button{background:linear-gradient(135deg,#06b6d4,#0284c7);color:#000;font-weight:800;padding:14px;border:none;border-radius:8px;cursor:pointer;width:100%;font-size:14px;text-transform:uppercase;letter-spacing:0.5px;}"
      "a{display:inline-block;margin-top:16px;color:#38bdf8;text-decoration:none;font-size:12px;}"
      "</style></head><body><div class='box'>"
      "<h2>💧 Actualización Inalámbrica (OTA)</h2>"
      "<p>Selecciona el archivo binario compilado <b>(.bin)</b> para actualizar el ESP32 por Wi-Fi sin usar cables USB.</p>"
      "<form method='POST' action='/update' enctype='multipart/form-data'>"
      "<input type='file' name='update' accept='.bin' required>"
      "<button type='submit'>⚡ ACTUALIZAR FIRMWARE AHORA</button>"
      "</form>"
      "<a href='/'>← Volver al Dashboard SCADA</a>"
      "</div></body></html>");
  });

  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "ERROR AL ACTUALIZAR" : "¡ACTUALIZACION EXITOSA! Reiniciando ESP32...");
    delay(1000);
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("[OTA] Inicio archivo: %s\n", upload.filename.c_str());
      apagarTodosLosActuadores(); // Estado seguro durante la subida
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) {
        Serial.printf("[OTA] Éxito: %u bytes grabados. Reiniciando...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
}

// Configuración de ArduinoOTA (Para subir desde Arduino IDE / CLI por red)
void setupArduinoOTA() {
  ArduinoOTA.setHostname("AguaDeGia-ESP32");
  ArduinoOTA.setPassword("agua_resiliente");

  ArduinoOTA.onStart([]() {
    Serial.println("[ArduinoOTA] Inicio de flasheo inalámbrico...");
    apagarTodosLosActuadores();
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\n[ArduinoOTA] Flasheo terminado con éxito. Reiniciando...");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("[ArduinoOTA] %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("[ArduinoOTA] Error [%u]\n", error);
  });
  ArduinoOTA.begin();
}

// ============================================================================
// 13. SETUP
// ============================================================================
void setup() {
  Serial.begin(115200);

  // Inicialización de Salidas en APAGADO (LOW) con máxima corriente de salida (GPIO_DRIVE_CAP_3 = ~40 mA)
  // para excitar con firmeza los optoacopladores de los SSR Fotek (P33, P27, P26) y MOSFETs Bixolon
  for (int i = 0; i < TOTAL_ACTUADORES; i++) {
    actuadores[i].estado = false;
    actuadores[i].pulsoFinMs = 0;
    digitalWrite(actuadores[i].pin, LOW);
    pinMode(actuadores[i].pin, OUTPUT);
    gpio_set_drive_capability((gpio_num_t)actuadores[i].pin, GPIO_DRIVE_CAP_3);
    digitalWrite(actuadores[i].pin, actuadores[i].esBuzzer ? BUZZER_INACTIVE_LEVEL : RELAY_INACTIVE_LEVEL);
  }

  // Beep inicial de arranque
  digitalWrite(PIN_BUZZER, BUZZER_ACTIVE_LEVEL);
  delay(150);
  digitalWrite(PIN_BUZZER, BUZZER_INACTIVE_LEVEL);

  // Entradas Digitales PULLUP
  pinMode(PIN_NIVEL_ALTO_TOTEM, INPUT_PULLUP);
  pinMode(PIN_PARO_REBOSE_CONO, INPUT_PULLUP);
  // IMPORTANTE: GPIO 22 en alta impedancia (INPUT sin pull-up) por si el usuario conectó ahí
  // la línea de datos del presostato (0.5V-4.5V) y coloca un puente de P22 -> P32 (ADC1_CH4).
  pinMode(PIN_PRESOSTATO_RO,    INPUT);
  pinMode(PIN_TEMP_DS18B20,     INPUT_PULLUP);

  // Caudalímetro Hall
  pinMode(PIN_FLOW_HALL, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_FLOW_HALL), isrCaudalimetro, FALLING);

  // Servo LEDC
  setupServoLEDC();
  escribirServoAngulo(90);

  // Entradas Analógicas ADC1 (12 bits)
  // GPIO 32 en alta impedancia (SIN pull-down) y ganancia máxima ADC_0db (100-950 mV)
  // para amplificar x3.6 la resolución del transductor de 100 PSI en los 1.10 m del tanque (500-563 mV).
  pinMode(PIN_ADC_PRESOSTATO, INPUT);
  pinMode(PIN_ADC_I_ELECTRODOS, INPUT);
  pinMode(PIN_ADC_I_BOMBAS_DC, INPUT);
  pinMode(PIN_ADC_ANODO_PASIVO, INPUT);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  analogSetPinAttenuation(PIN_ADC_PRESOSTATO, ADC_0db);
  analogSetPinAttenuation(PIN_ADC_I_ELECTRODOS, ADC_11db);
  analogSetPinAttenuation(PIN_ADC_I_BOMBAS_DC, ADC_11db);
  analogSetPinAttenuation(PIN_ADC_ANODO_PASIVO, ADC_11db);
  cargarCalibracionPt1Nvs();
  cargarCalibracionElectricaNvs();

  // CONFIGURACIÓN WI-FI DUAL: Access Point + Conexión a Redes de Casa
  // IMPORTANTE en Core 3.x:
  // 1) Desactivar ahorro de energía Wi-Fi (WiFi.setSleep(false)) para que el AP responda al instante.
  // 2) Conectar primero a la red de casa para conocer el canal físico de radio (WiFi.channel()).
  // 3) Llamar a WiFi.softAPConfig() ANTES de WiFi.softAP() para no romper el servidor DHCP del AP,
  //    y levantar el AP en el mismo canal de radio que el router.
  WiFi.persistent(false);
  WiFi.mode(WIFI_AP_STA);
  WiFi.setSleep(false);

  // Registrar redes de casa (Mayor alcance: "la última compuerta" + Red "Lacasita")
  wifiMulti.addAP("la última compuerta", "6646802560");
  wifiMulti.addAP("la ultima compuerta", "6646802560");
  wifiMulti.addAP("Lacasita", "6646802560");
  wifiMulti.addAP("lacasita", "6646802560");

  Serial.println("\n[WIFI] Conectando a red de casa (la última compuerta / Lacasita)...");
  int canalAP = 1;
  if (wifiMulti.run(6000) == WL_CONNECTED) {
    canalAP = (WiFi.channel() > 0) ? WiFi.channel() : 1;
    Serial.printf("[WIFI] ¡Conectado con éxito a red de casa: %s (Canal %d)!\n", WiFi.SSID().c_str(), canalAP);
    Serial.printf("[WIFI] IP en red local de casa: http://%s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("[WIFI] Sin red de casa por ahora. Iniciando AP local en canal 1.");
  }

  // Configurar IP del AP ANTES de iniciar softAP (preserva servidor DHCP activo en 192.168.4.1)
  WiFi.softAPConfig(apIP, apGateway, apSubnet);
  bool apOk = WiFi.softAP(AP_SSID, AP_PASS, canalAP, 0, 4);
  delay(100);
  WiFi.setSleep(false);

  // Servidor DNS Captive Portal (redirige cualquier dominio en el AP hacia 192.168.4.1)
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(53, "*", WiFi.softAPIP());

  Serial.printf("[WIFI] Punto de Acceso (AP) Local (%s, Canal %d):\n", apOk ? "ACTIVO" : "ERROR", canalAP);
  Serial.printf("       SSID:     %s\n", AP_SSID);
  Serial.printf("       Password: %s\n", AP_PASS);
  Serial.printf("       IP AP:    http://%s\n", WiFi.softAPIP().toString().c_str());

  // Inicializar OTA
  setupArduinoOTA();
  setupWebOTA();

  // Servidor Web Rutas + Captive Portal para celulares
  server.on("/", HTTP_GET, handleRoot);
  server.on("/generate_204", HTTP_GET, handleRoot);
  server.on("/hotspot-detect.html", HTTP_GET, handleRoot);
  server.on("/ncsi.txt", HTTP_GET, handleRoot);
  server.on("/api/status", HTTP_GET, handleApiStatus);
  server.on("/api/relay", HTTP_POST, handleApiRelay);
  server.on("/api/pulse", HTTP_POST, handleApiPulse);
  server.on("/api/servo", HTTP_POST, handleApiServo);
  server.on("/api/auto", HTTP_POST, handleApiAuto);
  server.on("/api/emergency", HTTP_POST, handleApiEmergency);
  server.on("/api/sample_anode", HTTP_POST, handleApiSampleAnode);
  server.on("/api/pt1/cal", HTTP_POST, handleApiPt1Cal);
  server.on("/api/calib", handleApiCalib);
  server.on("/api/bitacora_add", HTTP_POST, handleApiBitacoraAdd);
  server.on("/api/bitacora", HTTP_GET, handleApiBitacoraGet);
  server.on("/api/bitacora.csv", HTTP_GET, handleApiBitacoraCsv);
  server.on("/api/ag_reply", HTTP_POST, handleApiAgReply);
  server.onNotFound(handleRoot);

  server.begin();
  Serial.println("[HTTP] Servidor Web SCADA activo en puerto 80");
  Serial.println("[OTA]  Actualización Web disponible en /update");
}

// ============================================================================
// 14. LOOP PRINCIPAL
// ============================================================================
void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
  procesarTemporizadoresPulsos();
  procesarMuestreoAnodoPasivo();

  // Atención a ArduinoOTA inalámbrico
  ArduinoOTA.handle();

  // Mantener y reconectar Wi-Fi de casa ("la ultima compuerta" / "Lacasita") automáticamente si regresa el internet
  static uint32_t ultimoCheckWifiMs = 0;
  if (millis() - ultimoCheckWifiMs >= 20000) {
    ultimoCheckWifiMs = millis();
    if (WiFi.status() != WL_CONNECTED) {
      if (wifiMulti.run(4000) == WL_CONNECTED) {
        int nuevoCanal = (WiFi.channel() > 0) ? WiFi.channel() : 1;
        WiFi.softAP(AP_SSID, AP_PASS, nuevoCanal, 0, 4);
        WiFi.setSleep(false);
        Serial.printf("[WIFI] Reconectado a %s | IP: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
      }
    }
  }

  static uint32_t ultimoMuestreoMs = 0;
  if (millis() - ultimoMuestreoMs >= 100) {
    ultimoMuestreoMs = millis();
    actualizarLecturasSensores();
  }

  procesarAutomataMaestro();
}
