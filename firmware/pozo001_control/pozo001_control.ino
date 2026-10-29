/**
 * ============================================================================
 * PROYECTO: AquaResiliencia Tijuana — Nodo Centinela IoT (Pozo 001)
 * HARDWARE: ESP32 Dev Module + SIM800L GSM + Relés 120VAC / 24VDC
 * CONTROL:  Wi-Fi Web Dashboard + Control por Llamada Telefónica DTMF
 * ============================================================================
 * Tren de Tratamiento y Lógica:
 * - Pozo (6.50m a 7.30m, columna útil 80cm) -> Bomba 1 (24V 1A, pulsos 30s/60s)
 * - T1 (Barril presurizado) con presostato mecánico (corte 90 PSI / arranque 60 PSI)
 * - Prefiltración: Sedimentos -> Carbón Activado -> Caudalímetro Steren
 * - Ósmosis Inversa: Bomba Booster RO (~80 PSI) + Electroválvula (120 VAC)
 * - T2 (Barril Agua Limpia 20L): Corte a 40 PSI / Rearme a 20 PSI
 * - Protección pozo seco: Contacto magnético NO al fondo del pozo
 * ============================================================================
 */

#include "config.h"
#include "sensors.h"
#include "power_mgmt.h"
#include "dtmf_handler.h"
#include "connectivity.h"

static unsigned long lastSerialLog = 0;
static bool lastDryAlertSent = false;
static bool lastT2FullAlertSent = false;
static bool lastOverheatAlertSent = false;

void setup() {
    Serial.begin(115200);
    delay(1200);

    Serial.println("\n========================================================");
    Serial.println("   🌊 AQUARESILIENCIA TIJUANA — NODO POZO PILOTO 001   ");
    Serial.println("      FIRMWARE MODULAR ESP32 v2.5 (RO / 120V / 24V)    ");
    Serial.println("========================================================");

    // 1. Inicializar sensores de campo y presostatos
    Serial.println("[BOOT] 1/4 Inicializando sensores y presostatos...");
    sensors.begin();

    // 2. Inicializar relés de potencia (Bomba 24V, Booster RO, Electroválvula 120V)
    Serial.println("[BOOT] 2/4 Inicializando relevadores en modo seguro (FAIL-SAFE)...");
    powerMgmt.begin();

    // 3. Inicializar decodificador DTMF telefónico
    Serial.println("[BOOT] 3/4 Inicializando decodificador de comandos DTMF...");
    dtmfHandler.begin();

    // 4. Inicializar conectividad dual (SoftAP Wi-Fi + Módem GSM SIM800L)
    Serial.println("[BOOT] 4/4 Inicializando conectividad dual (SoftAP + SIM800L)...");
    connectivity.begin();

    Serial.println("\n[SISTEMA LISTO] El nodo Pozo 001 está operando normalmente.");
    Serial.println(" - Red Wi-Fi: " WIFI_AP_SSID);
    Serial.println(" - Dashboard: http://192.168.4.1");
    Serial.println(" - O marca al número SIM para control remoto por teclado telefónico.");
    Serial.println("========================================================\n");
}

void loop() {
    // 1. Actualización continua de sensores (caudal, presostatos, contacto magnético, DHT22, DS18B20 agua)
    sensors.update();

    // 2. Máquina de estados hidráulica y enclavamientos (incluye protección térmica)
    SensorData s = sensors.getData();
    powerMgmt.update(
        s.wellLevelSafe,
        s.pressureT1Cutoff,
        s.pressureT1Demand,
        s.t2CleanWaterFull,
        s.t2CleanWaterDemand,
        s.waterTempOverheat
    );

    // 3. Atención de clientes web y tráfico serie del módem SIM800L
    connectivity.update();

    // 4. Telemetría periódica por consola serie USB cada 3 segundos
    unsigned long currentMillis = millis();
    if (currentMillis - lastSerialLog >= 3000) {
        lastSerialLog = currentMillis;

        Serial.print("[TELEMETRÍA] ");
        Serial.printf("Pozo: %s | ", s.wellLevelSafe ? "OK (0.80m)" : "¡SECO!");
        Serial.printf("T1 (60-90): %s | ", s.pressureT1Cutoff ? "CORTE 90 PSI" : "OK");
        Serial.printf("T2 (20-40): %s | ", s.t2CleanWaterFull ? "LLENO 40 PSI" : "LLENANDO");
        Serial.printf("Caudal: %.1f L/m | Vol: %.1f L | ", s.flowRateLPM, s.totalLiters);
        Serial.printf("Bomba 24V: %s | ", powerMgmt.isWellPumpRunning() ? "ON (30s)" : (powerMgmt.getWellPumpState() == PUMP_RESTING ? "PAUSA (60s)" : "OFF"));
        Serial.printf("Booster RO: %s | ", powerMgmt.isROBoosterRunning() ? "ON" : (powerMgmt.getROState() == RO_LOCKED_OVERHEAT ? "BLOQUEO TÉRMICO" : "OFF"));
        Serial.printf("Válvula 120V: %s | ", powerMgmt.isSolenoid120VOpen() ? "ABIERTA" : "CERRADA");
        Serial.printf("Agua RO: %.1f°C (%s) | Cabina: %.1f°C\n", s.waterTempC, s.waterTempOverheat ? "🔥 ALERTA" : "OK", s.temperatureC);
    }

    // 5. Alertas de seguridad automáticas por SMS
    // Alerta 1: Pozo Seco detectado por sensor magnético
    if (!s.wellLevelSafe && !lastDryAlertSent) {
        lastDryAlertSent = true;
        connectivity.sendSMS(EMERGENCY_PHONE_NUMBER, "ALERTA POZO 001: Sensor magnetico de fondo activado. Nivel critico de pozo seco. Bomba 24V apagada.");
    } else if (s.wellLevelSafe && lastDryAlertSent) {
        lastDryAlertSent = false; // Recuperación de nivel freático
    }

    // Alerta 2: Sobretemperatura de agua en entrada de Ósmosis Inversa (>32°C)
    if (s.waterTempOverheat && !lastOverheatAlertSent) {
        lastOverheatAlertSent = true;
        String msg = "ALERTA POZO 001: Sobretemperatura en agua RO (" + String(s.waterTempC, 1) + "C > 32C). Booster RO y valvula 120V apagadas por proteccion termica.";
        connectivity.sendSMS(EMERGENCY_PHONE_NUMBER, msg);
    } else if (!s.waterTempOverheat && lastOverheatAlertSent) {
        lastOverheatAlertSent = false;
        String msg = "NOTIFICACION POZO 001: Temperatura de agua normalizada (" + String(s.waterTempC, 1) + "C). Bloqueo termico de RO rearmado.";
        connectivity.sendSMS(EMERGENCY_PHONE_NUMBER, msg);
    }

    // Notificación opcional: Barril T2 Agua Limpia lleno (40 PSI)
    if (s.t2CleanWaterFull && !lastT2FullAlertSent) {
        lastT2FullAlertSent = true;
        connectivity.sendSMS(EMERGENCY_PHONE_NUMBER, "NOTIFICACION POZO 001: Barril T2 de agua purificada lleno (40 PSI). Llenado detenido.");
    } else if (!s.t2CleanWaterFull && lastT2FullAlertSent) {
        lastT2FullAlertSent = false;
    }
}
