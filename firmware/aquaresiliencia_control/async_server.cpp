/**
 * ============================================================================
 * PROYECTO: AquaResiliencia Tijuana — Sistema Automatizado de Gestión Hídrica
 * HARDWARE: ESP8266 (ESP8266MOD / NodeMCU) y ESP32 DevKit
 * ARCHIVO:  async_server.cpp
 * ============================================================================
 */

#include "async_server.h"
#include "web_dashboard.h"
#include "sensors_manager.h"
#include "relay_controller.h"
#include "automation_logic.h"
#include "ota_manager.h"

#if defined(ESP8266)
  #include <ESP8266mDNS.h>
#else
  #include <ESPmDNS.h>
#endif

AsyncServerManager asyncServer;

// Callback estático de eventos de WebSocket
static void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        Serial.printf("[WS] Cliente #%u conectado desde %s\n", client->id(), client->remoteIP().toString().c_str());
        asyncServer.broadcastTelemetry();
    } else if (type == WS_EVT_DISCONNECT) {
        Serial.printf("[WS] Cliente #%u desconectado\n", client->id());
    } else if (type == WS_EVT_DATA) {
        AwsFrameInfo *info = (AwsFrameInfo*)arg;
        if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
            asyncServer.handleIncomingCommand((const char*)data, len);
        }
    }
}

AsyncServerManager::AsyncServerManager() : server(80), ws("/ws") {
    lastBroadcastTime = 0;
}

void AsyncServerManager::begin() {
    setupWiFi();
    setupWebSocket();
    setupRoutes();

    // Inicializar servidor HTTP asíncrono
    server.begin();
    Serial.println("[HTTP] Servidor Web Asincrono iniciado en puerto 80.");
}

void AsyncServerManager::setupWiFi() {
    Serial.println("\n[WIFI] Inicializando subsistema Wi-Fi...");

#if defined(ESP8266)
    // 1. Evitar reposo de radio y forzar máxima potencia de transmisión (evita desconexiones del AP)
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
    WiFi.setOutputPower(20.5);
#endif

    WiFi.persistent(false);
    WiFi.disconnect(true);
    WiFi.softAPdisconnect(true);
    delay(200);

    // 2. Escanear redes cercanas para detectar canal y nombre exacto del router
    WiFi.mode(WIFI_STA);
    delay(100);
    Serial.println("[WIFI] Escaneando redes 2.4 GHz cercanas...");
    int n = WiFi.scanNetworks();
    Serial.printf("[WIFI] Se detectaron %d redes:\n", n);

    String targetSSID = WIFI_STA_SSID;
    int targetChannel = 1;
    bool foundRouter = false;

    for (int i = 0; i < n; ++i) {
        String sName = WiFi.SSID(i);
        int32_t rssi = WiFi.RSSI(i);
        int32_t ch = WiFi.channel(i);
        Serial.printf("   [%d] \"%s\" (Canal %d | %d dBm)\n", i + 1, sName.c_str(), ch, rssi);

        String lowerName = sName;
        lowerName.toLowerCase();
        if (lowerName.indexOf("compuerta") >= 0 || lowerName.indexOf("ultima") >= 0) {
            targetSSID = sName;
            targetChannel = ch;
            foundRouter = true;
            Serial.printf("   --> ¡ROUTER ENCONTRADO!: \"%s\" en Canal %d\n", targetSSID.c_str(), targetChannel);
        }
    }

    // 3. Iniciar modo dual AP + STA en el mismo canal para evitar pérdida de señal
    WiFi.mode(WIFI_AP_STA);
    delay(100);

    IPAddress apIP(192, 168, 4, 1);
    IPAddress apGateway(192, 168, 4, 1);
    IPAddress apSubnet(255, 255, 255, 0);
    WiFi.softAPConfig(apIP, apGateway, apSubnet);

    bool apStarted = WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD, targetChannel, 0, 4);
    if (!apStarted) {
        WiFi.softAP(WIFI_AP_SSID); // Fallback abierto
    }
    Serial.println("\n[WIFI-AP] Red de respaldo transmitiendo de forma continua:");
    Serial.printf("   - Red AP:       %s\n", WIFI_AP_SSID);
    Serial.printf("   - Clave AP:     %s\n", WIFI_AP_PASSWORD);
    Serial.printf("   - Canal Radio:  Canal %d\n", targetChannel);
    Serial.printf("   - IP AP:        http://%s\n", WiFi.softAPIP().toString().c_str());

    // Iniciar servidor DNS cautivo para redirigir tráfico y evitar desconexiones celulares
    dnsServer.start(53, "*", apIP);
    Serial.println("[DNS] Servidor cautivo activo en puerto 53 (mantiene enlace estable en celular).");

    // 4. Conectar a la red del router ("la ultima compuerta")
    Serial.printf("\n[WIFI-STA] Enlazando con \"%s\" ...\n", targetSSID.c_str());
    WiFi.begin(targetSSID.c_str(), WIFI_STA_PASSWORD, targetChannel);

    unsigned long startMs = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startMs < 12000) {
        delay(300);
        Serial.print(".");
        // Parpadeo rápido del LED durante intento de enlace
        digitalWrite(PIN_STATUS_LED, !digitalRead(PIN_STATUS_LED));
    }
    Serial.println();

    Serial.println("========================================================");
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("   [OK] ¡CONECTADO CON EXITO A TU RED WI-FI!");
        Serial.printf("   - Red WiFi:     \"%s\"\n", targetSSID.c_str());
        Serial.printf("   - IP EN TU RED: http://%s\n", WiFi.localIP().toString().c_str());
        Serial.println("   -> Abre esa IP en el navegador de tu COMPUTADORA o CELULAR.");
#if defined(ESP8266)
        if (MDNS.begin("aquaresiliencia")) {
            MDNS.addService("http", "tcp", 80);
            Serial.println("   - Nombre local: http://aquaresiliencia.local");
        }
#endif
    } else {
        Serial.println("   [AVISO] No se pudo conectar a la red router tras 12s.");
        Serial.println("   -> Conéctate a la red Access Point directa emitida por el chip:");
        Serial.printf("   - Red AP Local: %s\n", WIFI_AP_SSID);
        Serial.printf("   - Clave AP:     %s\n", WIFI_AP_PASSWORD);
        Serial.printf("   - IP AP Local:  http://%s\n", WiFi.softAPIP().toString().c_str());
    }
    Serial.println("========================================================\n");
}

void AsyncServerManager::setupWebSocket() {
    ws.onEvent(onWsEvent);
    server.addHandler(&ws);
    Serial.println("[WS] Servidor WebSocket montado en ruta /ws");
}

void AsyncServerManager::setupRoutes() {
    // 1. Dashboard Principal (SPA embebida en Flash PROGMEM)
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", PAGE_INDEX_HTML);
    });

    // 2. API REST de respaldo (JSON)
    server.on("/api/telemetry", HTTP_GET, [](AsyncWebServerRequest *request) {
        JsonDocument doc;
        SensorData s = sensors.getData();

        doc["type"] = "telemetry";
        doc["timestamp"] = millis();

        JsonObject sens = doc["sensors"].to<JsonObject>();
        sens["p01_well_water"] = s.wellWaterPresent;
        sens["p01_well_raw"] = s.wellRawADC;
        sens["p02_ro_pressure_psi"] = s.roPressurePSI;
        sens["p05_totem2_full"] = s.totem2Full;
        sens["p06_totem1_min_ok"] = s.totem1HasMin20L;
        sens["p07_barrel_pressure_full"] = s.barrelCutoffReached;
        sens["p08_flow_lpm"] = s.flowRateLPM;
        sens["p08_total_liters"] = s.totalLiters;
        sens["p08_batch_liters"] = s.batchLiters;

        JsonObject rel = doc["relays"].to<JsonObject>();
        rel["relay1_pump"] = relays.getRelayState(RELAY_PUMP_WELL);
        rel["relay2_osmosis"] = relays.getRelayState(RELAY_OSMOSIS_VALV);
        rel["relay3_gravity"] = relays.getRelayState(RELAY_GRAVITY_VALV);
        rel["relay4_aux"] = relays.getRelayState(RELAY_AUX_CHANNEL);

        JsonObject ctrl = doc["control"].to<JsonObject>();
        ctrl["mode"] = relays.getOperationModeStr();
        ctrl["emergency_stop"] = relays.isEmergencyStopActive();
        ctrl["floc_target"] = automation.getFlocTargetVolume();
        ctrl["floc_batch_active"] = automation.isFlocBatchActive();
        ctrl["floc_progress_pct"] = automation.getFlocBatchProgressPercent();
        ctrl["last_event"] = automation.getLastSystemEvent();
        ctrl["uptime_sec"] = millis() / 1000UL;

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    // 3. Configurar endpoint de actualizacion de firmware por navegador (Web OTA)
    otaManager.setupWebOTA(server);

    // 4. Rutas de detección de portal cautivo para celulares Android e iOS
    server.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest *request) { request->redirect("/"); });
    server.on("/gen_204", HTTP_GET, [](AsyncWebServerRequest *request) { request->redirect("/"); });
    server.on("/hotspot-detect.html", HTTP_GET, [](AsyncWebServerRequest *request) { request->redirect("/"); });
    server.on("/canonical.html", HTTP_GET, [](AsyncWebServerRequest *request) { request->redirect("/"); });
    server.on("/connecttest.txt", HTTP_GET, [](AsyncWebServerRequest *request) { request->redirect("/"); });

    // 5. Manejador de rutas no encontradas (Redirección automática al panel de control)
    server.onNotFound([](AsyncWebServerRequest *request) {
        request->redirect("/");
    });
}

void AsyncServerManager::update() {
    dnsServer.processNextRequest();
#if defined(ESP8266)
    MDNS.update();
#endif
    ws.cleanupClients();

    unsigned long now = millis();
    if (now - lastBroadcastTime >= WS_BROADCAST_INTERVAL_MS) {
        lastBroadcastTime = now;
        broadcastTelemetry();
    }
}

void AsyncServerManager::broadcastTelemetry() {
    if (ws.count() == 0) return;

    JsonDocument doc;
    SensorData s = sensors.getData();

    doc["type"] = "telemetry";
    doc["timestamp"] = millis();

    JsonObject sens = doc["sensors"].to<JsonObject>();
    sens["p01_well_water"] = s.wellWaterPresent;
    sens["p01_well_raw"] = s.wellRawADC;
    sens["p02_ro_pressure_psi"] = s.roPressurePSI;
    sens["p05_totem2_full"] = s.totem2Full;
    sens["p06_totem1_min_ok"] = s.totem1HasMin20L;
    sens["p07_barrel_pressure_full"] = s.barrelCutoffReached;
    sens["p08_flow_lpm"] = s.flowRateLPM;
    sens["p08_total_liters"] = s.totalLiters;
    sens["p08_batch_liters"] = s.batchLiters;

    JsonObject rel = doc["relays"].to<JsonObject>();
    rel["relay1_pump"] = relays.getRelayState(RELAY_PUMP_WELL);
    rel["relay2_osmosis"] = relays.getRelayState(RELAY_OSMOSIS_VALV);
    rel["relay3_gravity"] = relays.getRelayState(RELAY_GRAVITY_VALV);
    rel["relay4_aux"] = relays.getRelayState(RELAY_AUX_CHANNEL);

    JsonObject ctrl = doc["control"].to<JsonObject>();
    ctrl["mode"] = relays.getOperationModeStr();
    ctrl["emergency_stop"] = relays.isEmergencyStopActive();
    ctrl["floc_target"] = automation.getFlocTargetVolume();
    ctrl["floc_batch_active"] = automation.isFlocBatchActive();
    ctrl["floc_progress_pct"] = automation.getFlocBatchProgressPercent();
    ctrl["last_event"] = automation.getLastSystemEvent();
    ctrl["uptime_sec"] = millis() / 1000UL;

    String jsonBuffer;
    serializeJson(doc, jsonBuffer);
    ws.textAll(jsonBuffer);
}

void AsyncServerManager::handleIncomingCommand(const char* payload, size_t len) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload, len);
    if (error) {
        Serial.printf("[WS] Error parseando JSON entrante: %s\n", error.c_str());
        return;
    }

    const char* action = doc["action"];
    if (!action) return;

    Serial.printf("[WS-CMD] Accion recibida: %s\n", action);

    if (strcmp(action, "emergency_stop") == 0) {
        relays.triggerEmergencyStop();
        broadcastTelemetry();
    } else if (strcmp(action, "emergency_reset") == 0) {
        relays.resetEmergencyStop();
        broadcastTelemetry();
    } else if (strcmp(action, "set_mode") == 0) {
        const char* modeStr = doc["mode"];
        if (modeStr) {
            if (strcmp(modeStr, "AUTO") == 0) relays.setOperationMode(MODE_AUTO);
            else if (strcmp(modeStr, "MANUAL") == 0) relays.setOperationMode(MODE_MANUAL);
        }
        broadcastTelemetry();
    } else if (strcmp(action, "toggle_relay") == 0) {
        int relayNum = doc["relay"] | 0;
        bool state = doc["state"] | false;
        if (relayNum >= 1 && relayNum <= 4) {
            relays.setRelay(relayNum - 1, state, true);
        }
        broadcastTelemetry();
    } else if (strcmp(action, "set_floc_target") == 0) {
        float val = doc["value"] | DEFAULT_FLOC_TARGET_LITERS;
        automation.setFlocTargetVolume(val);
        broadcastTelemetry();
    } else if (strcmp(action, "start_floc_batch") == 0) {
        automation.startFlocBatch();
        broadcastTelemetry();
    } else if (strcmp(action, "stop_floc_batch") == 0) {
        automation.stopFlocBatch();
        broadcastTelemetry();
    } else if (strcmp(action, "reset_total_volume") == 0) {
        sensors.resetTotalVolume();
        broadcastTelemetry();
    }
}
