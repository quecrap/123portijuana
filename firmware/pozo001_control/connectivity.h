#ifndef CONNECTIVITY_H
#define CONNECTIVITY_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "config.h"

class ConnectivityManager {
public:
    ConnectivityManager();
    void begin();
    void update(); // Atender clientes web y procesamiento de UART del SIM800L

    // Envío de alertas de emergencia por SMS
    bool sendSMS(const String& phoneNumber, const String& message);
    
    // Consulta de estado de módem GSM
    bool isGSMReady();
    int getGSMSignalQuality();

private:
    WebServer server;
    bool gsmReady;
    int gsmSignalCSQ;
    String gsmRxBuffer;
    int ringCount;
    bool inCall;
    unsigned long lastGSMKeepAlive;

    void setupWiFi();
    void setupWebServer();
    void setupSIM800L();

    void processGSMIncoming();
    void sendATCommand(const String& cmd, unsigned long timeout = 1000);
    
    // Handlers del servidor web
    void handleRoot();
    void handleStatusApi();
    void handleControlApi();
    void handleNotFound();
};

extern ConnectivityManager connectivity;

#endif // CONNECTIVITY_H
