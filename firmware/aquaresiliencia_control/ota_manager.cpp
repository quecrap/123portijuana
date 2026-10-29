/**
 * ============================================================================
 * PROYECTO: AquaResiliencia Tijuana — Sistema Automatizado de Gestión Hídrica
 * HARDWARE: ESP8266 (ESP8266MOD / NodeMCU) y ESP32 DevKit
 * ARCHIVO:  ota_manager.cpp
 * ============================================================================
 */

#include "ota_manager.h"
#include "relay_controller.h"

OTAManager otaManager;

OTAManager::OTAManager() {
    updatingActive = false;
}

void OTAManager::begin() {
    ArduinoOTA.setPort(8266);
#if defined(ESP8266)
    ArduinoOTA.setHostname("AquaResiliencia-ESP8266");
#else
    ArduinoOTA.setHostname("AquaResiliencia-ESP32");
#endif
    ArduinoOTA.setPassword("AquaResiliencia2026");

    ArduinoOTA.onStart([this]() {
        updatingActive = true;
        Serial.println("\n[OTA-WIFI] Iniciando actualizacion inalambrica...");
        // Apagar todos los reles antes de escribir en la memoria Flash
        relays.triggerEmergencyStop();
    });

    ArduinoOTA.onEnd([this]() {
        updatingActive = false;
        Serial.println("\n[OTA-WIFI] Actualizacion finalizada con exito! Reiniciando...");
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        if (total > 0) {
            Serial.printf("[OTA-WIFI] Progreso: %u%%\r", (progress / (total / 100)));
        }
    });

    ArduinoOTA.onError([this](ota_error_t error) {
        updatingActive = false;
        Serial.printf("[OTA-WIFI] Error [%u]\n", error);
    });

#if defined(ESP8266)
    ArduinoOTA.begin(false); // Sin mDNS en modo AP para evitar cuelgues de red
#else
    ArduinoOTA.begin();
#endif
    Serial.println("[OTA] Servicio ArduinoOTA iniciado.");
}

void OTAManager::handle() {
    ArduinoOTA.handle();
}

void OTAManager::setupWebOTA(AsyncWebServer& server) {
    server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {
        bool shouldReboot = !Update.hasError();
        AsyncWebServerResponse *response = request->beginResponse(
            shouldReboot ? 200 : 500, 
            "text/plain", 
            shouldReboot ? "OK" : "ERROR_OTA"
        );
        response->addHeader("Connection", "close");
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
        
        if (shouldReboot) {
            Serial.println("[OTA-WEB] Reiniciando en 1.5 segundos...");
            delay(1500);
            ESP.restart();
        }
    }, [this](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
        if (!index) {
            updatingActive = true;
            Serial.printf("\n[OTA-WEB] Recibiendo nuevo binario: %s\n", filename.c_str());
            
            // Apagar relés por seguridad física
            relays.triggerEmergencyStop();

#if defined(ESP8266)
            size_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & ~0xFFF;
            if (!Update.begin(maxSketchSpace)) {
                Update.printError(Serial);
            }
#else
            if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
                Update.printError(Serial);
            }
#endif
        }

        if (!Update.hasError()) {
            if (Update.write(data, len) != len) {
                Update.printError(Serial);
            }
        }

        if (final) {
            if (Update.end(true)) {
                Serial.printf("[OTA-WEB] Firmware cargado exitosamente: %u bytes totales.\n", (unsigned int)(index + len));
            } else {
                Update.printError(Serial);
            }
            updatingActive = false;
        }
    });

    Serial.println("[OTA] Endpoint Web OTA configurado en http://192.168.4.1/update");
}

bool OTAManager::isUpdating() const {
    return updatingActive;
}
