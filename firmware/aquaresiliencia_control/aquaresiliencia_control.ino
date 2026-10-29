/**
 * ============================================================================
 * PROYECTO: AquaResiliencia Tijuana — Sistema Automatizado de Gestión Hídrica
 * FIRMWARE: ESP32 DevKit v1 (WROOM-32)
 * ARCHIVO:  aquaresiliencia_control.ino
 * ============================================================================
 * Nodo Inteligente de Control Hidráulico, Electrofloculación y Ósmosis Inversa
 * Servidor Web Asíncrono + WebSockets + Actualización Inalámbrica WiFi (OTA)
 * ============================================================================
 */

#include "config.h"
#include <ArduinoJson.h>
#include "sensors_manager.h"
#include "relay_controller.h"
#include "automation_logic.h"
#include "ota_manager.h"
#include "async_server.h"

static unsigned long lastSerialTelemetry = 0;

void setup() {
    // 1. Inicializar comunicación serial USB para diagnóstico
    Serial.begin(SERIAL_BAUD_RATE);
    delay(1000);

    Serial.println("\n========================================================");
    Serial.println("   💧 AQUARESILIENCIA TIJUANA — SISTEMA AUTOMATIZADO    ");
    Serial.println("   Control Embebido ESP32 + WebSockets + Web OTA v3.0   ");
    Serial.println("========================================================");

    // 2. Inicializar relés en modo seguro (FAIL-SAFE OFF)
    Serial.println("[BOOT 1/5] Inicializando módulo de 4 relés (SRD-05VDC)...");
    relays.begin();

    // 3. Inicializar sensores físicos y rutina de interrupción de caudal
    Serial.println("[BOOT 2/5] Inicializando sensores [P-01] a [P-08]...");
    sensors.begin();

    // 4. Inicializar motor de lógica y automatización
    Serial.println("[BOOT 3/5] Cargando reglas de automatización hidráulica...");
    automation.begin();

    // 5. Inicializar conectividad WiFi SoftAP, WebSockets y servidor web
    Serial.println("[BOOT 4/5] Levantando Punto de Acceso WiFi y servidor asíncrono...");
    asyncServer.begin();

    // 6. Inicializar servicio de actualización inalámbrica (OTA)
    Serial.println("[BOOT 5/5] Activando servicios OTA (ArduinoOTA + Web Browser)...");
    otaManager.begin();

    Serial.println("\n========================================================");
    Serial.println("   ✅ ¡SISTEMA OPERANDO Y LISTO EN CAMPO!              ");
    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("   - Conectado a Router:  \"%s\"\n", WIFI_STA_SSID);
        Serial.printf("   - IP EN TU RED (PC/Cel): http://%s\n", WiFi.localIP().toString().c_str());
        Serial.println("   - Enlace mDNS Local:   http://aquaresiliencia.local");
    }
    Serial.printf("   - Red WiFi AP Respaldo: %s\n", WIFI_AP_SSID);
    Serial.printf("   - Clave WiFi AP:        %s\n", WIFI_AP_PASSWORD);
    Serial.printf("   - Panel Web AP:         http://%s\n", WiFi.softAPIP().toString().c_str());
    Serial.println("========================================================\n");
}

void loop() {
    // 0. Ceder tiempo al sistema de radio y watchdog del microcontrolador
    yield();

    // 1. Atención del servicio de actualización inalámbrica OTA
    otaManager.handle();

    // Si se está flasheando un nuevo firmware por OTA, pausar el ciclo de control
    if (otaManager.isUpdating()) {
        delay(10);
        return;
    }

    // 2. Lectura y filtrado digital de sensores hidráulicos
    sensors.update();

    // 3. Evaluación de las 3 reglas de automatización y enclavamientos
    automation.update();

    // 4. Transmisión periódica de telemetría JSON y gestión WebSocket
    asyncServer.update();

    // 5. Parpadeo de LED de latido (Heartbeat) para confirmar que el chip está vivo
    unsigned long now = millis();
    static unsigned long lastHeartbeat = 0;
    static bool heartbeatLed = false;
    if (now - lastHeartbeat >= 1000) {
        lastHeartbeat = now;
        heartbeatLed = !heartbeatLed;
        digitalWrite(PIN_STATUS_LED, heartbeatLed ? LOW : HIGH); // LOW enciende el LED azul en ESP8266MOD
    }

    // 6. Telemetría de diagnóstico por consola USB cada 3 segundos
    if (now - lastSerialTelemetry >= 3000) {
        lastSerialTelemetry = now;
        SensorData s = sensors.getData();

        Serial.print("[TELEMETRIA] ");
        if (WiFi.status() == WL_CONNECTED) {
            Serial.printf("IP: http://%s | ", WiFi.localIP().toString().c_str());
        } else {
            Serial.printf("AP IP: http://%s | ", WiFi.softAPIP().toString().c_str());
        }
        Serial.printf("Pozo[P01]: %s | ", s.wellWaterPresent ? "AGUA OK" : "SECO");
        Serial.printf("RO[P02]: %.1f PSI | ", s.roPressurePSI);
        Serial.printf("T2[P05]: %s | ", s.totem2Full ? "LLENO" : "LIBRE");
        Serial.printf("T1[P06]: %s | ", s.totem1HasMin20L ? ">=20L OK" : "<20L BAJO");
        Serial.printf("Barril[P07]: %s | ", s.barrelCutoffReached ? "60 PSI CORTE" : "NORMAL");
        Serial.printf("Caudal[P08]: %.2f L/m (Tot: %.1f L) | ", s.flowRateLPM, s.totalLiters);
        Serial.printf("Reles: [Bomba:%d RO:%d Grav:%d Aux:%d] | ", 
            relays.getRelayState(RELAY_PUMP_WELL),
            relays.getRelayState(RELAY_OSMOSIS_VALV),
            relays.getRelayState(RELAY_GRAVITY_VALV),
            relays.getRelayState(RELAY_AUX_CHANNEL)
        );
        Serial.printf("Modo: %s | Estado: %s\n", relays.getOperationModeStr(), automation.getDiagnosticStateStr());
    }
}
