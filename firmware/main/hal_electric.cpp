#include "hal_electric.h"
#include <esp_arduino_version.h>
#include <driver/gpio.h>
#include <LittleFS.h>
#include <Preferences.h>
#include <math.h>

// ============================================================================
// 1. TABLA OFICIAL DE CANALES DE POTENCIA — NODO 2 ("AGUA DE GIA")
// ============================================================================
static ActuatorChannel s_actuators[ACT_COUNT] = {
  // id,            pin,                  iecCode,      boardLocation,           loadDesc,                                                     activeLow, state, pulseEndMs
  { ACT_SSR1_40A,   PIN_SSR1_CELDA_40A,   "SSR1 Fotek", "SSR Fotek #1 (P33)",    "Fuente ATX 15A - Electrofloculación",                        false,     false, 0 },
  { ACT_K1_M1,      PIN_K1_BOMBA_POZO_M1, "BIX2-M2",    "Bixolon #2 · MOSFET 2", "Bomba Evans 24V DC / Timer ASL-52 (P14)",                    false,     false, 0 },
  { ACT_K2_YV1,     PIN_K2_SOL_LLENADO,   "SSR2 Fotek", "SSR Fotek #2 (P27)",    "Solenoide de Bronce 3/4\" 120V AC - Llenado",                false,     false, 0 },
  { ACT_K3_M2,      PIN_K3_WHIRLPOOL_M2,  "SSR3 Fotek", "SSR Fotek #3 (P26)",    "Bomba Pecera 120V AC - Whirlpool / Lavado (P26)",            false,     false, 0 },
  { ACT_K4_M3,      PIN_K4_TRASVASE_M3,   "BIX2-M1",    "Bixolon #2 · MOSFET 1", "Relé Automotriz 30A / Bomba Diafragma #1 Trasvase (P25)",    false,     false, 0 },
  { ACT_K5_M4_UV1,  PIN_K5_MANIFOLD_UV,   "RL3",        "Relé Lógico Canal #3",  "Bomba de Diafragma #2 Manifold 12V DC (P23)",                false,     false, 0 },
  { ACT_K6_YV2,     PIN_K6_SOL_CASA_60L,  "RL2",        "Relé Lógico Canal #2",  "Solenoide de Bronce 3/4\" 120V AC - Casa (P15)",             false,     false, 0 },
  { ACT_K7_YV3,     PIN_K7_SOL_PRE_RO,    "BIX1-M1",    "Bixolon #1 · MOSFET 1", "Solenoide 1/2\" Pre-RO 12V DC (P5)",                         false,     false, 0 },
  { ACT_K8_YV4,     PIN_K8_SOL_BYPASS_RO, "BIX1-M2",    "Bixolon #1 · MOSFET 2", "Solenoide 1/2\" Bypass RO 12V DC (P13)",                     false,     false, 0 },
  { ACT_BZ1_BELL,   PIN_BZ1_CAMPANA,      "BZ1",        "Alarma Acústica (P17)", "Buzzer / Alarma acústica de panel",                          false,     false, 0 }
};

static MultimeterTelemetry s_telemetry;

// Variables internas de ISR Caudalímetro Hall (FT1 - GPIO 18)
static volatile uint32_t s_hallPulseCounter = 0;
static uint32_t s_lastFlowCalcMs            = 0;
static uint32_t s_lastPulseSnapshot         = 0;

// Configuración PWM Servo MG996R SRV1 (GPIO 16)
static const uint32_t SERVO_PWM_FREQ_HZ     = 50;
static const uint8_t  SERVO_PWM_RES_BITS    = 14;
static const uint8_t  SERVO_PWM_CHANNEL     = 0;

// Máquina no bloqueante para repique de campana BZ1 ("Ding-Ding!")
static uint8_t  s_bellRingStep              = 0;
static uint32_t s_bellStepTimerMs           = 0;

// Temporizadores de muestreo
static uint32_t s_lastAdcSampleMs           = 0;
static uint32_t s_lastTempSampleMs          = 0;

// Variables de Control y Calibración del Sensor de Corriente #1 (CT1 - GPIO 34)
static bool  s_electrodeMeasurementEnabled  = true;   // Habilitado por defecto para el integrador de Faraday
static float s_ct1ZeroVolts                 = 1.65f;  // Punto medio en reposo del ACS712
static float s_ct1Sensitivity               = 0.185f; // Sensibilidad 185 mV/A (ACS712-05B en entrada 120V AC ATX)

// Variables de Alta Resolución y Calibración del Transductor de Presión PT1 (100 PSI en Tanque 110 cm)
// Cono inferior: 35 cm (15.79 L) + Cilindro: 75 cm (101.51 L) = 110 cm totales (117.3 L)
static Preferences s_pt1Prefs;
static float s_pt1ZeroMv                    = PT1_DEFAULT_ZERO_MV;   // 500.00 mV en vacío (0 cm)
static float s_pt1MvPerCm                   = PT1_DEFAULT_MV_PER_CM; // 0.56893 mV/cm H2O
static float s_pt1FilteredMv                = 0.0f;
static float s_pt1EfuseOffsetMv             = 75.0f;
static bool  s_pt1EfuseReady                = false;

static void savePt1CalToNvs() {
  if (s_pt1Prefs.begin("pt1cal", false)) {
    s_pt1Prefs.putFloat("zeroMv",  s_pt1ZeroMv);
    s_pt1Prefs.putFloat("mvPerCm", s_pt1MvPerCm);
    s_pt1Prefs.end();
  }
}

static void loadPt1CalFromNvs() {
  s_pt1ZeroMv  = PT1_DEFAULT_ZERO_MV;
  s_pt1MvPerCm = PT1_DEFAULT_MV_PER_CM;
  if (s_pt1Prefs.begin("pt1cal", true)) {
    float z = s_pt1Prefs.getFloat("zeroMv",  PT1_DEFAULT_ZERO_MV);
    float s = s_pt1Prefs.getFloat("mvPerCm", PT1_DEFAULT_MV_PER_CM);
    s_pt1Prefs.end();
    if (!isnan(z) && z >= 200.0f && z <= 1200.0f) s_pt1ZeroMv = z;
    if (!isnan(s) && s >= 0.10f  && s <= 15.0f)   s_pt1MvPerCm = s;
  }
}

float halVolumeFromHeightCm(float heightCm) {
  float h = constrain(heightCm, 0.0f, TANK_TOTAL_HEIGHT_CM);
  if (h <= 0.0f) return 0.0f;
  if (h <= TANK_CONE_HEIGHT_CM) {
    // Sección cónica inferior (0 a 35 cm): V(h) = V_cono * (h / 35)^3
    float ratio = h / TANK_CONE_HEIGHT_CM;
    return TANK_CONE_VOLUME_LITERS * ratio * ratio * ratio;
  }
  // Sección cilíndrica (35 a 110 cm): V(h) = 15.7904 L + 1.35346 L/cm * (h - 35 cm)
  return constrain(TANK_CONE_VOLUME_LITERS + TANK_CYL_LITERS_PER_CM * (h - TANK_CONE_HEIGHT_CM),
                   0.0f, VOLUMEN_NOMINAL_1BBL_L);
}

// ============================================================================
// 2. INTERRUPCIÓN EN IRAM PARA CAUDALÍMETRO HALL (FT1 - GPIO 18)
// ============================================================================
void IRAM_ATTR isrHallFlowSensor() {
  s_hallPulseCounter++;
}

// ============================================================================
// 3. FUNCIÓN AUXILIAR DE ESCRITURA FÍSICA CON INVERSIÓN DE LÓGICA
// ============================================================================
static void writePhysicalPin(const ActuatorChannel& ch) {
  bool levelHigh = ch.activeLow ? (!ch.state) : (ch.state);
  digitalWrite(ch.pin, levelHigh ? HIGH : LOW);
}

// ============================================================================
// 4. LECTURA ONEWIRE NATIVA PARA DS18B20 EN TT1 (GPIO 4) SIN LIBRERÍAS EXTERNAS
// ============================================================================
static bool ds18b20Reset(uint8_t pin) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  delayMicroseconds(480);
  pinMode(pin, INPUT_PULLUP);
  delayMicroseconds(70);
  bool presence = (digitalRead(pin) == LOW);
  delayMicroseconds(410);
  return presence;
}

static void ds18b20WriteBit(uint8_t pin, uint8_t bitVal) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  if (bitVal) {
    delayMicroseconds(6);
    pinMode(pin, INPUT_PULLUP);
    delayMicroseconds(64);
  } else {
    delayMicroseconds(60);
    pinMode(pin, INPUT_PULLUP);
    delayMicroseconds(10);
  }
}

static uint8_t ds18b20ReadBit(uint8_t pin) {
  uint8_t bitVal = 0;
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  delayMicroseconds(3);
  pinMode(pin, INPUT_PULLUP);
  delayMicroseconds(10);
  if (digitalRead(pin) == HIGH) {
    bitVal = 1;
  }
  delayMicroseconds(50);
  return bitVal;
}

static void ds18b20WriteByte(uint8_t pin, uint8_t data) {
  for (uint8_t i = 0; i < 8; i++) {
    ds18b20WriteBit(pin, (data >> i) & 0x01);
  }
}

static uint8_t ds18b20ReadByte(uint8_t pin) {
  uint8_t val = 0;
  for (uint8_t i = 0; i < 8; i++) {
    if (ds18b20ReadBit(pin)) {
      val |= (1 << i);
    }
  }
  return val;
}

static bool s_ds18b20ConversionStarted = false;

static float readWaterTempTT1() {
  if (!s_ds18b20ConversionStarted) {
    if (ds18b20Reset(PIN_TT1_TEMP_DS18B20)) {
      ds18b20WriteByte(PIN_TT1_TEMP_DS18B20, 0xCC);
      ds18b20WriteByte(PIN_TT1_TEMP_DS18B20, 0x44);
      s_ds18b20ConversionStarted = true;
    }
    return s_telemetry.tt1TempC > 0.1f ? s_telemetry.tt1TempC : 22.8f;
  } else {
    s_ds18b20ConversionStarted = false;
    if (ds18b20Reset(PIN_TT1_TEMP_DS18B20)) {
      ds18b20WriteByte(PIN_TT1_TEMP_DS18B20, 0xCC);
      ds18b20WriteByte(PIN_TT1_TEMP_DS18B20, 0xBE);
      uint8_t lsb = ds18b20ReadByte(PIN_TT1_TEMP_DS18B20);
      uint8_t msb = ds18b20ReadByte(PIN_TT1_TEMP_DS18B20);
      int16_t raw = (int16_t)((msb << 8) | lsb);
      float tempC = (float)raw / 16.0f;
      if (tempC >= -10.0f && tempC <= 85.0f && raw != -1) {
        return tempC;
      }
    }
    return 22.8f;
  }
}

// ============================================================================
// 5. LECTURA MULTIMUESTREO ADC1 Y MEDICIÓN AC/DC EN GPIO 34
// ============================================================================
static uint16_t readAdcOversampled(uint8_t pin, int samples = 16) {
  uint32_t acc = 0;
  for (int i = 0; i < samples; i++) {
    acc += analogRead(pin);
  }
  return (uint16_t)(acc / samples);
}

// Muestrea GPIO 34 (Pin 5 - Electrodo Activo con divisor 4:1) y calcula voltaje y corriente
static void sampleCt1AcAndDcCurrent() {
  const int N = 32;
  uint32_t sumRaw = 0;
  uint32_t sumMv  = 0;
  for (int i = 0; i < N; i++) {
    sumRaw += analogRead(PIN_CT1_CELDA_E1);
    sumMv  += analogReadMilliVolts(PIN_CT1_CELDA_E1);
    delayMicroseconds(250);
  }
  uint16_t meanRaw = (uint16_t)(sumRaw / N);
  float pinV = ((float)sumMv / (float)N) / 1000.0f;

  s_telemetry.ct1Raw    = meanRaw;
  s_telemetry.ct1Volts  = pinV;
  float cellV           = pinV * FACTOR_DIVISOR_CELDA_P34;
  s_telemetry.cellVolts = cellV;

  // Si se detecta voltaje en la celda o el actuador de electrofloculación está encendido:
  if (cellV > 2.0f || s_actuators[ACT_SSR1_40A].state) {
    // Estimación por caída de tensión de la fuente ATX 12V bajo carga
    if (cellV < V_VACIO_FUENTE_ATX_V) {
      float dropV = V_VACIO_FUENTE_ATX_V - cellV;
      s_telemetry.ct1Amps = constrain(dropV / R_INTERNA_FUENTE_ATX_OHMS, 0.0f, 25.0f);
    } else {
      s_telemetry.ct1Amps = 0.0f; // En vacío / sin carga
    }
    s_telemetry.ct1AcAmps = s_telemetry.ct1Amps;
  } else {
    s_telemetry.ct1Amps   = 0.0f;
    s_telemetry.ct1AcAmps = 0.0f;
  }

  s_telemetry.ct1FeGramPerSec = s_telemetry.ct1Amps * FARADAY_FE_G_PER_AMP_SEC;
  if (s_telemetry.ct1Amps >= CORRIENTE_TOQUE_ANODO_A) {
    float rCeldaOhms = (s_telemetry.ct1Amps > 0.01f) ? (cellV / s_telemetry.ct1Amps) : 999.0f;
    s_telemetry.ct1PpmInstant = constrain(K_CELDA_PPM_OHMS / rCeldaOhms, 80.0f, 8000.0f);
  } else {
    s_telemetry.ct1PpmInstant = 0.0f;
  }
}

// Asegura cabecera del archivo CSV en LittleFS
static void ensurePassiveCsvHeader() {
  if (!s_telemetry.littleFsReady) return;
  if (!LittleFS.exists(PASSIVE_ANODE_CSV_PATH)) {
    File f = LittleFS.open(PASSIVE_ANODE_CSV_PATH, FILE_WRITE);
    if (f) {
      f.println("uptime_s,etapa_s,v_anodo_pasivo_vp,v_pin36,i_ac_atx_a,i_dc_celda_a,coulombs_c,hierro_g,nivel_pct,litros_l,presion_psi,temp_c");
      f.close();
    }
  }
}

// Muestreo de ultra-alta resolución para el Transductor de Presión PT1 (100 PSI en Tanque de 110 cm)
// Usa ADC_0db (rango 100-950 mV -> ~0.20 mV por bit) + sobremuestreo 256x + calibración continua eFuse
static void samplePt1HighRes() {
  // 1. Breve descarga de capacitancia parásita en GPIO 32 para detectar si el pin está abierto (0V)
  pinMode(PIN_PT1_NIVEL_CONO, INPUT_PULLDOWN);
  delayMicroseconds(15);
  pinMode(PIN_PT1_NIVEL_CONO, INPUT);
  delayMicroseconds(30);

  // 2. Sobremuestreo 256x en GPIO 32 (ADC_0db)
  const int N_SAMPLES = 256;
  uint32_t sumRaw = 0;
  uint32_t sumMv  = 0;
  uint16_t minRaw = 4095;
  uint16_t maxRaw = 0;

  for (int i = 0; i < N_SAMPLES; i++) {
    uint16_t r = analogRead(PIN_PT1_NIVEL_CONO);
    sumRaw += r;
    if (r < minRaw) minRaw = r;
    if (r > maxRaw) maxRaw = r;
    if ((i & 7) == 0) {
      sumMv += analogReadMilliVolts(PIN_PT1_NIVEL_CONO);
    }
  }

  float rawAvg   = (float)sumRaw / (float)N_SAMPLES;
  float calMvAvg = (float)sumMv  / 32.0f;
  uint16_t ppRaw = (maxRaw >= minRaw) ? (maxRaw - minRaw) : 0;

  uint8_t activePin = PIN_PT1_NIVEL_CONO;
  // Con divisor de voltaje en P32, umbral de conexion >= 80 mV
  bool connected = (calMvAvg >= 80.0f && ppRaw < 450);

  s_telemetry.pt1Pin       = activePin;
  s_telemetry.pt1Connected = connected;
  s_telemetry.pt1ZeroMv    = s_pt1ZeroMv;
  s_telemetry.pt1MvPerCm   = s_pt1MvPerCm;

  if (!connected) {
    s_telemetry.pt1Raw      = (uint16_t)(rawAvg + 0.5f);
    s_telemetry.pt1RawFloat = rawAvg;
    s_telemetry.pt1Mv       = calMvAvg;
    s_telemetry.pt1Volts    = calMvAvg / 1000.0f;
    s_telemetry.pt1Psi      = 0.0f;
    s_telemetry.pt1HeightCm = 0.0f;
    s_telemetry.pt1LevelPct = 0.0f;
    s_telemetry.pt1Liters   = 0.0f;
    return;
  }

  // 3. Fusión sub-milivoltio: usa la resolución continua de 12 bits x 256 (0.1985 mV/cuenta en ADC_0db)
  //    anclada a la calibración de fábrica eFuse (analogReadMilliVolts) sin escalones enteros de 1 mV
  const float ADC_0DB_SLOPE_MV = 0.1985f;
  float instOffsetMv = calMvAvg - (rawAvg * ADC_0DB_SLOPE_MV);
  if (!s_pt1EfuseReady) {
    s_pt1EfuseOffsetMv = instOffsetMv;
    s_pt1EfuseReady    = true;
  } else {
    s_pt1EfuseOffsetMv = 0.95f * s_pt1EfuseOffsetMv + 0.05f * instOffsetMv;
  }
  float highResMv = (rawAvg * ADC_0DB_SLOPE_MV) + s_pt1EfuseOffsetMv;

  if (s_pt1FilteredMv < 150.0f) {
    s_pt1FilteredMv = highResMv;
  } else {
    s_pt1FilteredMv = 0.25f * highResMv + 0.75f * s_pt1FilteredMv;
  }

  s_telemetry.pt1Raw      = (uint16_t)(rawAvg + 0.5f);
  s_telemetry.pt1RawFloat = rawAvg;
  s_telemetry.pt1Mv       = s_pt1FilteredMv;
  s_telemetry.pt1Volts    = s_pt1FilteredMv / 1000.0f;

  // 4. Conversión de mV a altura de columna de agua (0..110 cm), PSI y Litros (Cono 35 cm + Cilindro 75 cm)
  float deltaMv = s_pt1FilteredMv - s_pt1ZeroMv;
  if (deltaMv <= 0.12f) {
    s_telemetry.pt1HeightCm = 0.0f;
    s_telemetry.pt1Psi      = 0.0f;
    s_telemetry.pt1LevelPct = 0.0f;
    s_telemetry.pt1Liters   = 0.0f;
  } else {
    float hCm = constrain(deltaMv / s_pt1MvPerCm, 0.0f, TANK_TOTAL_HEIGHT_CM);
    s_telemetry.pt1HeightCm = hCm;
    s_telemetry.pt1Psi      = hCm * PT1_PSI_PER_CM_H2O;
    s_telemetry.pt1LevelPct = constrain((hCm / TANK_TOTAL_HEIGHT_CM) * 100.0f, 0.0f, 100.0f);
    s_telemetry.pt1Liters   = halVolumeFromHeightCm(hCm);
  }
}

// ============================================================================
// 6. INICIALIZACIÓN DE HARDWARE (HAL INIT)
// ============================================================================
void halInit() {
  // 1. Inicializar los 10 canales de potencia en estado APAGADO seguro con máxima capacidad de corriente (40 mA)
  for (uint8_t i = 0; i < ACT_COUNT; i++) {
    pinMode(s_actuators[i].pin, OUTPUT);
    gpio_set_drive_capability((gpio_num_t)s_actuators[i].pin, GPIO_DRIVE_CAP_3);
    s_actuators[i].state = false;
    s_actuators[i].pulseEndMs = 0;
    writePhysicalPin(s_actuators[i]);
  }

  // 2. Configurar entradas digitales de nivel (GPIO 19, 21) y OneWire (GPIO 4)
  // IMPORTANTE: GPIO 22 se deja en INPUT (alta impedancia, SIN PULLUP) para no distorsionar el voltaje
  // de 0.5V del presostato si el usuario lo conectó en GPIO 22 y pone un puente hacia GPIO 32 (ADC1).
  pinMode(PIN_LSH01_TOTEM_ALTO, INPUT_PULLUP);
  pinMode(PIN_LSHH02_REBOSE,    INPUT_PULLUP);
  pinMode(PIN_PSL03_DEMANDA_RO, INPUT);
  pinMode(PIN_TT1_TEMP_DS18B20, INPUT_PULLUP);

  // 3. Configurar Caudalímetro Hall FT1 (GPIO 18) con interrupción por flanco de bajada
  pinMode(PIN_FT1_CAUDAL_HALL, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_FT1_CAUDAL_HALL), isrHallFlowSensor, FALLING);

  // 4. Configurar resolución del ADC1 a 12 bits (0..4095)
  //    PIN_PT1_NIVEL_CONO (GPIO 32) usa ADC_0db (100..950 mV) para triplicar la resolución
  //    del presostato de 100 PSI en el rango de 1.10 m de agua (500.0 mV a 562.6 mV)
  analogReadResolution(12);
  analogSetPinAttenuation(PIN_PT1_NIVEL_CONO,  ADC_0db);  // GPIO 32: Ganancia máxima ADC_0db para PT1 100 PSI
  analogSetPinAttenuation(PIN_CT1_CELDA_E1,    ADC_11db); // GPIO 34: Sensor de Corriente #1 (AC 120V fuente ATX)
  analogSetPinAttenuation(PIN_CT2_MOTORES_DC,  ADC_11db); // GPIO 35: Sensor de Corriente #2 (Bombas Diafragma DC)
  analogSetPinAttenuation(PIN_ANODO_PASIVO_VP, ADC_11db); // GPIO 36 (VP): Ánodo Pasivo (divisor 2x10kOhm)

  pinMode(PIN_PT1_NIVEL_CONO,  INPUT);
  pinMode(PIN_CT1_CELDA_E1,    INPUT);
  pinMode(PIN_CT2_MOTORES_DC,  INPUT);
  pinMode(PIN_ANODO_PASIVO_VP, INPUT);

  loadPt1CalFromNvs();

  // 5. Configurar PWM de 50 Hz para Servomotor MG996R (GPIO 16)
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  ledcAttach(PIN_SRV1_AGUJA_PWM, SERVO_PWM_FREQ_HZ, SERVO_PWM_RES_BITS);
#else
  ledcSetup(SERVO_PWM_CHANNEL, SERVO_PWM_FREQ_HZ, SERVO_PWM_RES_BITS);
  ledcAttachPin(PIN_SRV1_AGUJA_PWM, SERVO_PWM_CHANNEL);
#endif
  halSetServoAngle(SERVO_ANGULO_REPOSO);

  // 6. Inicializar estructura de telemetría y montar sistema de archivos LittleFS
  memset(&s_telemetry, 0, sizeof(s_telemetry));
  s_telemetry.tt1TempC     = 22.8f;
  s_telemetry.srv1AngleDeg = SERVO_ANGULO_REPOSO;
  s_telemetry.pt1Pin       = PIN_PT1_NIVEL_CONO;
  s_telemetry.pt1ZeroMv    = s_pt1ZeroMv;
  s_telemetry.pt1MvPerCm   = s_pt1MvPerCm;
  s_telemetry.lastSampledPassiveVolts = 0.00f;

  s_telemetry.littleFsReady = LittleFS.begin(true);
  if (s_telemetry.littleFsReady) {
    ensurePassiveCsvHeader();
    Serial.println("[LittleFS] Sistema de archivos montado. CSV: /anodo_pasivo.csv listo.");
  } else {
    Serial.println("[LittleFS] Advertencia: no se pudo montar LittleFS.");
  }

  s_lastFlowCalcMs   = millis();
  s_lastAdcSampleMs  = millis();
  s_lastTempSampleMs = millis();
}

// ============================================================================
// 7. ACTUALIZACIÓN PERIÓDICA NO BLOQUEANTE (HAL UPDATE)
// ============================================================================
void halUpdate() {
  uint32_t now = millis();

  // A. Expiración automática de pulsos de 3 segundos en el Banco de Pruebas
  for (uint8_t i = 0; i < ACT_COUNT; i++) {
    if (s_actuators[i].pulseEndMs != 0 && (int32_t)(now - s_actuators[i].pulseEndMs) >= 0) {
      s_actuators[i].pulseEndMs = 0;
      s_actuators[i].state = false;
      writePhysicalPin(s_actuators[i]);
    }
  }

  // B. Secuencia no bloqueante de campana de fin de ciclo ("Ding-Ding!")
  if (s_bellRingStep > 0 && (int32_t)(now - s_bellStepTimerMs) >= 0) {
    switch (s_bellRingStep) {
      case 1:
        halSetActuator(ACT_BZ1_BELL, true);
        s_bellStepTimerMs = now + 220;
        s_bellRingStep = 2;
        break;
      case 2:
        halSetActuator(ACT_BZ1_BELL, false);
        s_bellStepTimerMs = now + 140;
        s_bellRingStep = 3;
        break;
      case 3:
        halSetActuator(ACT_BZ1_BELL, true);
        s_bellStepTimerMs = now + 380;
        s_bellRingStep = 4;
        break;
      default:
        halSetActuator(ACT_BZ1_BELL, false);
        s_bellRingStep = 0;
        break;
    }
  }

  // C. Lectura de sensores digitales de nivel (GPIO 19 / 21 / 22)
  s_telemetry.lsh01Closed  = (digitalRead(PIN_LSH01_TOTEM_ALTO) == LOW);
  s_telemetry.lshh02Closed = (digitalRead(PIN_LSHH02_REBOSE) == LOW);
  s_telemetry.psl03Closed  = (digitalRead(PIN_PSL03_DEMANDA_RO) == LOW);

  // D. Muestreo de entradas analógicas ADC1 cada 200 ms
  if (now - s_lastAdcSampleMs >= 200) {
    s_lastAdcSampleMs = now;

    // 1. CT1 (GPIO 34): Sensor de Corriente #1 (Corriente AC 120V en entrada de fuente ATX -> estima I_DC)
    sampleCt1AcAndDcCurrent();

    // 2. CT2 (GPIO 35): Sensor de Corriente #2 (Corriente de bombas de diafragma DC)
    s_telemetry.ct2Raw   = readAdcOversampled(PIN_CT2_MOTORES_DC, 16);
    s_telemetry.ct2Volts = ((float)s_telemetry.ct2Raw / 4095.0f) * 3.30f;
    if (s_telemetry.ct2Volts < 0.06f) {
      s_telemetry.ct2Amps = 0.0f;
    } else {
      s_telemetry.ct2Amps = (s_telemetry.ct2Volts / 3.30f) * 10.0f;
    }
    s_telemetry.ct2DeadHead = (s_telemetry.ct2Amps >= CORRIENTE_DEADHEAD_DC_A);

    // 3. PT1 (GPIO 32 / VP): Transductor de Presión 100 PSI con Alta Resolución ADC_0db (256x)
    //    Geometría: 110 cm totales (Cono 35 cm = 15.79 L + Cilindro 75 cm = 101.51 L -> 117.3 L)
    samplePt1HighRes();

    // 4. VP / Ánodo Pasivo (GPIO 36): Si no está instalado aún, se lee sin generar alertas
    if (s_telemetry.pt1Pin != PIN_ANODO_PASIVO_VP) {
      s_telemetry.passiveRaw      = readAdcOversampled(PIN_ANODO_PASIVO_VP, 16);
      s_telemetry.passivePinVolts = ((float)s_telemetry.passiveRaw / 4095.0f) * 3.30f;
      s_telemetry.passiveVolts    = s_telemetry.passivePinVolts * FACTOR_DIVISOR_VP_2X10K;
    } else {
      s_telemetry.passiveRaw      = 0;
      s_telemetry.passivePinVolts = 0.0f;
      s_telemetry.passiveVolts    = 0.0f;
    }
  }

  // E. Cálculo de Caudalímetro Hall FT1 (GPIO 18) cada 500 ms
  if (now - s_lastFlowCalcMs >= 500) {
    uint32_t elapsedMs = now - s_lastFlowCalcMs;
    s_lastFlowCalcMs = now;

    noInterrupts();
    uint32_t currentPulses = s_hallPulseCounter;
    interrupts();

    uint32_t deltaPulses = currentPulses - s_lastPulseSnapshot;
    s_lastPulseSnapshot = currentPulses;

    s_telemetry.ft1Pulses = currentPulses;
    float freqHz = ((float)deltaPulses * 1000.0f) / (float)elapsedMs;
    s_telemetry.ft1FlowLpm = freqHz / 7.5f;
    s_telemetry.ft1TotalLiters += ((float)deltaPulses / 450.0f);
  }

  // F. Lectura de Temperatura TT1 (GPIO 4) cada 1000 ms
  if (now - s_lastTempSampleMs >= 1000) {
    s_lastTempSampleMs = now;
    s_telemetry.tt1TempC = readWaterTempTT1();
  }
}

// ============================================================================
// 8. MUESTREO GALVÁNICO EN PAUSA DE 1.5s (GPIO 36 / VP) Y REGISTRO LITTLEFS CSV
// ============================================================================
float halSampleAndLogPassiveAnode(uint32_t stageElapsedSec, float coulombs, float feGrams) {
  // Promedio de 64 lecturas con el SSR Fotek #1 (P33) apagado para máxima limpieza de señal
  uint32_t accMv = 0;
  for (int i = 0; i < 64; i++) {
    accMv += analogReadMilliVolts(PIN_ANODO_PASIVO_VP);
  }
  float pinVolts = ((float)accMv / 64.0f) / 1000.0f;
  float galvanicVolts = pinVolts * FACTOR_DIVISOR_VP_2X10K;

  s_telemetry.passivePinVolts         = pinVolts;
  s_telemetry.passiveVolts            = galvanicVolts;
  s_telemetry.lastSampledPassiveVolts = galvanicVolts;
  s_telemetry.passiveSampleCount++;

  if (s_telemetry.littleFsReady) {
    ensurePassiveCsvHeader();
    File f = LittleFS.open(PASSIVE_ANODE_CSV_PATH, FILE_APPEND);
    if (f) {
      f.printf("%lu,%lu,%.4f,%.4f,%.3f,%.2f,%.1f,%.4f,%.1f,%.1f,%.1f,%.1f\n",
               (unsigned long)(millis() / 1000UL),
               (unsigned long)stageElapsedSec,
               galvanicVolts,
               pinVolts,
               s_telemetry.ct1AcAmps,
               s_telemetry.ct1Amps,
               coulombs,
               feGrams,
               s_telemetry.pt1LevelPct,
               s_telemetry.pt1Liters,
               s_telemetry.pt1Psi,
               s_telemetry.tt1TempC);
      f.close();
    }
  }

  Serial.printf("[ANODO PASIVO GPIO36] t=%lus | V_galvanico=%.3fV (Pin36=%.3fV) | Q=%.1fC | Fe=%.3fg\n",
                (unsigned long)stageElapsedSec, galvanicVolts, pinVolts, coulombs, feGrams);
  return galvanicVolts;
}

bool halClearPassiveAnodeCsv() {
  s_telemetry.passiveSampleCount = 0;
  if (!s_telemetry.littleFsReady) return false;
  LittleFS.remove(PASSIVE_ANODE_CSV_PATH);
  ensurePassiveCsvHeader();
  return true;
}

// ============================================================================
// 9. FUNCIONES DE CONTROL DE BOBINAS, POLARIDAD Y SERVOMOTOR MG996R
// ============================================================================
void halSetActuator(uint8_t id, bool turnOn) {
  if (id >= ACT_COUNT) return;
  s_actuators[id].state = turnOn;
  s_actuators[id].pulseEndMs = 0;
  writePhysicalPin(s_actuators[id]);
}

bool halIsActuatorOn(uint8_t id) {
  if (id >= ACT_COUNT) return false;
  return s_actuators[id].state;
}

void halPulseActuator(uint8_t id, uint32_t durationMs) {
  if (id >= ACT_COUNT) return;
  s_actuators[id].state = true;
  s_actuators[id].pulseEndMs = millis() + durationMs;
  writePhysicalPin(s_actuators[id]);
}

void halSetActuatorPolarity(uint8_t id, bool activeLow) {
  if (id >= ACT_COUNT) return;
  s_actuators[id].activeLow = activeLow;
  writePhysicalPin(s_actuators[id]);
}

void halTurnOffAllActuators() {
  for (uint8_t i = 0; i < ACT_COUNT; i++) {
    s_actuators[i].state = false;
    s_actuators[i].pulseEndMs = 0;
    writePhysicalPin(s_actuators[i]);
  }
}

void halRingCycleEndBell() {
  s_bellRingStep = 1;
  s_bellStepTimerMs = millis();
}

void halSetServoAngle(int angleDeg) {
  angleDeg = constrain(angleDeg, 0, 180);
  s_telemetry.srv1AngleDeg = angleDeg;
  uint32_t duty = map(angleDeg, 0, 180, 410, 2048);
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  ledcWrite(PIN_SRV1_AGUJA_PWM, duty);
#else
  ledcWrite(SERVO_PWM_CHANNEL, duty);
#endif
}

int halGetServoAngle() {
  return s_telemetry.srv1AngleDeg;
}

void halResetFlowTotalizer() {
  noInterrupts();
  s_hallPulseCounter = 0;
  interrupts();
  s_lastPulseSnapshot = 0;
  s_telemetry.ft1Pulses = 0;
  s_telemetry.ft1FlowLpm = 0.0f;
  s_telemetry.ft1TotalLiters = 0.0f;
}

void halSetElectrodeMeasurement(bool enabled) {
  s_electrodeMeasurementEnabled = enabled;
}

bool halGetElectrodeMeasurement() {
  return s_electrodeMeasurementEnabled;
}

void halTareElectrodeZero() {
  s_ct1ZeroVolts = s_telemetry.ct1Volts;
}

float halGetElectrodeZeroVolts() {
  return s_ct1ZeroVolts;
}

void halTarePt1Zero() {
  samplePt1HighRes();
  if (s_telemetry.pt1Mv >= 200.0f && s_telemetry.pt1Mv <= 1200.0f) {
    s_pt1ZeroMv = s_telemetry.pt1Mv;
    s_telemetry.pt1ZeroMv = s_pt1ZeroMv;
    savePt1CalToNvs();
    samplePt1HighRes();
    Serial.printf("[PT1 CAL] Cero calibrado (0 cm) -> %.2f mV\n", s_pt1ZeroMv);
  }
}

bool halCalibratePt1AtHeightCm(float waterHeightCm) {
  samplePt1HighRes();
  if (waterHeightCm <= 0.5f || waterHeightCm > 150.0f) {
    halTarePt1Zero();
    return true;
  }
  float deltaMv = s_telemetry.pt1Mv - s_pt1ZeroMv;
  if (deltaMv < 0.8f) {
    // Si el voltaje actual es casi igual o menor al cero pero sabemos que hay agua a waterHeightCm,
    // recalculamos el cero asumiendo la pendiente actual para alinear exactamente la altura medida
    s_pt1ZeroMv = s_telemetry.pt1Mv - (waterHeightCm * s_pt1MvPerCm);
  } else {
    float newSlope = deltaMv / waterHeightCm;
    if (newSlope >= 0.10f && newSlope <= 15.0f) {
      s_pt1MvPerCm = newSlope;
    }
  }
  s_telemetry.pt1ZeroMv  = s_pt1ZeroMv;
  s_telemetry.pt1MvPerCm = s_pt1MvPerCm;
  savePt1CalToNvs();
  samplePt1HighRes();
  Serial.printf("[PT1 CAL] Altura calibrada a %.1f cm -> V=%.2f mV | Zero=%.2f mV | Pendiente=%.4f mV/cm\n",
                waterHeightCm, s_telemetry.pt1Mv, s_pt1ZeroMv, s_pt1MvPerCm);
  return true;
}

void halSetPt1Calibration(float zeroMv, float mvPerCm) {
  if (!isnan(zeroMv) && zeroMv >= 200.0f && zeroMv <= 1200.0f) {
    s_pt1ZeroMv = zeroMv;
  }
  if (!isnan(mvPerCm) && mvPerCm >= 0.10f && mvPerCm <= 15.0f) {
    s_pt1MvPerCm = mvPerCm;
  }
  s_telemetry.pt1ZeroMv  = s_pt1ZeroMv;
  s_telemetry.pt1MvPerCm = s_pt1MvPerCm;
  savePt1CalToNvs();
  samplePt1HighRes();
}

void halResetPt1Calibration() {
  s_pt1ZeroMv  = PT1_DEFAULT_ZERO_MV;
  s_pt1MvPerCm = PT1_DEFAULT_MV_PER_CM;
  s_telemetry.pt1ZeroMv  = s_pt1ZeroMv;
  s_telemetry.pt1MvPerCm = s_pt1MvPerCm;
  savePt1CalToNvs();
  samplePt1HighRes();
}

const ActuatorChannel* halGetActuatorTable() {
  return s_actuators;
}

const MultimeterTelemetry& halGetTelemetry() {
  return s_telemetry;
}
