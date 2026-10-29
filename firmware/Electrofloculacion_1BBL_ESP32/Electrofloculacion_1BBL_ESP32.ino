/**
 * ============================================================================
 * PROYECTO: AquaResiliencia - "Agua de GIA" (Tijuana, B.C.)
 * MÓDULO:   Controlador Casi Autónomo de Electrofloculación, Reposo Forzado,
 *           Válvulas Solenoides y Válvula de Aguja Motorizada (1 BBL / 117.3 L)
 * PLACA:    ESP32 (WROOM-32 / DevKit V1)
 * FECHA:    29 de octubre de 2026
 * ============================================================================
 * ARQUITECTURA 100% FÍSICA (SIN BIOFINE NI ADITIVOS QUÍMICOS):
 * 1. Llenado por Lote (1 BBL): Válvula Solenoide de Entrada (SV_LLENADO) + lectura V_vacío.
 * 2. Electrofloculación (12V / 15A): Monitoreo continuo de caída de tensión (DeltaV)
 *    mediante divisor resistivo pasivo en GPIO34 (ADC1_CH6).
 * 3. Barrido de Polarización H2: Pulsos de bomba de recirculación (30-45 s cada 5-7 min).
 * 4. REPOSO FORZADO POR ALGORITMO: Bloqueo de seguridad obligatorio post-floculación
 *    (ninguna válvula ni bomba puede mover el agua mientras el flóculo precipita).
 * 5. Trasvase y Ósmosis Inversa con Válvula de Aguja Motorizada:
 *    Válvula Solenoide de Salida Clarificada (SV_TRASVASE) + Actuador PWM sobre
 *    válvula de aguja para sostener 85-90 PSI óptimos en membrana RO.
 * ============================================================================
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// ================= 1. CONFIGURACIÓN DE RED =================
const char* WIFI_SSID     = "TU_WIFI";
const char* WIFI_PASSWORD = "TU_PASSWORD";
const char* AP_SSID       = "GIA_Planta_Autonoma_1BBL";
const char* AP_PASS       = "12345678";

WebServer server(80);

// ================= 2. MAPA DE PINES (ESP32) =================
#define PIN_ADC_VOLTAJE     34  // ADC1_CH6: Divisor de voltaje fuente 12V/15A (0-12V -> 0-2.8V)
#define PIN_ADC_PRESION_RO  35  // ADC1_CH7: Transductor de presión 0-100 PSI (Acero Inox 316)
#define PIN_ADC_TDS         36  // ADC1_CH0 (VP): Sensor analógico de TDS (en pruebas de calibración)

#define PIN_RELAY_POWER     26  // Relé 1: Fuente DC 12V/15A de Electrofloculación
#define PIN_RELAY_PUMP      27  // Relé 2: Bomba de recirculación/oxigenación (barrido H2)
#define PIN_RELAY_SV_LLENA  14  // Relé 3: Válvula Solenoide de Llenado de Lote (SV-01)
#define PIN_RELAY_SV_TRASV  12  // Relé 4: Válvula Solenoide de Trasvase / Bomba RO (SV-02)
#define PIN_MOSFET_TDS      23  // Pulso VCC para energizar sensor TDS solo al medir sin ruido
#define PIN_SERVO_AGUJA     13  // Señal PWM para actuador de Válvula de Aguja (Rechazo RO)

#define RELE_ON             LOW
#define RELE_OFF            HIGH

// ================= 3. PARÁMETROS DE PROCESO Y CALIBRACIÓN =================
float factorDivisorVoltaje   = 4.30f;  // R1=33k, R2=10k
float umbralAmarilloV        = 10.40f;
float umbralRojoV            = 10.30f; // <10.30V sostenido = Alerta sobrecorriente
float umbralCortoCriticoV    = 9.85f;

uint32_t intervaloBombaSeg   = 360;    // 6 min (rango 5-7 min)
uint32_t duracionBombaSeg    = 38;     // 38 s (rango 30-45 s)
uint32_t metaFloculacionMin  = 75;     // 60 a 90 min
uint32_t tiempoReposoForzadoMin = 45;  // Bloqueo obligatorio para no romper ni agitar el flóculo

// Control automático de Válvula de Aguja RO
float presionMetaPSI         = 88.0f;  // Rango óptimo 85 - 90 PSI
float presionActualPSI       = 0.0f;
int   aperturaAgujaGrados    = 90;     // Posición del actuador de válvula de aguja (15° cerrado - 165° abierto)
int   tdsMedidoPPM           = 620;

// ================= 4. MÁQUINA DE ESTADOS (DEL POZO AL VASO Y AL RIEGO) =================
enum EstadoSistema {
  ETAPA0_ESPERA_VACIO = 0,      // Paso 1: Leyendo V_vacío (11.4-11.6V) en espera de iniciar lote
  ETAPA1_LLENANDO_LOTE = 1,     // Válvula Solenoide SV-01 abierta llenando el barril de 1 BBL
  ETAPA2_CALIBRANDO_BASE = 2,   // Paso 2: Inmersión activa, capturando V_base (~10.6V) en 5 s
  ETAPA3_FLOCULANDO = 3,        // Electrofloculación 12V/15A + monitoreo 1Hz DeltaV
  ETAPA3B_BARRIDO_H2 = 4,       // Pulso de 30-45 s de bomba para desalojar H2 sin romper nata
  ETAPA4_REPOSO_FORZADO = 5,    // BLOQUEO ESTRICTO: Flóculo precipitando, prohibido mover agua
  ETAPA5_TRASVASE_Y_RO = 6,     // SV-02 abierta + Prefiltro 80µm + Válvula de Aguja regulando 88 PSI
  ETAPA_ALERTA_PROTECCION = 7   // Corte por sobrecorriente (<10.30V sostenido) o sobrepresión
};

EstadoSistema estadoActual = ETAPA0_ESPERA_VACIO;

float vVacio            = 11.50f;
float vCargaBase        = 10.60f;
float deltaVBase        = 0.90f;
float vCargaActual      = 11.50f;
float deltaVActual      = 0.00f;
float corrienteEstimadaA = 0.0f;

#define VENTANA_HISTORIAL 30
float historialVoltaje[VENTANA_HISTORIAL];
uint8_t idxHistorial    = 0;
bool historialLleno     = false;
float varianzaVoltaje   = 0.0f;
float pctAgotamiento    = 0.0f;

unsigned long tUltimo1Hz          = 0;
unsigned long tInicioEtapa        = 0;
unsigned long tUltimoBarridoBomba = 0;
uint16_t      segBajo1030         = 0;
uint16_t      totalBarridos       = 0;

String zonaSeguridad  = "VERDE";
String mensajeSistema = "Sistema en espera (Paso 1). Leyendo V_vacío de la fuente 12V/15A.";

// ================= 5. FUNCIONES DE HARDWARE =================
void apagarTodosLosReles() {
  digitalWrite(PIN_RELAY_POWER, RELE_OFF);
  digitalWrite(PIN_RELAY_PUMP, RELE_OFF);
  digitalWrite(PIN_RELAY_SV_LLENA, RELE_OFF);
  digitalWrite(PIN_RELAY_SV_TRASV, RELE_OFF);
}

void moverActuadorAguja(int grados) {
  aperturaAgujaGrados = constrain(grados, 15, 165);
  // Pulso servo estándar 50Hz en ESP32 mediante LEDC canal 0
  uint32_t dutyUs = map(aperturaAgujaGrados, 0, 180, 500, 2400);
  uint32_t duty16 = (dutyUs * 65535UL) / 20000UL;
  ledcWrite(PIN_SERVO_AGUJA, duty16);
}

float leerVoltajeFuente12V() {
  uint32_t sumaMV = 0;
  for (int i = 0; i < 64; i++) {
    sumaMV += analogReadMilliVolts(PIN_ADC_VOLTAJE);
    delayMicroseconds(120);
  }
  return ((sumaMV / 64.0f) / 1000.0f) * factorDivisorVoltaje;
}

float leerPresionRO_PSI() {
  uint32_t mv = analogReadMilliVolts(PIN_ADC_PRESION_RO);
  float v = mv / 1000.0f;
  // Transductor 0-100 PSI (0.5V a 4.5V escalado a 0.33V-2.97V en ADC)
  float psi = (v - 0.33f) * (100.0f / (2.97f - 0.33f));
  return (psi < 0.0f) ? 0.0f : psi;
}

int medirTDSSinRuido() {
  digitalWrite(PIN_MOSFET_TDS, HIGH);
  delay(120);
  float v = analogReadMilliVolts(PIN_ADC_TDS) / 1000.0f;
  digitalWrite(PIN_MOSFET_TDS, LOW);
  float tds = (133.42f * v * v * v - 255.86f * v * v + 857.39f * v) * 0.5f;
  return (tds < 0.0f) ? 0 : (int)tds;
}

void registrarVoltaje(float v) {
  historialVoltaje[idxHistorial] = v;
  idxHistorial = (idxHistorial + 1) % VENTANA_HISTORIAL;
  if (idxHistorial == 0) historialLleno = true;
  uint8_t n = historialLleno ? VENTANA_HISTORIAL : idxHistorial;
  if (n < 5) { varianzaVoltaje = 0.0f; return; }
  float media = 0.0f;
  for (uint8_t i = 0; i < n; i++) media += historialVoltaje[i];
  media /= n;
  float sc = 0.0f;
  for (uint8_t i = 0; i < n; i++) { float d = historialVoltaje[i] - media; sc += d * d; }
  varianzaVoltaje = sc / n;
}

String nombreEtapa() {
  switch (estadoActual) {
    case ETAPA0_ESPERA_VACIO:    return "0. ESPERA / LECTURA V_VACÍO";
    case ETAPA1_LLENANDO_LOTE:   return "1. LLENANDO BARRIL 1 BBL (SV-01)";
    case ETAPA2_CALIBRANDO_BASE: return "2. CALIBRANDO LÍNEA BASE (ΔV)";
    case ETAPA3_FLOCULANDO:      return "3. ELECTROFLOCULANDO (12V/15A)";
    case ETAPA3B_BARRIDO_H2:     return "3B. BARRIENDO POLARIZACIÓN H2";
    case ETAPA4_REPOSO_FORZADO:  return "4. REPOSO FORZADO (BLOQUEO ACTIVO)";
    case ETAPA5_TRASVASE_Y_RO:   return "5. TRASVASE 80/20 + VÁLVULA AGUJA RO";
    case ETAPA_ALERTA_PROTECCION:return "ALERTA DE PROTECCIÓN ELÉCTRICA/RO";
    default:                     return "DESCONOCIDO";
  }
}

// ================= 6. ALGORITMO MAESTRO 1 HZ =================
void actualizarCiclo1Hz() {
  unsigned long ahora = millis();
  if (ahora - tUltimo1Hz < 1000) return;
  tUltimo1Hz = ahora;

  vCargaActual = leerVoltajeFuente12V();

  if (estadoActual == ETAPA0_ESPERA_VACIO || estadoActual == ETAPA1_LLENANDO_LOTE) {
    vVacio = vCargaActual;
    deltaVActual = 0.0f;
    corrienteEstimadaA = 0.0f;
    zonaSeguridad = "VERDE";
    return;
  }

  deltaVActual = max(0.0f, vVacio - vCargaActual);
  if (estadoActual == ETAPA2_CALIBRANDO_BASE || estadoActual == ETAPA3_FLOCULANDO || estadoActual == ETAPA3B_BARRIDO_H2) {
    corrienteEstimadaA = constrain(deltaVActual * 10.5f, 0.0f, 16.0f);
  } else {
    corrienteEstimadaA = 0.0f;
  }

  // ETAPA 2: Captura de línea base tras inmersión
  if (estadoActual == ETAPA2_CALIBRANDO_BASE) {
    if (ahora - tInicioEtapa >= 5000) {
      vCargaBase = vCargaActual;
      deltaVBase = max(0.20f, vVacio - vCargaBase);
      estadoActual = ETAPA3_FLOCULANDO;
      tInicioEtapa = ahora;
      tUltimoBarridoBomba = ahora;
      idxHistorial = 0;
      historialLleno = false;
      mensajeSistema = "Línea base capturada: V_base=" + String(vCargaBase, 2) + "V (ΔV=" + String(deltaVBase, 2) + "V). Electrofloculando.";
    }
    return;
  }

  // ETAPA 3 y 3B: Electrofloculación y Barrido de H2
  if (estadoActual == ETAPA3_FLOCULANDO || estadoActual == ETAPA3B_BARRIDO_H2) {
    registrarVoltaje(vCargaActual);

    if (vCargaActual < umbralCortoCriticoV) {
      apagarTodosLosReles();
      estadoActual = ETAPA_ALERTA_PROTECCION;
      zonaSeguridad = "ROJA";
      mensajeSistema = "CORTE CRÍTICO (<9.85V): Posible corto entre electrodos.";
      return;
    }

    if (vCargaActual < umbralRojoV) {
      segBajo1030++;
      zonaSeguridad = (estadoActual == ETAPA3B_BARRIDO_H2) ? "AMARILLA" : "ROJA";
      uint16_t maxSeg = (estadoActual == ETAPA3B_BARRIDO_H2) ? 25 : 12;
      if (segBajo1030 >= maxSeg) {
        apagarTodosLosReles();
        estadoActual = ETAPA_ALERTA_PROTECCION;
        zonaSeguridad = "ROJA";
        mensajeSistema = "PARO POR SOBRECORRIENTE: Voltaje < 10.30V sostenido por " + String(segBajo1030) + " s.";
        return;
      }
    } else if (vCargaActual < umbralAmarilloV) {
      if (segBajo1030 > 0) segBajo1030--;
      zonaSeguridad = "AMARILLA";
    } else {
      segBajo1030 = 0;
      zonaSeguridad = "VERDE";
    }

    // Ciclos de bomba de barrido H2
    if (estadoActual == ETAPA3_FLOCULANDO && (ahora - tUltimoBarridoBomba >= intervaloBombaSeg * 1000UL)) {
      digitalWrite(PIN_RELAY_PUMP, RELE_ON);
      estadoActual = ETAPA3B_BARRIDO_H2;
      tUltimoBarridoBomba = ahora;
      totalBarridos++;
      mensajeSistema = "Barrido #" + String(totalBarridos) + ": Desalojando microburbujas de H2 por " + String(duracionBombaSeg) + " s.";
    } else if (estadoActual == ETAPA3B_BARRIDO_H2 && (ahora - tUltimoBarridoBomba >= duracionBombaSeg * 1000UL)) {
      digitalWrite(PIN_RELAY_PUMP, RELE_OFF);
      estadoActual = ETAPA3_FLOCULANDO;
      tUltimoBarridoBomba = ahora;
      mensajeSistema = "Barrido #" + String(totalBarridos) + " finalizado. Nata superficial intacta.";
    }

    // Evaluación de agotamiento iónico o fin de tiempo -> Pasar a REPOSO FORZADO
    uint32_t minFloc = (ahora - tInicioEtapa) / 60000UL;
    float recV = max(0.0f, vCargaActual - vCargaBase);
    pctAgotamiento = constrain((recV / deltaVBase) * 100.0f, 0.0f, 100.0f);

    bool inflexionAlcanzada = (estadoActual == ETAPA3_FLOCULANDO && minFloc >= 40 && historialLleno && pctAgotamiento >= 65.0f && varianzaVoltaje < 0.0008f);
    if (inflexionAlcanzada || minFloc >= metaFloculacionMin) {
      apagarTodosLosReles();
      estadoActual = ETAPA4_REPOSO_FORZADO;
      tInicioEtapa = ahora;
      mensajeSistema = "REPOSO FORZADO ACTIVO (" + String(tiempoReposoForzadoMin) + " min): Prohibido mover o trasvasar el agua mientras sedimenta el flóculo.";
    }
    return;
  }

  // ETAPA 4: Reposo Forzado Obligatorio (Sin Biofine)
  if (estadoActual == ETAPA4_REPOSO_FORZADO) {
    apagarTodosLosReles(); // Garantiza bloqueo físico total
    uint32_t minReposo = (ahora - tInicioEtapa) / 60000UL;
    if (minReposo >= tiempoReposoForzadoMin) {
      tdsMedidoPPM = medirTDSSinRuido();
      digitalWrite(PIN_RELAY_SV_TRASV, RELE_ON);
      estadoActual = ETAPA5_TRASVASE_Y_RO;
      tInicioEtapa = ahora;
      mensajeSistema = "Reposo completado (TDS lote: " + String(tdsMedidoPPM) + " ppm). Iniciando trasvase y ajuste automático de Válvula de Aguja RO.";
    }
    return;
  }

  // ETAPA 5: Trasvase y Autorregulación de Válvula de Aguja en Ósmosis Inversa (85-90 PSI)
  if (estadoActual == ETAPA5_TRASVASE_Y_RO) {
    presionActualPSI = leerPresionRO_PSI();
    if (presionActualPSI > 98.0f) {
      apagarTodosLosReles();
      estadoActual = ETAPA_ALERTA_PROTECCION;
      zonaSeguridad = "ROJA";
      mensajeSistema = "ALERTA SOBREPRESIÓN RO (" + String(presionActualPSI, 0) + " PSI). Bomba detenida.";
      return;
    }
    // Lazo proporcional sobre la válvula de aguja para clavar la presión en presionMetaPSI (88 PSI)
    if (presionActualPSI < (presionMetaPSI - 2.0f)) {
      moverActuadorAguja(aperturaAgujaGrados - 2); // Cierra ligeramente aguja para subir presión
    } else if (presionActualPSI > (presionMetaPSI + 2.0f)) {
      moverActuadorAguja(aperturaAgujaGrados + 2); // Abre ligeramente aguja para aliviar presión
    }
  }
}

// ================= 7. SERVIDOR WEB =================
void enviarEstadoJSON() {
  unsigned long ahora = millis();
  uint32_t minEtapa = (ahora - tInicioEtapa) / 60000UL;
  String json = "{";
  json += "\"etapaIdx\":" + String((int)estadoActual) + ",";
  json += "\"estado\":\"" + nombreEtapa() + "\",";
  json += "\"zona\":\"" + zonaSeguridad + "\",";
  json += "\"vVacio\":" + String(vVacio, 2) + ",";
  json += "\"vBase\":" + String(vCargaBase, 2) + ",";
  json += "\"vActual\":" + String(vCargaActual, 2) + ",";
  json += "\"deltaV\":" + String(deltaVActual, 2) + ",";
  json += "\"amps\":" + String(corrienteEstimadaA, 1) + ",";
  json += "\"psi\":" + String(presionActualPSI, 1) + ",";
  json += "\"aguja\":" + String(aperturaAgujaGrados) + ",";
  json += "\"tds\":" + String(tdsMedidoPPM) + ",";
  json += "\"minEtapa\":" + String(minEtapa) + ",";
  json += "\"agotamiento\":" + String(pctAgotamiento, 1) + ",";
  json += "\"mensaje\":\"" + mensajeSistema + "\"";
  json += "}";
  server.send(200, "application/json", json);
}

void manejarCmd() {
  String a = server.arg("accion");
  if (a == "llenar") {
    apagarTodosLosReles();
    vVacio = leerVoltajeFuente12V();
    digitalWrite(PIN_RELAY_SV_LLENA, RELE_ON);
    estadoActual = ETAPA1_LLENANDO_LOTE;
    tInicioEtapa = millis();
    mensajeSistema = "Válvula Solenoide SV-01 abierta. Llenando barril de 1 BBL (117.3 L)...";
  } else if (a == "flocular") {
    apagarTodosLosReles();
    vVacio = leerVoltajeFuente12V();
    digitalWrite(PIN_RELAY_POWER, RELE_ON);
    estadoActual = ETAPA2_CALIBRANDO_BASE;
    tInicioEtapa = millis();
    segBajo1030 = 0;
    totalBarridos = 0;
    mensajeSistema = "Inmersión confirmada. Estabilizando 5 s para capturar línea base del lote...";
  } else if (a == "reposo") {
    apagarTodosLosReles();
    estadoActual = ETAPA4_REPOSO_FORZADO;
    tInicioEtapa = millis();
    mensajeSistema = "Reposo forzado iniciado manualmente (" + String(tiempoReposoForzadoMin) + " min de bloqueo).";
  } else if (a == "trasvase_ro") {
    if (estadoActual == ETAPA4_REPOSO_FORZADO && ((millis() - tInicioEtapa) / 60000UL) < tiempoReposoForzadoMin) {
      mensajeSistema = "BLOQUEO DE SEGURIDAD: Aún faltan minutos de reposo forzado. Mover el agua ahora rompería el flóculo.";
    } else {
      apagarTodosLosReles();
      tdsMedidoPPM = medirTDSSinRuido();
      digitalWrite(PIN_RELAY_SV_TRASV, RELE_ON);
      estadoActual = ETAPA5_TRASVASE_Y_RO;
      tInicioEtapa = millis();
      mensajeSistema = "Válvula Solenoide SV-02 abierta y Ósmosis activa. Válvula de aguja regulando a 88 PSI.";
    }
  } else if (a == "paro") {
    apagarTodosLosReles();
    estadoActual = ETAPA0_ESPERA_VACIO;
    zonaSeguridad = "VERDE";
    mensajeSistema = "Paro ejecutado. Todas las válvulas y relés apagados.";
  }
  enviarEstadoJSON();
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_RELAY_POWER, OUTPUT);
  pinMode(PIN_RELAY_PUMP, OUTPUT);
  pinMode(PIN_RELAY_SV_LLENA, OUTPUT);
  pinMode(PIN_RELAY_SV_TRASV, OUTPUT);
  pinMode(PIN_MOSFET_TDS, OUTPUT);
  apagarTodosLosReles();
  digitalWrite(PIN_MOSFET_TDS, LOW);

  analogReadResolution(12);
  analogSetPinAttenuation(PIN_ADC_VOLTAJE, ADC_11db);
  analogSetPinAttenuation(PIN_ADC_PRESION_RO, ADC_11db);
  analogSetPinAttenuation(PIN_ADC_TDS, ADC_11db);

  ledcAttach(PIN_SERVO_AGUJA, 50, 16);
  moverActuadorAguja(90);

  vVacio = leerVoltajeFuente12V();
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(AP_SSID, AP_PASS);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  server.on("/status", enviarEstadoJSON);
  server.on("/cmd", manejarCmd);
  server.begin();
}

void loop() {
  server.handleClient();
  actualizarCiclo1Hz();
}
