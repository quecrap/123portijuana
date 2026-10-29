/**
 * ============================================================================
 * PROYECTO: AquaResiliencia - "Agua de GIA" (Pozo 001, Tijuana, B.C.)
 * MÓDULO:   NODO 1 - Telemetría Subterránea ("Modo Cápsula del Tiempo")
 * PLACA:    LilyGO T-A7670G (ESP32 + Módem 4G LTE A7670G + Wi-Fi)
 * ============================================================================
 * HARDWARE:
 * - Nivel de Agua: Campana de aire (manguera PU 4mm junto a CPVC 1/2" en pozo 2")
 *                  conectada a sensor de presión HX710B / MPS20N0040D (0-40 kPa).
 * - Calidad (TDS): DFRobot Gravity TDS conmutado por MOSFET (99% del tiempo OFF).
 * - Energía:       Celda galvánica subterránea (Ánodo de Magnesio + Tubo de Cobre)
 *                  + Cosechador BQ25504 + Batería de litio reciclada de terminal.
 * - Almacenamiento: LittleFS interno (/capsula_tiempo.csv) previo a transmitir.
 * - Supervivencia: Backoff exponencial en RTC (30-60m WiFi -> 24h LTE ->
 *                  Días 1-7: 1/día | Días 8-30: 1/sem | Meses 2-12: 1/mes | >1 año: 1/año).
 * ============================================================================
 */

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <LittleFS.h>
#include "esp_sleep.h"

// ================= 1. CONFIGURACIÓN DE RED Y ENDPOINT =================
const char* WIFI_SSID     = "TU_WIFI_SUPERFICIE";
const char* WIFI_PASSWORD = "TU_PASSWORD";
const char* APN_CELULAR   = "internet.itelcel.com"; // APN 4G LTE
const char* SERVER_HOST   = "api.aquaresiliencia.org";
const char* SERVER_PATH   = "/v1/gia/nodo1/telemetria";

// ================= 2. MAPA DE PINES (LilyGO T-A7670G ESP32) =================
#define MODEM_TX_PIN          26
#define MODEM_RX_PIN          27
#define MODEM_PWRKEY_PIN      4
#define MODEM_DTR_PIN         32
#define MODEM_RI_PIN          33
#define MODEM_FLIGHT_PIN      25
#define MODEM_STATUS_PIN      34

// Sensores de Pozo ("Agua de GIA")
#define PIN_HX710B_DOUT       19   // Datos del sensor de presión de aire 0-40 kPa (Campana)
#define PIN_HX710B_SCK        18   // Reloj del sensor HX710B / MPS20N0040D
#define PIN_MOSFET_TDS        23   // Gate del MOSFET (HIGH = energiza sensor TDS <1% del tiempo)
#define PIN_TDS_ADC           36   // VP (GPIO36) - Entrada analógica del DFRobot Gravity TDS
#define PIN_BAT_ADC           35   // Divisor resistivo de batería de litio + cosechador BQ25504
#define PIN_BOTON_EXTRACCION  0    // Botón BOOT para activar Wi-Fi Direct / Extracción CSV

// ================= 3. CONSTANTES FÍSICAS Y DE CALIBRACIÓN =================
#define RANGO_MAX_KPA         40.0f
#define KPA_POR_METRO_AGUA    9.80665f
#define PROFUNDIDAD_CAMPANA_M 14.0f
#define HX710B_OFFSET_RAW     838860L
#define HX710B_SENS_RAW_KPA   185000.0f

// Intervalos de sueño profundo en segundos (64-bit)
#define SEGUNDOS_WIFI_NORMAL  (45ULL * 60ULL)            // 45 min (rango 30-60 min)
#define SEGUNDOS_UN_DIA       (24ULL * 3600ULL)          // 86,400 s (1 vez al día)
#define SEGUNDOS_UNA_SEMANA   (7ULL * 24ULL * 3600ULL)   // 604,800 s (Días 8 a 30)
#define SEGUNDOS_UN_MES       (30ULL * 24ULL * 3600ULL)  // 2,592,000 s (Meses 2 a 12)
#define SEGUNDOS_UN_ANIO      (365ULL * 24ULL * 3600ULL) // 31,536,000 s (> 1 año sin red)

// ================= 4. MEMORIA RTC (SOBREVIVE A DEEP SLEEP Y APAGONES) =================
RTC_DATA_ATTR uint32_t rtcCiclosTotales             = 0;
RTC_DATA_ATTR uint32_t rtcDiasSinConectividad       = 0;
RTC_DATA_ATTR uint32_t rtcSegundosAcumuladosOffline = 0;
RTC_DATA_ATTR float    rtcUltimoNivelMetros         = 0.0f;
RTC_DATA_ATTR int      rtcUltimoTdsPPM              = 0;

struct LecturaCapsula {
  uint32_t ciclo;
  float presionKPa;
  float columnaAguaMetros;
  int tdsPPM;
  uint16_t bateriaMV;
  uint32_t diasOffline;
  String viaTransmision;
};

HardwareSerial SerialAT(1);

// ================= 5. LECTURA DE SENSORES PROTEGIDOS =================
long leerHX710BRaw() {
  pinMode(PIN_HX710B_DOUT, INPUT);
  pinMode(PIN_HX710B_SCK, OUTPUT);
  digitalWrite(PIN_HX710B_SCK, LOW);

  unsigned long inicio = millis();
  while (digitalRead(PIN_HX710B_DOUT) == HIGH) {
    if (millis() - inicio > 500) return HX710B_OFFSET_RAW;
    delay(1);
  }

  long valor = 0;
  for (uint8_t i = 0; i < 24; i++) {
    digitalWrite(PIN_HX710B_SCK, HIGH);
    delayMicroseconds(5);
    valor = (valor << 1) | digitalRead(PIN_HX710B_DOUT);
    digitalWrite(PIN_HX710B_SCK, LOW);
    delayMicroseconds(5);
  }
  digitalWrite(PIN_HX710B_SCK, HIGH);
  delayMicroseconds(5);
  digitalWrite(PIN_HX710B_SCK, LOW);

  if (valor & 0x800000) {
    valor |= 0xFF000000;
  }
  return valor;
}

void medirCampanaDeAire(float &presionKPa, float &columnaMetros) {
  long suma = 0;
  const int muestras = 5;
  for (int i = 0; i < muestras; i++) {
    suma += leerHX710BRaw();
    delay(25);
  }
  long promedio = suma / muestras;
  presionKPa = (float)(promedio - HX710B_OFFSET_RAW) / HX710B_SENS_RAW_KPA;
  if (presionKPa < 0.0f) presionKPa = 0.0f;
  if (presionKPa > RANGO_MAX_KPA) presionKPa = RANGO_MAX_KPA;

  columnaMetros = presionKPa / KPA_POR_METRO_AGUA;
}

int medirTDSProtegido() {
  pinMode(PIN_MOSFET_TDS, OUTPUT);
  digitalWrite(PIN_MOSFET_TDS, HIGH);
  delay(350);

  long sumaADC = 0;
  const int muestras = 10;
  for (int i = 0; i < muestras; i++) {
    sumaADC += analogRead(PIN_TDS_ADC);
    delay(10);
  }
  digitalWrite(PIN_MOSFET_TDS, LOW);

  float voltaje = (sumaADC / (float)muestras) * (3.3f / 4095.0f);
  float tds = (133.42f * voltaje * voltaje * voltaje
             - 255.86f * voltaje * voltaje
             + 857.39f * voltaje) * 0.5f;
  if (tds < 0.0f) tds = 0.0f;
  return (int)tds;
}

uint16_t medirBateriaTierraMV() {
  uint32_t raw = analogRead(PIN_BAT_ADC);
  float mv = (raw / 4095.0f) * 3300.0f * 2.0f;
  return (uint16_t)mv;
}

// ================= 6. PERSISTENCIA FLASH INTERNA (LITTLEFS CSV) =================
void guardarEnCapsulaCSV(const LecturaCapsula &lec) {
  if (!LittleFS.begin(true)) {
    Serial.println("[CAPSULA] Error montando LittleFS.");
    return;
  }

  bool existe = LittleFS.exists("/capsula_tiempo.csv");
  File archivo = LittleFS.open("/capsula_tiempo.csv", FILE_APPEND);
  if (!archivo) {
    Serial.println("[CAPSULA] No se pudo abrir /capsula_tiempo.csv");
    return;
  }

  if (!existe) {
    archivo.println("ciclo_rtc,presion_kpa,columna_agua_m,tds_ppm,bateria_mv,dias_offline,modo_red");
  }

  String linea = String(lec.ciclo) + "," +
                 String(lec.presionKPa, 2) + "," +
                 String(lec.columnaAguaMetros, 2) + "," +
                 String(lec.tdsPPM) + "," +
                 String(lec.bateriaMV) + "," +
                 String(lec.diasOffline) + "," +
                 lec.viaTransmision;
  archivo.println(linea);
  archivo.close();
  Serial.println("[CAPSULA CSV] Guardado exitoso: " + linea);
}

// ================= 7. COMUNICACIONES (WI-FI -> 4G LTE -> BACKOFF RTC) =================
bool intentarEnvioWiFi(const LecturaCapsula &lec) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - t0 < 8000)) {
    delay(250);
  }
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    return false;
  }

  HTTPClient http;
  String url = "http://" + String(SERVER_HOST) + String(SERVER_PATH);
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  String payload = "{\"nodo\":\"GIA_NODO1_POZO001\",\"ciclo\":" + String(lec.ciclo) +
                   ",\"kpa\":" + String(lec.presionKPa, 2) +
                   ",\"nivel_m\":" + String(lec.columnaAguaMetros, 2) +
                   ",\"tds_ppm\":" + String(lec.tdsPPM) +
                   ",\"vbat_mv\":" + String(lec.bateriaMV) + "}";

  int code = http.POST(payload);
  http.end();
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  return (code >= 200 && code < 300);
}

bool enviarComandoAT(const String &cmd, const char* esperado, uint32_t timeoutMs) {
  SerialAT.println(cmd);
  unsigned long t0 = millis();
  String resp = "";
  while (millis() - t0 < timeoutMs) {
    while (SerialAT.available()) {
      resp += (char)SerialAT.read();
      if (resp.indexOf(esperado) >= 0) return true;
    }
    delay(10);
  }
  return false;
}

bool intentarEnvio4GLTE(const LecturaCapsula &lec) {
  pinMode(MODEM_PWRKEY_PIN, OUTPUT);
  pinMode(MODEM_FLIGHT_PIN, OUTPUT);
  digitalWrite(MODEM_FLIGHT_PIN, HIGH);

  digitalWrite(MODEM_PWRKEY_PIN, LOW);
  delay(100);
  digitalWrite(MODEM_PWRKEY_PIN, HIGH);
  delay(1000);
  digitalWrite(MODEM_PWRKEY_PIN, LOW);

  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX_PIN, MODEM_TX_PIN);
  delay(3000);

  if (!enviarComandoAT("AT", "OK", 3000)) return false;
  if (!enviarComandoAT("AT+CREG?", ",1", 10000) &&
      !enviarComandoAT("AT+CREG?", ",5", 5000)) {
    enviarComandoAT("AT+CPOF", "OK", 2000);
    return false;
  }

  enviarComandoAT(String("AT+CGDCONT=1,\"IP\",\"") + APN_CELULAR + "\"", "OK", 3000);
  enviarComandoAT("AT+HTTPINIT", "OK", 3000);
  enviarComandoAT(String("AT+HTTPPARA=\"URL\",\"http://") + SERVER_HOST + SERVER_PATH + "\"", "OK", 3000);
  enviarComandoAT("AT+HTTPPARA=\"CONTENT\",\"application/json\"", "OK", 2000);

  String json = "{\"nodo\":\"GIA_NODO1_4G\",\"ciclo\":" + String(lec.ciclo) +
                ",\"nivel_m\":" + String(lec.columnaAguaMetros, 2) +
                ",\"tds_ppm\":" + String(lec.tdsPPM) +
                ",\"vbat_mv\":" + String(lec.bateriaMV) + "}";

  if (enviarComandoAT("AT+HTTPDATA=" + String(json.length()) + ",5000", "DOWNLOAD", 3000)) {
    SerialAT.print(json);
    delay(500);
  }
  bool exito = enviarComandoAT("AT+HTTPACTION=1", "+HTTPACTION: 1,20", 15000);
  enviarComandoAT("AT+HTTPTERM", "OK", 2000);
  enviarComandoAT("AT+CPOF", "OK", 3000);
  return exito;
}

uint64_t calcularSegundosBackoff(uint32_t diasSinRed) {
  if (diasSinRed <= 7) {
    return SEGUNDOS_UN_DIA;
  } else if (diasSinRed <= 30) {
    return SEGUNDOS_UNA_SEMANA;
  } else if (diasSinRed <= 365) {
    return SEGUNDOS_UN_MES;
  } else {
    return SEGUNDOS_UN_ANIO;
  }
}

// ================= 8. CICLO PRINCIPAL (SETUP + DEEP SLEEP) =================
void setup() {
  Serial.begin(115200);
  delay(200);
  analogReadResolution(12);

  rtcCiclosTotales++;

  LecturaCapsula lec;
  lec.ciclo = rtcCiclosTotales;
  medirCampanaDeAire(lec.presionKPa, lec.columnaAguaMetros);
  lec.tdsPPM = medirTDSProtegido();
  lec.bateriaMV = medirBateriaTierraMV();
  lec.diasOffline = rtcDiasSinConectividad;
  lec.viaTransmision = "LOCAL_ONLY";

  rtcUltimoNivelMetros = lec.columnaAguaMetros;
  rtcUltimoTdsPPM = lec.tdsPPM;

  uint64_t segundosDormir = SEGUNDOS_WIFI_NORMAL;

  if (intentarEnvioWiFi(lec)) {
    lec.viaTransmision = "WIFI_OK";
    rtcDiasSinConectividad = 0;
    rtcSegundosAcumuladosOffline = 0;
    segundosDormir = SEGUNDOS_WIFI_NORMAL;
  } else {
    if (intentarEnvio4GLTE(lec)) {
      lec.viaTransmision = "4G_LTE_DIA";
      rtcDiasSinConectividad = 0;
      rtcSegundosAcumuladosOffline = 0;
      segundosDormir = SEGUNDOS_UN_DIA;
    } else {
      if (rtcDiasSinConectividad == 0) {
        rtcDiasSinConectividad = 1;
      }
      segundosDormir = calcularSegundosBackoff(rtcDiasSinConectividad);
      rtcSegundosAcumuladosOffline += (uint32_t)segundosDormir;
      rtcDiasSinConectividad = 1 + (rtcSegundosAcumuladosOffline / 86400UL);
      lec.diasOffline = rtcDiasSinConectividad;
      lec.viaTransmision = "OFFLINE_BACKOFF_D" + String(rtcDiasSinConectividad);
    }
  }

  guardarEnCapsulaCSV(lec);

  Serial.printf("[NODO 1 GIA] Nivel: %.2f m | TDS: %d ppm | Bat: %u mV | Red: %s | Sleep: %llu s\n",
                lec.columnaAguaMetros, lec.tdsPPM, lec.bateriaMV,
                lec.viaTransmision.c_str(), segundosDormir);
  Serial.flush();

  esp_sleep_enable_timer_wakeup(segundosDormir * 1000000ULL);
  esp_deep_sleep_start();
}

void loop() {
}

