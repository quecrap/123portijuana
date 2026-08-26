/**
 * ============================================================================
 * PROYECTO: AquaResiliencia Tijuana — Nodo Centinela IoT v1.0
 * HARDWARE: ESP32 Dev Module (WROOM-32)
 * PROPÓSITO: Telemetría en tiempo real y corte preventivo de extracción somera
 * ============================================================================
 * Sensores & Actuadores:
 * - TDS / Conductividad Eléctrica: Sensor analógico Gravity (ADC1 - GPIO 34)
 * - Nivel Freático Dinámico: Sensor ultrasónico impermeable JSN-SR04T (Trig 5 / Echo 19)
 * - Caudal Instantáneo & Volumen: Caudalímetro de efecto Hall YF-S201 (GPIO 18)
 * - Actuador de Corte: Relevador de 1 canal 5V/12V Optoacoplado (GPIO 23)
 * - LED Indicador de Estado: LED Verde/Rojo (GPIO 2)
 * ============================================================================
 */

#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// --------------------------- CONFIGURACIÓN DE RED ---------------------------
const char* ssid = "AquaResiliencia_AP";
const char* password = "agua_centinela";

// Servidor Web Local para diagnóstico directo en campo
WebServer server(80);

// --------------------------- ASIGNACIÓN DE PINES ---------------------------
#define PIN_TDS           34   // ADC1 Canal 6 (0 - 3.3V)
#define PIN_FLOW_SENSOR   18   // Interrupción externa para pulsos Hall
#define PIN_TRIG          5    // Disparo ultrasónico JSN-SR04T
#define PIN_ECHO          19   // Recepción eco JSN-SR04T
#define PIN_RELAY_PUMP    23   // Relé de control de motobomba (Activo en LOW)
#define PIN_LED_STATUS    2    // LED onboard de estado

// --------------------------- UMBRALES DE SEGURIDAD --------------------------
const float MAX_DAILY_VOLUME_LITERS = 500.0;   // Límite diario de subsistencia
const float MIN_SAFE_WATER_LEVEL_M   = 2.0;    // Nivel mínimo sobre la bomba (evita pozo seco)
const int   MAX_SAFE_TDS_PPM         = 1800;   // Umbral de corte por intrusión o contaminante
const float FLOW_CALIBRATION_FACTOR  = 7.5;    // Factor pulsos/min para sensor YF-S201

// --------------------------- VARIABLES DE ESTADO ----------------------------
volatile unsigned long pulseCount = 0;
float flowRateLPM = 0.0;
float totalLitersExtracted = 0.0;
float currentWaterLevelM = 6.5;
int   currentTDSPPM = 420;
bool  pumpEnabled = true;
String systemState = "NORMAL_EXTRACTION";

unsigned long lastTelemetryUpdate = 0;
unsigned long lastPulseTime = 0;

// --------------------------- RUTINA DE INTERRUPCIÓN (CAUDAL) ----------------
void IRAM_ATTR pulseCounterISR() {
  pulseCount++;
}

// --------------------------- FUNCIONES DE MEDICIÓN --------------------------

// 1. Lectura y calibración del sensor analógico de TDS
int readTDS() {
  int rawADC = analogRead(PIN_TDS);
  float voltage = (rawADC / 4095.0) * 3.3;
  // Ecuación de compensación polinomial estándar para sensor TDS Gravity
  float compensationCoefficient = 1.0; // Asumiendo agua a 20-25°C
  float compensationVoltage = voltage / compensationCoefficient;
  float tdsValue = (133.42 * pow(compensationVoltage, 3) - 255.86 * pow(compensationVoltage, 2) + 857.39 * compensationVoltage) * 0.5;
  if (tdsValue < 0) tdsValue = 0;
  return (int)tdsValue;
}

// 2. Lectura del nivel freático con sensor ultrasónico JSN-SR04T
float readWaterLevel() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  long duration = pulseIn(PIN_ECHO, HIGH, 30000); // Timeout 30ms (~5 metros)
  if (duration == 0) {
    // Si no hay eco directo, mantener valor de respaldo
    return currentWaterLevelM;
  }
  // Distancia en metros: (duración * 0.0343) / 2 / 100
  float distanceM = (duration * 0.000343) / 2.0;
  return distanceM;
}

// 3. Cálculo de Caudal y Acumulado
void updateFlowMetrics() {
  unsigned long now = millis();
  if (now - lastPulseTime >= 1000) {
    detachInterrupt(digitalPinToInterrupt(PIN_FLOW_SENSOR));
    
    // Caudal en Litros por Minuto
    flowRateLPM = (pulseCount / FLOW_CALIBRATION_FACTOR);
    
    // Volumen en Litros en este intervalo (1 segundo)
    float litersInSecond = (flowRateLPM / 60.0);
    totalLitersExtracted += litersInSecond;
    
    pulseCount = 0;
    lastPulseTime = now;
    attachInterrupt(digitalPinToInterrupt(PIN_FLOW_SENSOR), pulseCounterISR, RISING);
  }
}

// --------------------------- LÓGICA DE CONTROL Y CORTE ----------------------
void evaluateSafetyRules() {
  // Regla 1: Corte por volumen diario superado
  if (totalLitersExtracted >= MAX_DAILY_VOLUME_LITERS) {
    pumpEnabled = false;
    systemState = "CUTOFF_DAILY_LIMIT_REACHED";
  }
  // Regla 2: Corte por salinidad crítica o contaminante anómalo
  else if (currentTDSPPM >= MAX_SAFE_TDS_PPM) {
    pumpEnabled = false;
    systemState = "CUTOFF_HIGH_TDS_CONTAMINATION";
  }
  // Regla 3: Nivel freático abatido (protección de bomba y acuífero)
  else if (currentWaterLevelM < MIN_SAFE_WATER_LEVEL_M) {
    pumpEnabled = false;
    systemState = "CUTOFF_AQUIFER_DRAWDOWN";
  }
  else {
    pumpEnabled = true;
    systemState = "NORMAL_EXTRACTION";
  }

  // Actuación del relevador (Invertido: LOW = Bomba encendida, HIGH = Apagada)
  if (pumpEnabled) {
    digitalWrite(PIN_RELAY_PUMP, LOW);
    digitalWrite(PIN_LED_STATUS, HIGH);
  } else {
    digitalWrite(PIN_RELAY_PUMP, HIGH);
    digitalWrite(PIN_LED_STATUS, LOW);
  }
}

// --------------------------- ENDPOINTS API JSON -----------------------------
void handleTelemetryAPI() {
  StaticJsonDocument<300> doc;
  doc["nodo_id"] = "CENTINELA-TIJ-001";
  doc["acuifero"] = "0201-Tijuana";
  doc["caudal_lpm"] = flowRateLPM;
  doc["volumen_total_l"] = totalLitersExtracted;
  doc["nivel_dinamico_m"] = currentWaterLevelM;
  doc["tds_ppm"] = currentTDSPPM;
  doc["bomba_activa"] = pumpEnabled;
  doc["estado"] = systemState;
  doc["timestamp_ms"] = millis();

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleRoot() {
  String html = "<html><body style='font-family:sans-serif; background:#070d18; color:#fff; padding:2rem;'>";
  html += "<h2>AquaResiliencia — Nodo Centinela ESP32</h2>";
  html += "<p>Estado: <b>" + systemState + "</b></p>";
  html += "<p>Caudal: <b>" + String(flowRateLPM, 2) + " L/min</b></p>";
  html += "<p>Volumen Total: <b>" + String(totalLitersExtracted, 1) + " L</b></p>";
  html += "<p>Nivel Dinamico: <b>" + String(currentWaterLevelM, 2) + " m</b></p>";
  html += "<p>TDS Conductividad: <b>" + String(currentTDSPPM) + " ppm</b></p>";
  html += "<p><a style='color:#00e5ff' href='/api/telemetry'>Ver JSON API</a></p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

// --------------------------- SETUP & LOOP -----------------------------------
void setup() {
  Serial.begin(115200);

  // Configuración de Pines
  pinMode(PIN_TDS, INPUT);
  pinMode(PIN_FLOW_SENSOR, INPUT_PULLUP);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_RELAY_PUMP, OUTPUT);
  pinMode(PIN_LED_STATUS, OUTPUT);

  // Iniciar relé apagado por seguridad
  digitalWrite(PIN_RELAY_PUMP, HIGH);

  // Configurar interrupción para caudalímetro
  attachInterrupt(digitalPinToInterrupt(PIN_FLOW_SENSOR), pulseCounterISR, RISING);

  // Configurar Punto de Acceso WiFi para campo
  WiFi.softAP(ssid, password);
  Serial.println("\n[AQUA-CENTINELA] Nodo Iniciado.");
  Serial.print("Punto de Acceso WiFi: ");
  Serial.println(ssid);
  Serial.print("IP del Nodo: ");
  Serial.println(WiFi.softAPIP());

  // Rutas Web
  server.on("/", handleRoot);
  server.on("/api/telemetry", handleTelemetryAPI);
  server.begin();
}

void loop() {
  server.handleClient();
  updateFlowMetrics();

  // Actualizar lecturas y evaluar reglas de seguridad cada 1 segundo
  if (millis() - lastTelemetryUpdate >= 1000) {
    currentTDSPPM = readTDS();
    currentWaterLevelM = readWaterLevel();
    evaluateSafetyRules();

    // Log por consola serial
    Serial.printf("[CENTINELA] Caudal: %.2f L/min | Vol: %.1f L | Nivel: %.2f m | TDS: %d ppm | Estado: %s\n",
                  flowRateLPM, totalLitersExtracted, currentWaterLevelM, currentTDSPPM, systemState.c_str());

    lastTelemetryUpdate = millis();
  }
}
