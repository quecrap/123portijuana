/**
 * ============================================================================
 * PROYECTO: AquaResiliencia Tijuana — Sistema Automatizado de Gestión Hídrica
 * FIRMWARE: ESP32 DevKit v1 (WROOM-32)
 * ARCHIVO:  async_server.h
 * ============================================================================
 */

#ifndef ASYNC_SERVER_H
#define ASYNC_SERVER_H

#include <Arduino.h>
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#else
  #include <WiFi.h>
#endif
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <DNSServer.h>
#include "config.h"

class AsyncServerManager {
public:
    AsyncServerManager();
    void begin();
    void update();

    // Emisión periódica de telemetría a todos los clientes WebSocket
    void broadcastTelemetry();

    // Procesamiento de comandos JSON entrantes
    void handleIncomingCommand(const char* payload, size_t len);

private:
    AsyncWebServer server;
    AsyncWebSocket ws;
    DNSServer dnsServer;
    unsigned long lastBroadcastTime;

    void setupWiFi();
    void setupRoutes();
    void setupWebSocket();
};

extern AsyncServerManager asyncServer;

#endif // ASYNC_SERVER_H
