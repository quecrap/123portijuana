/**
 * ============================================================================
 * PROYECTO: AquaResiliencia Tijuana · Planta Purificadora «Agua de Gía»
 * ARQUITECTURA: Lavadora de Agua de los Años 70 (Modo 1) + Test Bench IEC (Modo 2)
 * PLACA: NodeMCU ESP-32S v1.1 (38 Pines, USB Tipo-C)
 * WI-FI AP: SSID "AguaDeGia_Lavadora" | PASS "agua_resiliente" | IP 192.168.4.1
 * ============================================================================
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WebServer.h>
#include <ArduinoOTA.h>
#include <Update.h>
#include <LittleFS.h>

#include "config.h"
#include "hal_electric.h"
#include "washing_cycle_fsm.h"
#include "reactor_ctrl.h"
#include "retro_washer_ui.h"
#include "vitrolero_data.h"

static WebServer server(80);
static WiFiMulti wifiMulti;
static uint32_t  lastWifiCheckMs = 0;

// ============================================================================
// 1. PÁGINA LIGERA DE ACTUALIZACIÓN INALÁMBRICA WEB OTA (/update)
// ============================================================================
static const char OTA_FORM_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html lang="es"><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Agua de Gía · Actualización OTA</title>
<style>body{background:#0b111e;color:#f8fafc;font-family:sans-serif;padding:24px;text-align:center;}
.box{max-width:440px;margin:30px auto;background:#131c2e;border:2px solid #f59e0b;border-radius:12px;padding:20px;}
input,button{margin-top:14px;padding:10px 16px;border-radius:6px;font-weight:bold;}</style></head>
<body><div class="box"><h2>⚡ Actualización OTA · Agua de Gía</h2>
<p>Selecciona el archivo compilado <b>.bin</b> para actualizar el ESP32S por Wi-Fi:</p>
<form method="POST" action="/update" enctype="multipart/form-data">
<input type="file" name="update" accept=".bin" required><br>
<button type="submit" style="background:#d97706;color:#fff;border:none;cursor:pointer;">SUBIR FIRMWARE (.BIN)</button>
</form><p style="margin-top:14px;"><a href="/" style="color:#38bdf8;">← Volver a la Lavadora de Agua</a></p></div></body></html>
)rawliteral";

// ============================================================================
// 2. ENDPOINTS HTTP / API REST JSON
// ============================================================================

// GET / -> Interfaz Dual (Modo 1: Lavadora Años 70 + Modo 2: Test Bench Ingeniero)
static void handleRootPage() {
  server.sendHeader("Content-Encoding", "gzip");
  server.send_P(200, "text/html; charset=utf-8", (const char*)RETRO_WASHER_HTML_GZ, RETRO_WASHER_HTML_GZ_LEN);
}

// Construye el JSON de telemetría (usado tanto por HTTP como por USB Serial)
static String buildStatusJson() {
  const WasherCycleStatus&   w   = fsmGetStatus();
  const MultimeterTelemetry& m   = halGetTelemetry();
  const ActuatorChannel*     act = halGetActuatorTable();

  String json;
  json.reserve(4096);
  json = "{";
  json += "\"wifi\":{\"connected\":" + String(WiFi.status() == WL_CONNECTED ? "true" : "false") +
          ",\"ip\":\"" + WiFi.localIP().toString() + "\",\"apIp\":\"" + WiFi.softAPIP().toString() + "\"},";

  // Bloque 1: Estado de la Lavadora de Agua (FSM)
  json += "\"washer\":{";
  json += "\"engineerMode\":" + String(w.uiEngineerMode ? "true" : "false") + ",";
  json += "\"running\":"      + String(w.cycleRunning ? "true" : "false") + ",";
  json += "\"paused\":"       + String(w.cyclePaused ? "true" : "false") + ",";
  json += "\"dryRun\":"       + String(w.dryRunSimulation ? "true" : "false") + ",";
  json += "\"knobPos\":"      + String((int)w.knobPos) + ",";
  json += "\"knobAngle\":"    + String(w.knobAngleDeg) + ",";
  json += "\"stageTitle\":\"" + String(w.stageTitle) + "\",";
  json += "\"stageSub\":\""   + String(w.stageSubtitle) + "\",";
  json += "\"loadSize\":"     + String((int)w.loadSize) + ",";
  json += "\"targetLiters\":" + String(w.targetVolumeLiters, 1) + ",";
  json += "\"soapType\":"     + String((int)w.soapType) + ",";
  json += "\"r0Ohms\":"       + String(w.measuredInitialR0Ohms, 2) + ",";
  json += "\"estPpm\":"       + String(w.estimatedWaterPpm, 0) + ",";
  json += "\"targetFe\":"     + String(w.targetFeGrams, 2) + ",";
  json += "\"dissolvedFe\":"  + String(w.dissolvedFeGrams, 3) + ",";
  json += "\"coulombs\":"     + String(w.accumulatedCoulombs, 1) + ",";
  json += "\"faradayPct\":"   + String(w.faradayProgressPct, 1) + ",";
  json += "\"whirlpoolPrimed\":" + String(w.whirlpoolPrimed ? "true" : "false") + ",";
  json += "\"passivePause\":" + String(w.passivePauseActive ? "true" : "false") + ",";
  json += "\"stageElapsedSec\":" + String(w.stageElapsedSec) + ",";
  json += "\"soakRemainingSec\":" + String(w.soakRemainingSec) + ",";
  json += "\"waitingValve\":" + String(w.waitingManualValve ? "true" : "false") + ",";
  json += "\"roFlushing\":"   + String(w.roFlushingActive ? "true" : "false") + ",";
  json += "\"roFlushRemSec\":"+ String(w.roFlushRemainingSec) + ",";
  json += "\"zeroFlowSec\":"  + String(w.zeroFlowTimerSec) + ",";
  json += "\"emergency\":"    + String(w.emergencyStop ? "true" : "false") + ",";
  json += "\"emergencyReason\":\"" + w.emergencyReason + "\",";
  json += "\"bellCounter\":"  + String(w.bellDingCounter);
  json += "},";

  // Bloque 2: Multímetro Digital y Testigos de Continuidad
  json += "\"multimeter\":{";
  json += "\"ct1Raw\":"       + String(m.ct1Raw) + ",";
  json += "\"ct1Volts\":"     + String(m.ct1Volts, 2) + ",";
  json += "\"cellVolts\":"    + String(m.cellVolts, 2) + ",";
  json += "\"ct1AcAmps\":"    + String(m.ct1AcAmps, 2) + ",";
  json += "\"ct1Amps\":"      + String(m.ct1Amps, 2) + ",";
  json += "\"ct1Ppm\":"       + String(m.ct1PpmInstant, 0) + ",";
  json += "\"ct1FeGps\":"     + String(m.ct1FeGramPerSec, 5) + ",";
  json += "\"ct2Raw\":"       + String(m.ct2Raw) + ",";
  json += "\"ct2Volts\":"     + String(m.ct2Volts, 2) + ",";
  json += "\"ct2Amps\":"      + String(m.ct2Amps, 2) + ",";
  json += "\"ct2DeadHead\":"  + String(m.ct2DeadHead ? "true" : "false") + ",";
  json += "\"pt1Pin\":"       + String(m.pt1Pin) + ",";
  json += "\"pt1Connected\":" + String(m.pt1Connected ? "true" : "false") + ",";
  json += "\"pt1Raw\":"       + String(m.pt1Raw) + ",";
  json += "\"pt1RawFloat\":"  + String(m.pt1RawFloat, 2) + ",";
  json += "\"pt1Mv\":"        + String(m.pt1Mv, 2) + ",";
  json += "\"pt1Volts\":"     + String(m.pt1Volts, 4) + ",";
  json += "\"pt1Psi\":"       + String(m.pt1Psi, 3) + ",";
  json += "\"pt1HeightCm\":"  + String(m.pt1HeightCm, 1) + ",";
  json += "\"pt1Pct\":"       + String(m.pt1LevelPct, 1) + ",";
  json += "\"pt1Liters\":"    + String(m.pt1Liters, 1) + ",";
  json += "\"pt1ZeroMv\":"    + String(m.pt1ZeroMv, 2) + ",";
  json += "\"pt1MvPerCm\":"   + String(m.pt1MvPerCm, 4) + ",";
  json += "\"vpRaw\":"        + String(m.passiveRaw) + ",";
  json += "\"vpPinVolts\":"   + String(m.passivePinVolts, 3) + ",";
  json += "\"vpVolts\":"      + String(m.passiveVolts, 3) + ",";
  json += "\"vpLastVolts\":"  + String(m.lastSampledPassiveVolts, 3) + ",";
  json += "\"vpSamples\":"    + String(m.passiveSampleCount) + ",";
  json += "\"littleFs\":"     + String(m.littleFsReady ? "true" : "false") + ",";
  json += "\"tt1TempC\":"     + String(m.tt1TempC, 1) + ",";
  json += "\"ft1Pulses\":"    + String(m.ft1Pulses) + ",";
  json += "\"ft1Lpm\":"       + String(m.ft1FlowLpm, 2) + ",";
  json += "\"ft1TotalL\":"    + String(m.ft1TotalLiters, 2) + ",";
  json += "\"lsh01\":"        + String(m.lsh01Closed ? "true" : "false") + ",";
  json += "\"lshh02\":"       + String(m.lshh02Closed ? "true" : "false") + ",";
  json += "\"psl03\":"        + String(m.psl03Closed ? "true" : "false") + ",";
  json += "\"srv1Angle\":"    + String(m.srv1AngleDeg) + ",";
  json += "\"electrodeEnabled\":" + String(halGetElectrodeMeasurement() ? "true" : "false") + ",";
  json += "\"electrodeZeroVolts\":" + String(halGetElectrodeZeroVolts(), 3);
  json += "},";

  // Bloque 3: Las 10 Bobinas / Relés / Zumbador (SSR1, K1..K8, BZ1)
  json += "\"actuators\":[";
  for (uint8_t i = 0; i < ACT_COUNT; i++) {
    if (i > 0) json += ",";
    json += "{";
    json += "\"id\":"        + String(act[i].id) + ",";
    json += "\"pin\":"       + String(act[i].pin) + ",";
    json += "\"code\":\""    + String(act[i].iecCode) + "\",";
    json += "\"board\":\""   + String(act[i].boardLocation) + "\",";
    json += "\"desc\":\""    + String(act[i].loadDesc) + "\",";
    json += "\"activeLow\":" + String(act[i].activeLow ? "true" : "false") + ",";
    json += "\"state\":"     + String(act[i].state ? "true" : "false") + ",";
    json += "\"pulsing\":"   + String(act[i].pulseEndMs != 0 ? "true" : "false");
    json += "}";
  }
  json += "],";
  json += "\"reactor\":" + reactorStatusJson();
  json += "}";
  return json;
}

// GET /api/status -> Telemetría en vivo cada 500ms (FSM + Multímetro + Bobinas)
static void handleApiStatus() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", buildStatusJson());
}

// POST /api/mode?engineer=1|0
static void handleApiMode() {
  if (server.hasArg("engineer")) {
    bool eng = (server.arg("engineer").toInt() == 1);
    fsmSetEngineerMode(eng);
  }
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", "{\"ok\":true}");
}

// POST /api/washer?cmd=...&val=...
static void handleApiWasher() {
  String cmd = server.arg("cmd");
  int val    = server.arg("val").toInt();

  if (cmd == "load") {
    if (val >= 0 && val <= 2) fsmSetLoadSize((LoadSizeMode)val);
  } else if (cmd == "soap") {
    if (val >= 0 && val <= 3) fsmSetSoapType((SoapTypeMode)val);
  } else if (cmd == "pos") {
    if (val >= 1 && val <= 7) fsmSetKnobPosition((WasherKnobPos)val, true);
  } else if (cmd == "start") {
    fsmStartOrResume();
  } else if (cmd == "pause") {
    fsmPause();
  } else if (cmd == "advance") {
    fsmAdvanceNextStage();
  } else if (cmd == "dryrun") {
    fsmSetDryRunSimulation(val == 1);
  } else if (cmd == "soaktime") {
    fsmSetSoakDurationSeconds((uint32_t)val);
  }

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", "{\"ok\":true}");
}

// POST /api/relay?id=0..9&action=on|off|pulse|invert|off_all
static void handleApiRelay() {
  String action = server.arg("action");
  uint8_t id    = (uint8_t)server.arg("id").toInt();

  if (action == "off_all") {
    reactorStop("Apagado general", false);
    halTurnOffAllActuators();
    server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", "{\"ok\":true}");
    return;
  }

  if (id < ACT_COUNT) {
    if (reactorOwnsActuator(id)) {
      if (action == "off") {
        reactorStop("Detenido desde el banco de reles", false);
      } else {
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.send(409, "application/json", "{\"ok\":false,\"err\":\"Rele controlado por el Reactor. Detén el lote primero.\"}");
        return;
      }
    }
    if (action == "on") {
      halSetActuator(id, true);
    } else if (action == "off") {
      halSetActuator(id, false);
    } else if (action == "pulse") {
      halPulseActuator(id, 3000); // Pulso no bloqueante de 3 segundos
    } else if (action == "invert") {
      bool currentLow = halGetActuatorTable()[id].activeLow;
      halSetActuatorPolarity(id, !currentLow);
    }
  }
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", "{\"ok\":true}");
}

// POST /api/servo?angle=0..180
static void handleApiServo() {
  if (server.hasArg("angle")) {
    int angle = server.arg("angle").toInt();
    halSetServoAngle(angle);
  }
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", "{\"ok\":true}");
}

// POST /api/reset_emergency
static void handleApiResetEmergency() {
  fsmClearEmergency();
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", "{\"ok\":true}");
}

// POST /api/reset_flow
static void handleApiResetFlow() {
  halResetFlowTotalizer();
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", "{\"ok\":true}");
}

// POST /api/electrode?enable=1|0&tare=1
static void handleApiElectrode() {
  if (server.hasArg("enable")) {
    bool en = (server.arg("enable").toInt() == 1);
    halSetElectrodeMeasurement(en);
  }
  if (server.hasArg("tare")) {
    halTareElectrodeZero();
  }
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", "{\"ok\":true}");
}

// POST /api/pt1/cal?cmd=tare|height|set|reset&cm=35.0&zero_mv=500.0&mv_per_cm=0.5689
static void handleApiPt1Cal() {
  String cmd = server.hasArg("cmd") ? server.arg("cmd") : "";
  if (cmd == "tare") {
    halTarePt1Zero();
  } else if (cmd == "height") {
    float cm = server.hasArg("cm") ? server.arg("cm").toFloat() : 0.0f;
    halCalibratePt1AtHeightCm(cm);
  } else if (cmd == "set") {
    float z = server.hasArg("zero_mv")   ? server.arg("zero_mv").toFloat()   : NAN;
    float s = server.hasArg("mv_per_cm") ? server.arg("mv_per_cm").toFloat() : NAN;
    halSetPt1Calibration(z, s);
  } else if (cmd == "reset") {
    halResetPt1Calibration();
  }
  const MultimeterTelemetry& m = halGetTelemetry();
  String res = "{\"ok\":true,\"pt1Mv\":" + String(m.pt1Mv, 2) +
               ",\"pt1ZeroMv\":" + String(m.pt1ZeroMv, 2) +
               ",\"pt1MvPerCm\":" + String(m.pt1MvPerCm, 4) +
               ",\"pt1HeightCm\":" + String(m.pt1HeightCm, 1) +
               ",\"pt1Liters\":" + String(m.pt1Liters, 1) + "}";
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", res);
}

// ----------------------------------------------------------------------------
// REACTOR CONICO 117 L: /api/reactor/*
// ----------------------------------------------------------------------------
static void sendJsonBody(int code, const String& body) {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(code, "application/json", body);
}

// POST /api/reactor/cmd?cmd=start|stop|measure|reset_dose|clear_log|empty_tank
static void handleApiReactorCmd() {
  String cmd = server.arg("cmd");
  String err;
  bool ok = true;
  if      (cmd == "start")      ok = reactorStart(err);
  else if (cmd == "stop")       reactorStop("Detenido manualmente", false);
  else if (cmd == "measure")    ok = reactorMeasureNow(err);
  else if (cmd == "reset_dose") reactorResetDose();
  else if (cmd == "clear_log")  reactorClearLog();
  else if (cmd == "empty_tank") reactorSetVolume(0.0f);
  else { ok = false; err = "Comando desconocido"; }
  if (ok) sendJsonBody(200, "{\"ok\":true}");
  else    sendJsonBody(400, "{\"ok\":false,\"err\":\"" + err + "\"}");
}

// GET /api/reactor/cfg -> parametros actuales
static void handleApiReactorCfgGet() {
  sendJsonBody(200, reactorConfigJson());
}

// POST /api/reactor/cfg?clave=valor&... -> aplica y guarda en memoria (NVS)
static void handleApiReactorCfgSet() {
  uint8_t applied = 0;
  for (int i = 0; i < server.args(); i++) {
    String name = server.argName(i);
    if (name == "plain") continue;
    if (reactorSetParam(name.c_str(), server.arg(i).toFloat())) applied++;
  }
  if (applied > 0) reactorSaveConfig();
  sendJsonBody(200, reactorConfigJson());
}

// POST /api/reactor/note?amps=8.7 -> anota lectura de pinza / cable
static void handleApiReactorNote() {
  if (!server.hasArg("amps")) { sendJsonBody(400, "{\"ok\":false,\"err\":\"Falta amps\"}"); return; }
  reactorAddClampNote(server.arg("amps").toFloat());
  sendJsonBody(200, "{\"ok\":true}");
}

// GET /api/anodo_pasivo.csv -> Descarga directa de la curva en LittleFS
static void handleApiPassiveAnodeCsv() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Content-Disposition", "attachment; filename=anodo_pasivo.csv");
  if (halGetTelemetry().littleFsReady && LittleFS.exists(PASSIVE_ANODE_CSV_PATH)) {
    File f = LittleFS.open(PASSIVE_ANODE_CSV_PATH, FILE_READ);
    if (f) {
      server.streamFile(f, "text/csv; charset=utf-8");
      f.close();
      return;
    }
  }
  server.send(200, "text/csv; charset=utf-8",
              "uptime_s,etapa_s,v_anodo_pasivo_vp,v_pin36,i_ac_atx_a,i_dc_celda_a,coulombs_c,hierro_g,nivel_pct,litros_l,presion_psi,temp_c\n");
}

// POST /api/reactor/ log.csv -> descarga de bitacora (CSV)
static void handleApiReactorLog() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Content-Disposition", "attachment; filename=reactor_bitacora.csv");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/csv; charset=utf-8", "");
  server.sendContent(reactorLogCsvHeader());
  uint16_t n = reactorLogCount();
  for (uint16_t i = 0; i < n; i++) {
    server.sendContent(reactorLogCsvLine(i));
  }
  server.sendContent("");
}

// ============================================================================
// 3. INICIALIZACIÓN DE WI-FI (AP + STA) Y OTA
// ============================================================================
static void initWifiAndOta() {
  WiFi.persistent(false);
  WiFi.mode(WIFI_AP_STA);
  WiFi.setSleep(false);

  // 1. Registrar redes Wi-Fi de casa ("Lacasita" / "la ultima compuerta") y conectar primero
  //    para adoptar el canal físico del router antes de levantar el AP local.
  for (uint8_t i = 0; i < WIFI_STA_COUNT; i++) {
    wifiMulti.addAP(WIFI_STA_LIST[i].ssid, WIFI_STA_LIST[i].password);
  }

  Serial.println("[STA] Conectando a red Wi-Fi doméstica (Lacasita / la última compuerta)...");
  int apChannel = 1;
  if (wifiMulti.run(6000) == WL_CONNECTED) {
    apChannel = (WiFi.channel() > 0) ? WiFi.channel() : 1;
    Serial.printf("[STA] ¡Conectado a %s (Canal %d)! IP en casa: http://%s\n",
                  WiFi.SSID().c_str(), apChannel, WiFi.localIP().toString().c_str());
  } else {
    Serial.println("[STA] Operando en Modo AP Autónomo (192.168.4.1). Reintentará en segundo plano.");
  }

  // 2. Levantar Punto de Acceso Local Oficial: "AguaDeGia_Lavadora" -> 192.168.4.1
  IPAddress apIP(WIFI_AP_IP_A, WIFI_AP_IP_B, WIFI_AP_IP_C, WIFI_AP_IP_D);
  IPAddress apSub(255, 255, 255, 0);
  WiFi.softAPConfig(apIP, apIP, apSub);
  WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASS, apChannel, 0, 4);
  delay(100);
  WiFi.setSleep(false);

  Serial.println("======================================================");
  Serial.printf("[AP] Red Wi-Fi creada: %s (Canal %d)\n", WIFI_AP_SSID, apChannel);
  Serial.printf("[AP] Contraseña:       %s\n", WIFI_AP_PASS);
  Serial.printf("[AP] Dirección IP:     http://%s\n", WiFi.softAPIP().toString().c_str());
  Serial.println("======================================================");

  // 3. Configurar ArduinoOTA
  ArduinoOTA.setHostname("AguaDeGia-Lavadora");
  ArduinoOTA.setPassword("agua_resiliente");
  ArduinoOTA.begin();

  // 4. Registrar rutas HTTP del Servidor Web
  server.on("/", HTTP_GET, handleRootPage);
  server.on("/vitrolero.jpg", HTTP_GET, []() {
    server.sendHeader("Cache-Control", "public, max-age=604800, immutable");
    server.send_P(200, "image/jpeg", (const char*)VITROLERO_DATA, VITROLERO_DATA_LEN);
  });
  server.on("/api/status", HTTP_GET, handleApiStatus);
  server.on("/api/mode",            HTTP_POST, handleApiMode);
  server.on("/api/washer",          HTTP_POST, handleApiWasher);
  server.on("/api/relay",           HTTP_POST, handleApiRelay);
  server.on("/api/servo",           HTTP_POST, handleApiServo);
  server.on("/api/reset_emergency", HTTP_POST, handleApiResetEmergency);
  server.on("/api/reset_flow",      HTTP_POST, handleApiResetFlow);
  server.on("/api/electrode",       HTTP_POST, handleApiElectrode);
  server.on("/api/pt1/cal",         HTTP_POST, handleApiPt1Cal);
  server.on("/api/reactor/cmd",     HTTP_POST, handleApiReactorCmd);
  server.on("/api/reactor/cfg",     HTTP_GET,  handleApiReactorCfgGet);
  server.on("/api/reactor/cfg",     HTTP_POST, handleApiReactorCfgSet);
  server.on("/api/reactor/note",    HTTP_POST, handleApiReactorNote);
  server.on("/api/reactor/log.csv", HTTP_GET,  handleApiReactorLog);
  server.on("/api/anodo_pasivo.csv",HTTP_GET,  handleApiPassiveAnodeCsv);
  server.on("/anodo_pasivo.csv",    HTTP_GET,  handleApiPassiveAnodeCsv);

  // Endpoint Web OTA (/update)
  server.on("/update", HTTP_GET, []() {
    server.send_P(200, "text/html; charset=utf-8", OTA_FORM_HTML);
  });
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "ERROR OTA" : "ACTUALIZACION EXITOSA. REINICIANDO ESP32S...");
    delay(600);
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      halTurnOffAllActuators();
      Update.begin(UPDATE_SIZE_UNKNOWN);
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      Update.write(upload.buf, upload.currentSize);
    } else if (upload.status == UPLOAD_FILE_END) {
      Update.end(true);
    }
  });

  server.begin();
  Serial.println("[HTTP] Servidor Web «Lavadora de Agua Años 70 & Test Bench» ACTIVO en puerto 80.");
}


// ============================================================================
// 3.5 PUENTE USB SERIAL PARA BANCO DE PRUEBAS EN PC (115200 BAUDIOS)
// Permite usar la misma interfaz web desde la PC por cable USB aunque no haya Wi-Fi
// ============================================================================
static void handleSerialBridge() {
  while (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) continue;

    if (line == "STATUS" || line == "/api/status") {
      Serial.print("JSON_STATUS:");
      Serial.println(buildStatusJson());
    } else if (line.startsWith("RELAY ")) {
      // Formato: RELAY <id> <action>
      int sp1 = line.indexOf(' ');
      int sp2 = line.indexOf(' ', sp1 + 1);
      if (sp2 > sp1) {
        String idStr = line.substring(sp1 + 1, sp2);
        String action = line.substring(sp2 + 1);
        action.trim();
        if (action == "off_all") {
          halTurnOffAllActuators();
        } else {
          uint8_t id = (uint8_t)idStr.toInt();
          if (id < ACT_COUNT) {
            if (action == "on") halSetActuator(id, true);
            else if (action == "off") halSetActuator(id, false);
            else if (action == "pulse") halPulseActuator(id, 3000);
            else if (action == "invert") {
              bool currentLow = halGetActuatorTable()[id].activeLow;
              halSetActuatorPolarity(id, !currentLow);
            }
          }
        }
      }
      Serial.println("JSON_OK:{\"ok\":true}");
    } else if (line.startsWith("SERVO ")) {
      int angle = line.substring(6).toInt();
      halSetServoAngle(angle);
      Serial.println("JSON_OK:{\"ok\":true}");
    } else if (line.startsWith("MODE ")) {
      int eng = line.substring(5).toInt();
      fsmSetEngineerMode(eng == 1);
      Serial.println("JSON_OK:{\"ok\":true}");
    } else if (line.startsWith("WASHER ")) {
      int sp1 = line.indexOf(' ');
      int sp2 = line.indexOf(' ', sp1 + 1);
      String cmd = (sp2 > sp1) ? line.substring(sp1 + 1, sp2) : line.substring(sp1 + 1);
      int val = (sp2 > sp1) ? line.substring(sp2 + 1).toInt() : 0;
      cmd.trim();
      if (cmd == "load" && val >= 0 && val <= 2) fsmSetLoadSize((LoadSizeMode)val);
      else if (cmd == "soap" && val >= 0 && val <= 3) fsmSetSoapType((SoapTypeMode)val);
      else if (cmd == "pos" && val >= 1 && val <= 7) fsmSetKnobPosition((WasherKnobPos)val, true);
      else if (cmd == "start") fsmStartOrResume();
      else if (cmd == "pause") fsmPause();
      else if (cmd == "advance") fsmAdvanceNextStage();
      else if (cmd == "dryrun") fsmSetDryRunSimulation(val == 1);
      else if (cmd == "soaktime") fsmSetSoakDurationSeconds((uint32_t)val);
      Serial.println("JSON_OK:{\"ok\":true}");
    } else if (line == "RESET_EMERGENCY") {
      fsmClearEmergency();
      Serial.println("JSON_OK:{\"ok\":true}");
    } else if (line == "RESET_FLOW") {
      halResetFlowTotalizer();
      Serial.println("JSON_OK:{\"ok\":true}");
    } else if (line == "PT1_TARE") {
      halTarePt1Zero();
      Serial.println("JSON_OK:{\"ok\":true}");
    } else if (line.startsWith("PT1_CAL ")) {
      float cm = line.substring(8).toFloat();
      halCalibratePt1AtHeightCm(cm);
      Serial.println("JSON_OK:{\"ok\":true}");
    } else if (line == "PT1_RESET") {
      halResetPt1Calibration();
      Serial.println("JSON_OK:{\"ok\":true}");
    }
  }
}

// ============================================================================
// 4. SETUP Y LOOP PRINCIPAL (ARDUINO FRAMEWORK)
// ============================================================================
void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n\n*** INICIANDO SISTEMA AGUA DE GÍA · LAVADORA RETRO AÑOS 70 ***");

  halInit();
  fsmInit();
  reactorInit();
  initWifiAndOta();
}

void loop() {
  // 1. Actualizar capa eléctrica (ADC1, ISR Caudalímetro, DS18B20, Pulsos de 3s, Campana)
  halUpdate();
  reactorUpdate();

  // 2. Actualizar autómata de 7 posiciones de la Lavadora de Agua
  fsmUpdate();
  if (fsmGetStatus().emergencyStop && reactorIsRunning()) {
    reactorStop("Paro de emergencia", false);
  }

  // 3. Atender peticiones HTTP del celular y OTA
  server.handleClient();
  ArduinoOTA.handle();
  handleSerialBridge();

  // 4. Reconexión automática a red doméstica cada 10s si se perdió
  if (millis() - lastWifiCheckMs >= 10000) {
    lastWifiCheckMs = millis();
    if (WiFi.status() != WL_CONNECTED) {
      if (wifiMulti.run() == WL_CONNECTED) {
        Serial.printf("[STA] Reconectado a %s -> IP: http://%s\n",
                      WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
      }
    }
  }
}

