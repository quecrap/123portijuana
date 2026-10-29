#include "connectivity.h"
#include "sensors.h"
#include "power_mgmt.h"
#include "dtmf_handler.h"

ConnectivityManager connectivity;

ConnectivityManager::ConnectivityManager() : server(80) {
    gsmReady = false;
    gsmSignalCSQ = 0;
    gsmRxBuffer = "";
    ringCount = 0;
    inCall = false;
    lastGSMKeepAlive = 0;
}

void ConnectivityManager::begin() {
    setupWiFi();
    setupWebServer();
    setupSIM800L();
}

void ConnectivityManager::setupWiFi() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);

    Serial.println("\n[WIFI] Punto de Acceso iniciado:");
    Serial.print(" - SSID: ");
    Serial.println(WIFI_AP_SSID);
    Serial.print(" - IP Local: ");
    Serial.println(WiFi.softAPIP());
}

void ConnectivityManager::setupWebServer() {
    // 1. Dashboard HTML con autenticación Basic HTTP
    server.on("/", HTTP_GET, [this]() {
        if (!server.authenticate(HTTP_AUTH_USER, HTTP_AUTH_PASS)) {
            return server.requestAuthentication();
        }
        handleRoot();
    });

    // 2. API REST: Estatus del sistema en JSON
    server.on("/api/status", HTTP_GET, [this]() {
        if (!server.authenticate(HTTP_AUTH_USER, HTTP_AUTH_PASS)) {
            return server.requestAuthentication();
        }
        handleStatusApi();
    });

    // 3. API REST: Control manual de actuadores
    server.on("/api/control", HTTP_POST, [this]() {
        if (!server.authenticate(HTTP_AUTH_USER, HTTP_AUTH_PASS)) {
            return server.requestAuthentication();
        }
        handleControlApi();
    });

    server.onNotFound([this]() { handleNotFound(); });

    server.begin();
    Serial.println("[WEB] Servidor HTTP listo en puerto 80.");
}

void ConnectivityManager::setupSIM800L() {
    // UART2 en pines 16 (RX) y 17 (TX) a 9600 baud
    Serial2.begin(9600, SERIAL_8N1, PIN_GSM_RX, PIN_GSM_TX);
    delay(800);

    Serial.println("[GSM] Inicializando módem SIM800L...");
    sendATCommand("AT");            // Sincronización
    sendATCommand("ATE0");          // Eco OFF
    sendATCommand("AT+CLIP=1");     // Identificador de llamadas
    sendATCommand("AT+DDET=1");     // Detección de tonos DTMF
    sendATCommand("AT+CMGF=1");     // SMS en modo texto

    gsmReady = true;
    Serial.println("[GSM] SIM800L configurado con soporte DTMF.");
}

void ConnectivityManager::update() {
    server.handleClient();
    processGSMIncoming();
}

void ConnectivityManager::processGSMIncoming() {
    while (Serial2.available()) {
        char c = (char)Serial2.read();
        if (c == '\r') continue;

        if (c == '\n') {
            if (gsmRxBuffer.length() > 0) {
                gsmRxBuffer.trim();

                // 1. Timbrazo de llamada entrante
                if (gsmRxBuffer.indexOf("RING") >= 0) {
                    ringCount++;
                    Serial.printf("[GSM] Llamada entrante (Ring %d)...\n", ringCount);
                    if (ringCount >= 2 && !inCall) {
                        Serial.println("[GSM] Contestando llamada (ATA)...");
                        Serial2.println("ATA");
                        inCall = true;
                        ringCount = 0;
                    }
                }
                // 2. Detección de tono DTMF: +DTMF: <char>
                else if (gsmRxBuffer.indexOf("+DTMF:") >= 0) {
                    int idx = gsmRxBuffer.indexOf("+DTMF:");
                    char tone = gsmRxBuffer.charAt(idx + 7);
                    Serial.printf("[GSM] DTMF recibido: %c\n", tone);
                    
                    dtmfHandler.processTone(tone);

                    if (tone == '#' || tone == '0') {
                        Serial2.println("ATH");
                        inCall = false;
                    } else {
                        dtmfHandler.sendAudioFeedback(Serial2, tone);
                    }
                }
                // 3. Fin de llamada
                else if (gsmRxBuffer.indexOf("NO CARRIER") >= 0 || gsmRxBuffer.indexOf("BUSY") >= 0) {
                    Serial.println("[GSM] Llamada concluida.");
                    inCall = false;
                    ringCount = 0;
                }

                gsmRxBuffer = "";
            }
        } else {
            gsmRxBuffer += c;
        }
    }
}

void ConnectivityManager::sendATCommand(const String& cmd, unsigned long timeout) {
    Serial2.println(cmd);
    unsigned long start = millis();
    while (millis() - start < timeout) {
        if (Serial2.available()) {
            String resp = Serial2.readString();
            Serial.print("[AT RESP] ");
            Serial.println(resp);
            break;
        }
    }
}

bool ConnectivityManager::sendSMS(const String& phoneNumber, const String& message) {
    if (!gsmReady) return false;

    Serial.printf("[GSM] Enviando SMS a %s...\n", phoneNumber.c_str());
    Serial2.print("AT+CMGS=\"");
    Serial2.print(phoneNumber);
    Serial2.println("\"");
    delay(300);

    Serial2.print(message);
    Serial2.write(26); // Ctrl+Z
    delay(3000);

    Serial.println("[GSM] SMS despachado.");
    return true;
}

// --------------------------- WEB DASHBOARD -----------------------------------

void ConnectivityManager::handleRoot() {
    SensorData s = sensors.getData();
    bool p1On = powerMgmt.isWellPumpRunning();
    bool roOn = powerMgmt.isROBoosterRunning();
    bool val120On = powerMgmt.isSolenoid120VOpen();
    bool autoOn = powerMgmt.isAutoMode();

    String html = "<!DOCTYPE html><html lang='es'><head><meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<title>AquaResiliencia — Nodo Pozo 001</title>";
    html += "<style>";
    html += "body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; background: #090d16; color: #f1f5f9; margin: 0; padding: 16px; }";
    html += ".wrap { max-width: 600px; margin: auto; background: #131d2e; border-radius: 16px; padding: 22px; border: 1px solid #1e293b; box-shadow: 0 10px 30px rgba(0,0,0,0.6); }";
    html += "h1 { color: #38bdf8; font-size: 20px; margin: 0 0 16px; border-bottom: 1px solid #1e293b; padding-bottom: 10px; }";
    html += ".grid { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; margin-bottom: 18px; }";
    html += ".card { background: #0c1421; padding: 12px; border-radius: 10px; border: 1px solid #1f2d42; }";
    html += ".label { font-size: 11px; color: #94a3b8; text-transform: uppercase; font-weight: 700; letter-spacing: 0.5px; }";
    html += ".val { font-size: 18px; font-weight: bold; color: #ffffff; margin-top: 4px; }";
    html += ".tag { display: inline-block; padding: 3px 8px; border-radius: 6px; font-size: 11px; font-weight: bold; margin-top: 4px; }";
    html += ".tag-ok { background: #064e3b; color: #34d399; } .tag-warn { background: #78350f; color: #fde047; } .tag-err { background: #7f1d1d; color: #fca5a5; }";
    html += ".btn { display: block; width: 100%; padding: 12px; border-radius: 8px; font-size: 14px; font-weight: bold; cursor: pointer; border: none; margin-bottom: 8px; text-align: center; }";
    html += ".btn-danger { background: #dc2626; color: white; }";
    html += ".btn-primary { background: #0284c7; color: white; }";
    html += ".btn-sec { background: #334155; color: #f1f5f9; }";
    html += ".sec-title { font-size: 14px; color: #cbd5e1; margin: 18px 0 10px; text-transform: uppercase; letter-spacing: 0.5px; }";
    html += "</style></head><body>";
    html += "<div class='wrap'>";
    html += "<h1>💧 Pozo 001 — Centinela & Osmosis Inversa</h1>";
    
    html += "<div class='grid'>";
    html += "<div class='card'><div class='label'>Pozo (Fondo 7.30m)</div>";
    html += s.wellLevelSafe ? "<div class='tag tag-ok'>AGUA SEGURA (0.80m)</div>" : "<div class='tag tag-err'>⚠️ ¡POZO SECO!</div>";
    html += "</div>";

    html += "<div class='card'><div class='label'>Presostato T1 (60-90)</div>";
    html += s.pressureT1Cutoff ? "<div class='tag tag-warn'>CORTE 90 PSI</div>" : "<div class='tag tag-ok'>PRESIÓN OK</div>";
    html += "</div>";

    html += "<div class='card'><div class='label'>Barril T2 Limpio (20-40)</div>";
    html += s.t2CleanWaterFull ? "<div class='tag tag-ok'>LLENO (40 PSI)</div>" : "<div class='tag tag-warn'>LLENANDO (&lt;40 PSI)</div>";
    html += "</div>";

    html += "<div class='card'><div class='label'>Caudal Steren</div><div class='val'>" + String(s.flowRateLPM, 1) + " L/min</div></div>";
    html += "<div class='card'><div class='label'>Volumen Acumulado</div><div class='val'>" + String(s.totalLiters, 1) + " L</div></div>";
    html += "<div class='card'><div class='label'>Cabina DHT22</div><div class='val'>" + String(s.temperatureC, 1) + "°C | " + String(s.humidityPercent, 0) + "%</div></div>";
    
    html += "<div class='card'><div class='label'>Agua Intercambiador RO</div>";
    html += "<div class='val'>" + String(s.waterTempC, 1) + "°C</div>";
    html += s.waterTempOverheat ? "<div class='tag tag-err'>🔥 SOBRETEMPERATURA (&gt;32°C)</div>" : "<div class='tag tag-ok'>TEMPERATURA SEGURA</div>";
    html += "</div>";
    html += "</div>";

    html += "<div class='sec-title'>Estado de Actuadores</div>";
    html += "<div class='grid'>";
    html += "<div class='card'><div class='label'>Bomba 1 Pozo (24V)</div><div class='val'>" + String(p1On ? "⚡ TRABAJANDO" : "APAGADA") + "</div></div>";
    html += "<div class='card'><div class='label'>Booster RO</div><div class='val'>";
    if (powerMgmt.getROState() == RO_LOCKED_OVERHEAT) {
        html += "<span style='color:#ef4444;'>🛑 BLOQUEO TÉRMICO</span>";
    } else {
        html += String(roOn ? "⚡ ACTIVA (~80 PSI)" : "APAGADA");
    }
    html += "</div></div>";
    html += "<div class='card'><div class='label'>Electroválvula 120V</div><div class='val'>" + String(val120On ? "ABIERTA" : "CERRADA") + "</div></div>";
    html += "<div class='card'><div class='label'>Modo Sistema</div><div class='val'>" + String(autoOn ? "AUTOMÁTICO" : "MANUAL") + "</div></div>";
    html += "</div>";

    html += "<div class='sec-title'>Control y Enclavamientos</div>";
    html += "<button class='btn btn-danger' onclick=\"fetch('/api/control?action=stop',{method:'POST'}).then(()=>location.reload())\">🛑 PARO DE EMERGENCIA GENERAL</button>";
    
    if (autoOn) {
        html += "<button class='btn btn-sec' onclick=\"fetch('/api/control?auto=0',{method:'POST'}).then(()=>location.reload())\">Cambiar a Modo Manual</button>";
    } else {
        html += "<button class='btn btn-primary' onclick=\"fetch('/api/control?auto=1',{method:'POST'}).then(()=>location.reload())\">Activar Modo Automático</button>";
    }

    if (p1On) {
        html += "<button class='btn btn-sec' onclick=\"fetch('/api/control?pump1=0',{method:'POST'}).then(()=>location.reload())\">Apagar Bomba Pozo (24V)</button>";
    } else {
        html += "<button class='btn btn-primary' onclick=\"fetch('/api/control?pump1=1',{method:'POST'}).then(()=>location.reload())\">Encender Bomba Pozo (Pulsos 30s/60s)</button>";
    }

    if (roOn) {
        html += "<button class='btn btn-sec' onclick=\"fetch('/api/control?ro=0',{method:'POST'}).then(()=>location.reload())\">Detener Booster RO y Válvula 120V</button>";
    } else {
        html += "<button class='btn btn-primary' onclick=\"fetch('/api/control?ro=1',{method:'POST'}).then(()=>location.reload())\">Forzar Llenado T2 (Booster + Válvula 120V)</button>";
    }

    html += "<button class='btn btn-sec' style='margin-top:8px;' onclick=\"fetch('/api/control?reset_liters=1',{method:'POST'}).then(()=>location.reload())\">Resetear Contador Litros</button>";

    html += "<p style='font-size:11px;color:#64748b;margin-top:16px;'>Control por llamada celular DTMF: Marca al SIM800L (1: Estado, 2: Bomba 1 ON, 3: Paro Total, 4/5: Válvula 120V, 6/7: Booster RO, 9: Reset L, #: Colgar).</p>";
    html += "</div></body></html>";

    server.send(200, "text/html", html);
}

void ConnectivityManager::handleStatusApi() {
    SensorData s = sensors.getData();

    String json = "{";
    json += "\"well_level_safe\":" + String(s.wellLevelSafe ? "true" : "false") + ",";
    json += "\"pressure_t1_cutoff_90psi\":" + String(s.pressureT1Cutoff ? "true" : "false") + ",";
    json += "\"t2_clean_water_full_40psi\":" + String(s.t2CleanWaterFull ? "true" : "false") + ",";
    json += "\"flow_lpm\":" + String(s.flowRateLPM, 2) + ",";
    json += "\"total_liters\":" + String(s.totalLiters, 2) + ",";
    json += "\"temperature_c\":" + String(s.temperatureC, 1) + ",";
    json += "\"humidity_percent\":" + String(s.humidityPercent, 1) + ",";
    json += "\"water_temp_c\":" + String(s.waterTempC, 1) + ",";
    json += "\"water_temp_overheat\":" + String(s.waterTempOverheat ? "true" : "false") + ",";
    json += "\"pump_well_running\":" + String(powerMgmt.isWellPumpRunning() ? "true" : "false") + ",";
    json += "\"ro_booster_running\":" + String(powerMgmt.isROBoosterRunning() ? "true" : "false") + ",";
    json += "\"solenoid_120v_open\":" + String(powerMgmt.isSolenoid120VOpen() ? "true" : "false") + ",";
    json += "\"ro_locked_overheat\":" + String(powerMgmt.getROState() == RO_LOCKED_OVERHEAT ? "true" : "false") + ",";
    json += "\"auto_mode\":" + String(powerMgmt.isAutoMode() ? "true" : "false");
    json += "}";

    server.send(200, "application/json", json);
}

void ConnectivityManager::handleControlApi() {
    if (server.hasArg("action") && server.arg("action") == "stop") {
        powerMgmt.emergencyStop();
    }
    if (server.hasArg("auto")) {
        powerMgmt.setAutoMode(server.arg("auto").toInt() == 1);
    }
    if (server.hasArg("pump1")) {
        powerMgmt.setManualExtraction(server.arg("pump1").toInt() == 1);
    }
    if (server.hasArg("ro")) {
        bool ro = (server.arg("ro").toInt() == 1);
        powerMgmt.setROBooster(ro);
        powerMgmt.setSolenoid120V(ro);
    }
    if (server.hasArg("reset_liters")) {
        sensors.resetTotalVolume();
    }

    server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void ConnectivityManager::handleNotFound() {
    server.send(404, "text/plain", "Recurso no encontrado en nodo ESP32 Pozo 001.");
}

bool ConnectivityManager::isGSMReady() {
    return gsmReady;
}

int ConnectivityManager::getGSMSignalQuality() {
    return gsmSignalCSQ;
}
