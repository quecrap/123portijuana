#include "dtmf_handler.h"
#include "power_mgmt.h"
#include "sensors.h"

DTMFHandler dtmfHandler;

DTMFHandler::DTMFHandler() {
    lastTone = '\0';
    lastToneTime = 0;
}

void DTMFHandler::begin() {
    Serial.println("[DTMF] Decodificador telefónico inicializado.");
}

void DTMFHandler::processTone(char tone) {
    Serial.printf("[DTMF] Tono telefónico recibido: %c\n", tone);

    lastTone = tone;
    lastToneTime = millis();
    SensorData s = sensors.getData();

    switch (tone) {
        case '1':
            // Tecla 1: Consulta de estado por consola / beep
            Serial.println("[DTMF CMD 1] Telemetría rápida del sistema:");
            Serial.printf(" - Nivel Pozo Seguro: %s\n", s.wellLevelSafe ? "SI" : "¡POZO SECO!");
            Serial.printf(" - Presión T1 (60-90 PSI): %s\n", s.pressureT1Cutoff ? "CORTE 90 PSI" : "NORMAL");
            Serial.printf(" - Barril T2 (20-40 PSI): %s\n", s.t2CleanWaterFull ? "LLENO 40 PSI" : "LLENANDO");
            Serial.printf(" - Temperatura Agua RO: %.1f °C (%s)\n", s.waterTempC, s.waterTempOverheat ? "¡ALERTA TÉRMICA!" : "OK");
            Serial.printf(" - Bomba Pozo: %s\n", powerMgmt.isWellPumpRunning() ? "OPERANDO" : "APAGADA");
            Serial.printf(" - Booster RO: %s\n", powerMgmt.isROBoosterRunning() ? "OPERANDO" : (powerMgmt.getROState() == RO_LOCKED_OVERHEAT ? "BLOQUEO TÉRMICO" : "APAGADA"));
            Serial.printf(" - Electroválvula 120V: %s\n", powerMgmt.isSolenoid120VOpen() ? "ABIERTA" : "CERRADA");
            break;

        case '2':
            // Tecla 2: Iniciar ciclo de extracción Bomba 1 Pozo (24V)
            Serial.println("[DTMF CMD 2] Encendiendo extracción Bomba 1...");
            powerMgmt.setManualExtraction(true);
            break;

        case '3':
            // Tecla 3: Paro de emergencia general (apaga todo de inmediato)
            Serial.println("[DTMF CMD 3] PARO DE EMERGENCIA GENERAL: Todo apagado.");
            powerMgmt.emergencyStop();
            break;

        case '4':
            // Tecla 4: Abrir electroválvula de 120 VAC
            Serial.println("[DTMF CMD 4] Electroválvula 120V abierta manualmente.");
            powerMgmt.setSolenoid120V(true);
            break;

        case '5':
            // Tecla 5: Cerrar electroválvula de 120 VAC
            Serial.println("[DTMF CMD 5] Electroválvula 120V cerrada manualmente.");
            powerMgmt.setSolenoid120V(false);
            break;

        case '6':
            // Tecla 6: Forzar encendido de Bomba Booster RO
            Serial.println("[DTMF CMD 6] Bomba Booster RO encendida manualmente.");
            powerMgmt.setROBooster(true);
            break;

        case '7':
            // Tecla 7: Apagar Bomba Booster RO
            Serial.println("[DTMF CMD 7] Bomba Booster RO apagada.");
            powerMgmt.setROBooster(false);
            break;

        case '9':
            // Tecla 9: Reiniciar contador de litros acumulados a 0 L
            Serial.println("[DTMF CMD 9] Volumen total reseteado a 0 L.");
            sensors.resetTotalVolume();
            break;

        case '0':
        case '#':
            // Tecla 0 o #: Finalizar y colgar llamada
            Serial.println("[DTMF CMD #/0] Solicitud de colgar llamada.");
            break;

        default:
            Serial.printf("[DTMF] Tecla no asignada: %c\n", tone);
            break;
    }
}

void DTMFHandler::sendAudioFeedback(HardwareSerial& gsmSerial, char tone) {
    // Genera tono audible DTMF de confirmación en el auricular del operador (Comando AT+VTS)
    gsmSerial.print("AT+VTS=\"");
    gsmSerial.print(tone);
    gsmSerial.println("\"");
}
