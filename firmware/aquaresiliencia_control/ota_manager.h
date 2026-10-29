/**
 * ============================================================================
 * PROYECTO: AquaResiliencia Tijuana — Sistema Automatizado de Gestión Hídrica
 * HARDWARE: ESP8266 (ESP8266MOD / NodeMCU) y ESP32 DevKit
 * ARCHIVO:  ota_manager.h
 * ============================================================================
 */

#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESPAsyncWebServer.h>

#if defined(ESP8266)
  #include <Updater.h>
#else
  #include <Update.h>
#endif

class OTAManager {
public:
    OTAManager();
    void begin();
    void handle();
    void setupWebOTA(AsyncWebServer& server);

    bool isUpdating() const;

private:
    bool updatingActive;
};

extern OTAManager otaManager;

#endif // OTA_MANAGER_H
