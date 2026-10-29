#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>

// ================= 1. CONFIGURACIÓN DE RED =================
const char* ssid = "TU_WIFI"; // Cambiar por el nombre de tu Wi-Fi
const char* password = "TU_PASSWORD"; // Cambiar por tu contraseña

ESP8266WebServer server(80);
Servo servoRechazo;

// ================= 2. MAPA DE PINES (NodeMCU ESP8266) =================
#define PIN_RELE_SOLENOIDE D1 // GPIO5 - Relé 1: Válvula de llenado
#define PIN_RELE_ELECTRO D2 // GPIO4 - Relé 2: Fuente Electrocoagulación
#define PIN_RELE_AIREADOR D5 // GPIO14 - Relé 3: Bomba Aireadora
#define PIN_RELE_BOMBA_RO D6 // GPIO12 - Relé 4: Bomba Ósmosis / Transferencia

#define PIN_FLUJO_ENTRADA D7 // GPIO13 - Sensor Hall de flujo de agua
#define PIN_SERVO_RECHAZO D8 // GPIO15 - Servo MG996R para válvula de rechazo
#define PIN_POWER_TDS D0 // GPIO16 - Alimenta el módulo TDS solo al medir
#define PIN_ANALOGICO A0 // ADC0 - Entrada analógica

#define RELE_ON LOW
#define RELE_OFF HIGH

// ================= 3. MÁQUINA DE ESTADOS Y VARIABLES =================
enum EstadoPlanta {
  IDLE = 0,
  LLENANDO = 1,
  ELECTROCOAGULANDO = 2,
  AIREANDO = 3,
  LISTO_PARA_FILTRAR = 4,
  FILTRANDO_OSMOSIS = 5,
  EMERGENCIA = 6
};

EstadoPlanta estadoActual = IDLE;

float metaLitrosLote = 20.0;
float litrosActuales = 0.0;
float caudalLPM = 0.0;
float factorCalibracionFlujo = 7.5;

float amperiosFuente = 0.0;
float limiteMaxAmperios = 12.0;
int tdsActualPPM = 650;
int metaTdsPPM = 250;
float presionPSI = 0.0;
int anguloServo = 90;
String mensajeAlerta = "Sistema en espera. Listo para iniciar lote.";

unsigned long ultimoCalculoFlujo = 0;
unsigned long inicioEstadoTiempo = 0;
unsigned long ultimoMuestreoTDS = 0;
unsigned long tiempoAireacionMin = 120;
bool midiendoTdsEnPausa = false;

volatile unsigned long pulsosFlujo = 0;
void IRAM_ATTR contarPulsoFlujo() {
  pulsosFlujo++;
}

// ================= 4. FUNCIONES AUXILIARES DE HARDWARE =================
void apagarTodosLosReles() {
  digitalWrite(PIN_RELE_SOLENOIDE, RELE_OFF);
  digitalWrite(PIN_RELE_ELECTRO, RELE_OFF);
  digitalWrite(PIN_RELE_AIREADOR, RELE_OFF);
  digitalWrite(PIN_RELE_BOMBA_RO, RELE_OFF);
}

String obtenerNombreEstado() {
  switch (estadoActual) {
    case IDLE: return "EN ESPERA";
    case LLENANDO: return "LLENANDO LOTE";
    case ELECTROCOAGULANDO: return "ELECTROCOAGULACION";
    case AIREANDO: return "AIREACION Y REPOSO";
    case LISTO_PARA_FILTRAR: return "QUITAR NATAS / LISTO";
    case FILTRANDO_OSMOSIS: return "FILTRACION Y OSMOSIS";
    case EMERGENCIA: return "PARO DE EMERGENCIA";
    default: return "DESCONOCIDO";
  }
}

void actualizarSensoresAnalogicos() {
  int raw = analogRead(PIN_ANALOGICO);
  float voltaje = raw * (3.3 / 1023.0);

  if (estadoActual == ELECTROCOAGULANDO && !midiendoTdsEnPausa) {
    amperiosFuente = abs((voltaje - 1.65) / 0.066);
    if (amperiosFuente > limiteMaxAmperios) {
      apagarTodosLosReles();
      estadoActual = EMERGENCIA;
      mensajeAlerta = "ALERTA: Sobrecarga en la fuente (" + String(amperiosFuente, 1) + " A). Fuente apagada por proteccion.";
    }
  }
  else if (estadoActual == FILTRANDO_OSMOSIS) {
    presionPSI = (voltaje - 0.33) * (100.0 / (2.97 - 0.33));
    if (presionPSI < 0) presionPSI = 0;

    if (presionPSI > 85.0) {
      apagarTodosLosReles();
      estadoActual = EMERGENCIA;
      mensajeAlerta = "ALERTA: Sobrepresion en membrana (" + String(presionPSI, 0) + " PSI). Bomba detenida.";
    }
  }
}

void medirTdsSinRuido() {
  unsigned long ahora = millis();

  if (estadoActual == ELECTROCOAGULANDO && !midiendoTdsEnPausa && (ahora - ultimoMuestreoTDS > 30000)) {
    digitalWrite(PIN_RELE_ELECTRO, RELE_OFF);
    digitalWrite(PIN_POWER_TDS, HIGH);
    midiendoTdsEnPausa = true;
    ultimoMuestreoTDS = ahora;
  }

  if (midiendoTdsEnPausa && (ahora - ultimoMuestreoTDS > 1500)) {
    int raw = analogRead(PIN_ANALOGICO);
    float v = raw * (3.3 / 1023.0);
    tdsActualPPM = (int)((133.42 * v * v * v - 255.86 * v * v + 857.39 * v) * 0.5);

    digitalWrite(PIN_POWER_TDS, LOW);
    midiendoTdsEnPausa = false;

    if (tdsActualPPM <= metaTdsPPM) {
      apagarTodosLosReles();
      digitalWrite(PIN_RELE_AIREADOR, RELE_ON);
      estadoActual = AIREANDO;
      inicioEstadoTiempo = millis();
      mensajeAlerta = "Meta de TDS alcanzada (" + String(tdsActualPPM) + " ppm). Iniciando aireacion y sedimentacion.";
    } else {
      digitalWrite(PIN_RELE_ELECTRO, RELE_ON);
    }
  }
}

void actualizarMedidorFlujo() {
  if (millis() - ultimoCalculoFlujo >= 1000) {
    detachInterrupt(digitalPinToInterrupt(PIN_FLUJO_ENTRADA));
    caudalLPM = (pulsosFlujo / factorCalibracionFlujo);
    float litrosSegundo = caudalLPM / 60.0;
    if (estadoActual == LLENANDO || estadoActual == FILTRANDO_OSMOSIS) {
      litrosActuales += litrosSegundo;
    }
    pulsosFlujo = 0;
    ultimoCalculoFlujo = millis();
    attachInterrupt(digitalPinToInterrupt(PIN_FLUJO_ENTRADA), contarPulsoFlujo, FALLING);
  }
}

// ================= 5. INTERFAZ WEB CON VOZ (HTML + JS) =================
const char PAGINA_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Planta de Agua - Control por Voz</title>
<style>
  body { font-family: system-ui, sans-serif; background: #0f172a; color: #f8fafc; margin: 0; padding: 16px; text-align: center; }
  .card { background: #1e293b; border-radius: 14px; padding: 16px; margin-bottom: 14px; box-shadow: 0 4px 12px rgba(0,0,0,0.4); }
  h1 { font-size: 1.3rem; margin: 0 0 8px 0; color: #38bdf8; }
  .estado-badge { display: inline-block; padding: 8px 16px; border-radius: 20px; font-weight: bold; background: #0284c7; font-size: 1rem; margin: 6px 0; }
  .grid { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; margin-top: 10px; }
  .metric { background: #0f172a; padding: 12px; border-radius: 10px; }
  .metric span { display: block; font-size: 0.8rem; color: #94a3b8; }
  .metric strong { font-size: 1.4rem; color: #e2e8f0; }
  button { width: 100%; padding: 14px; margin: 6px 0; border: none; border-radius: 10px; font-size: 1rem; font-weight: bold; cursor: pointer; color: white; }
  .btn-voz { background: #8b5cf6; font-size: 1.1rem; }
  .btn-inicio { background: #10b981; }
  .btn-ro { background: #3b82f6; }
  .btn-stop { background: #ef4444; }
  input[type=range] { width: 100%; margin: 10px 0; }
  .alerta { color: #fde047; font-size: 0.95rem; margin-top: 8px; min-height: 1.4em; }
</style>
</head>
<body>

<div class="card">
  <h1>Planta de Tratamiento - Nodo 2</h1>
  <div id="estado" class="estado-badge">CARGANDO...</div>
  <div id="alerta" class="alerta">Conectando con el controlador...</div>
  <button class="btn-voz" onclick="escucharComandoVoz()">🎤 Hablar con la Planta</button>
</div>

<div class="card">
  <div class="grid">
    <div class="metric"><span>Volumen Lote</span><strong id="litros">0.0 / 20 L</strong></div>
    <div class="metric"><span>Caudal</span><strong id="caudal">0.0 L/min</strong></div>
    <div class="metric"><span>Calidad (TDS)</span><strong id="tds">0 ppm</strong></div>
    <div class="metric"><span>Corriente Fuente</span><strong id="amperios">0.0 A</strong></div>
    <div class="metric"><span>Presión RO</span><strong id="presion">0 PSI</strong></div>
    <div class="metric"><span>Válvula Rechazo</span><strong id="servo">90°</strong></div>
  </div>
</div>

<div class="card">
  <button class="btn-inicio" onclick="enviarAccion('iniciar_lote')">▶ Iniciar Lote (20 L)</button>
  <button class="btn-ro" onclick="enviarAccion('iniciar_osmosis')">💧 Quitar Natas / Iniciar Ósmosis</button>
  <button class="btn-stop" onclick="enviarAccion('paro')">🛑 PARO DE EMERGENCIA</button>
</div>

<div class="card">
  <label>Ajuste Válvula de Rechazo (Servo): <span id="valServo">90</span>°</label>
  <input type="range" min="15" max="165" value="90" onchange="ajustarServo(this.value)">
</div>

<script>
let ultimoMensajeHablado = "";

function hablar(texto) {
  if ('speechSynthesis' in window) {
    window.speechSynthesis.cancel();
    const u = new SpeechSynthesisUtterance(texto);
    u.lang = 'es-MX';
    window.speechSynthesis.speak(u);
  }
}

function enviarAccion(accion, valor = 0) {
  fetch(`/cmd?accion=${accion}&val=${valor}`)
    .then(r => r.json())
    .then(d => {
      actualizarPantalla(d);
      if (d.voz) hablar(d.voz);
    });
}

function ajustarServo(angulo) {
  document.getElementById('valServo').innerText = angulo;
  enviarAccion('servo', angulo);
}

function actualizarPantalla(d) {
  document.getElementById('estado').innerText = d.estado;
  document.getElementById('litros').innerText = `${d.litros.toFixed(1)} / ${d.metaLitros.toFixed(0)} L`;
  document.getElementById('caudal').innerText = `${d.caudal.toFixed(1)} L/min`;
  document.getElementById('tds').innerText = `${d.tds} ppm`;
  document.getElementById('amperios').innerText = `${d.amperios.toFixed(1)} A`;
  document.getElementById('presion').innerText = `${d.presion.toFixed(0)} PSI`;
  document.getElementById('servo').innerText = `${d.servo}°`;
  document.getElementById('alerta').innerText = d.alerta;

  if (d.alerta !== ultimoMensajeHablado && d.alerta.length > 0) {
    ultimoMensajeHablado = d.alerta;
    hablar(d.alerta);
  }
}

setInterval(() => {
  fetch('/status').then(r => r.json()).then(d => actualizarPantalla(d));
}, 2000);

function escucharComandoVoz() {
  const SpeechRecognition = window.SpeechRecognition || window.webkitSpeechRecognition;
  if (!SpeechRecognition) {
    alert("Tu navegador no soporta reconocimiento de voz. Usa Chrome en Android.");
    return;
  }
  const rec = new SpeechRecognition();
  rec.lang = 'es-MX';
  rec.start();

  rec.onresult = function(event) {
    const frase = event.results[0][0].transcript.toLowerCase();
    document.getElementById('alerta').innerText = `Escuché: "${frase}"`;

    if (frase.includes("estado") || frase.includes("cómo va") || frase.includes("reporte")) {
      fetch('/status').then(r => r.json()).then(d => {
        hablar(`El sistema está en ${d.estado}. Llevamos ${d.litros.toFixed(1)} litros. El T D S actual es de ${d.tds} partes por millón, y la presión es de ${d.presion.toFixed(0)} P S I.`);
      });
    } else if (frase.includes("iniciar lote") || frase.includes("llenar")) {
      enviarAccion('iniciar_lote');
    } else if (frase.includes("ósmosis") || frase.includes("osmosis") || frase.includes("filtrar")) {
      enviarAccion('iniciar_osmosis');
    } else if (frase.includes("paro") || frase.includes("detener") || frase.includes("apagar") || frase.includes("emergencia")) {
      enviarAccion('paro');
    } else {
      hablar("No reconocí el comando. Puedes decir: estado, iniciar lote, iniciar ósmosis, o paro.");
    }
  };
}
</script>
</body>
</html>
)rawliteral";

// ================= 6. RUTAS DEL SERVIDOR WEB =================
void responderJSON(String textoVoz = "") {
  String json = "{";
  json += "\"estado\":\"" + obtenerNombreEstado() + "\",";
  json += "\"litros\":" + String(litrosActuales, 2) + ",";
  json += "\"metaLitros\":" + String(metaLitrosLote, 1) + ",";
  json += "\"caudal\":" + String(caudalLPM, 2) + ",";
  json += "\"tds\":" + String(tdsActualPPM) + ",";
  json += "\"amperios\":" + String(amperiosFuente, 2) + ",";
  json += "\"presion\":" + String(presionPSI, 1) + ",";
  json += "\"servo\":" + String(anguloServo) + ",";
  json += "\"alerta\":\"" + mensajeAlerta + "\",";
  json += "\"voz\":\"" + textoVoz + "\"";
  json += "}";
  server.send(200, "application/json", json);
}

void manejarComandos() {
  String accion = server.arg("accion");
  String textoVoz = "";

  if (accion == "iniciar_lote") {
    apagarTodosLosReles();
    litrosActuales = 0.0;
    estadoActual = LLENANDO;
    digitalWrite(PIN_RELE_SOLENOIDE, RELE_ON);
    mensajeAlerta = "Abriendo valvula solenoide. Llenando lote de " + String(metaLitrosLote, 0) + " litros.";
    textoVoz = mensajeAlerta;
  }
  else if (accion == "iniciar_osmosis") {
    apagarTodosLosReles();
    litrosActuales = 0.0;
    estadoActual = FILTRANDO_OSMOSIS;
    digitalWrite(PIN_RELE_BOMBA_RO, RELE_ON);
    mensajeAlerta = "Bomba de osmosis encendida. Vigilando presion y caudal.";
    textoVoz = mensajeAlerta;
  }
  else if (accion == "paro") {
    apagarTodosLosReles();
    estadoActual = IDLE;
    mensajeAlerta = "Paro ejecutado. Todos los reles estan apagados.";
    textoVoz = mensajeAlerta;
  }
  else if (accion == "servo") {
    anguloServo = constrain(server.arg("val").toInt(), 15, 165);
    servoRechazo.write(anguloServo);
    textoVoz = "Valvula de rechazo ajustada a " + String(anguloServo) + " grados.";
  }

  responderJSON(textoVoz);
}

// ================= 7. SETUP Y LOOP PRINCIPAL =================
void setup() {
  Serial.begin(115200);

  pinMode(PIN_RELE_SOLENOIDE, OUTPUT);
  pinMode(PIN_RELE_ELECTRO, OUTPUT);
  pinMode(PIN_RELE_AIREADOR, OUTPUT);
  pinMode(PIN_RELE_BOMBA_RO, OUTPUT);
  apagarTodosLosReles();

  pinMode(PIN_POWER_TDS, OUTPUT);
  digitalWrite(PIN_POWER_TDS, LOW);

  pinMode(PIN_FLUJO_ENTRADA, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_FLUJO_ENTRADA), contarPulsoFlujo, FALLING);

  servoRechazo.attach(PIN_SERVO_RECHAZO);
  servoRechazo.write(anguloServo);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Conectando a Wi-Fi");
  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 20) {
    delay(500);
    Serial.print(".");
    intentos++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConectado! IP del Nodo 2: " + WiFi.localIP().toString());
  } else {
    WiFi.softAP("Planta_Agua_Nodo2", "12345678");
    Serial.println("\nModo AP activado. Conecta a 'Planta_Agua_Nodo2', IP: " + WiFi.softAPIP().toString());
  }

  server.on("/", []() { server.send_P(200, "text/html", PAGINA_HTML); });
  server.on("/status", []() { responderJSON(); });
  server.on("/cmd", manejarComandos);
  server.begin();
}

void loop() {
  server.handleClient();
  actualizarMedidorFlujo();
  actualizarSensoresAnalogicos();

  switch (estadoActual) {
    case LLENANDO:
      if (litrosActuales >= metaLitrosLote) {
        apagarTodosLosReles();
        digitalWrite(PIN_RELE_ELECTRO, RELE_ON);
        estadoActual = ELECTROCOAGULANDO;
        ultimoMuestreoTDS = millis();
        mensajeAlerta = "Lote de " + String(metaLitrosLote, 0) + " litros lleno. Iniciando electrocoagulacion.";
      }
      break;

    case ELECTROCOAGULANDO:
      medirTdsSinRuido();
      break;

    case AIREANDO:
      if (millis() - inicioEstadoTiempo >= (tiempoAireacionMin * 60000UL)) {
        apagarTodosLosReles();
        estadoActual = LISTO_PARA_FILTRAR;
        mensajeAlerta = "Aireacion y reposo terminados. Por favor retire las natas y presione Iniciar Osmosis.";
      }
      break;

    default:
      break;
  }
}

