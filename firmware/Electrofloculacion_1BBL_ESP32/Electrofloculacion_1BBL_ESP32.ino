/**
 * ============================================================================
 * PROYECTO: AquaResiliencia - "Agua de GIA" (Tijuana, B.C.)
 * FIRMWARE: Electrofloculación Adaptativa, Ley de Faraday y Filtración (1 BBL)
 * PLACA:    ESP32 (WROOM-32 / DevKit V1)
 * FECHA:    Noviembre 2026
 * ============================================================================
 * ARQUITECTURA FÍSICA Y ELECTROQUÍMICA (CERO QUÍMICOS EXTERNOS):
 * 1. Fuente 1: 12V / 11A para Ánodo de Fierro dulce y Cátodo (Carcasa/Acero).
 * 2. Fuente 2: 12V / 2A para Bomba de Trasvase a través de filtros y tótem.
 * 3. Llenado Adaptativo por Pulsos (Solenoide NC) controlando corriente (<9.5A).
 * 4. Sensor de Nivel Alto por Puntas de Cobre (Conductividad por puente de agua).
 * 5. Ley de Faraday en Tiempo Real: Estimación de TDS inicial (~3,000 ppm),
 *    cálculo de masa meta de Fe sacrificial y conteo de culombios segundo a segundo.
 * 6. Desgasificación por Bombita de Pecera (35 s cada 5 min) sin inversión de polaridad.
 * 7. Reposo Forzado por Algoritmo (45 min) con bloqueo físico de relés.
 * 8. Trasvase por Bomba 12V con sensor Hall a través de Malla 80, Sedimentos y Carbón.
 * 9. Servidor Web Integrado (AP + STA) con Dashboard en vivo y notificaciones 24/7.
 * ============================================================================
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// ================= 1. CONFIGURACIÓN DE RED =================
const char* WIFI_SSID     = "TU_WIFI";
const char* WIFI_PASSWORD = "TU_PASSWORD";
const char* AP_SSID       = "GIA_Planta_Agua_1BBL";
const char* AP_PASS       = "12345678";

WebServer server(80);

// ================= 2. MAPA DE PINES (ESP32) =================
#define PIN_ADC_VOLTAJE     34  // ADC1_CH6: Divisor de voltaje fuente 12V/11A (R1=33k, R2=10k)
#define PIN_NIVEL_ALTO      32  // Entrada Digital con PULLUP: Puntas de cobre nivel alto
#define PIN_SENSOR_HALL     13  // Interrupción: Caudalímetro Hall salida cono hacia filtros

#define PIN_RELAY_POWER     26  // Relé 1: Fuente 12V/11A (Electrodos Ánodo/Cátodo)
#define PIN_RELAY_PUMP_H2   27  // Relé 2: Bombita de pecera (barrido microburbujas H2)
#define PIN_RELAY_SV_LLENA  14  // Relé 3: Válvula Solenoide Llenado NC (SV-01)
#define PIN_RELAY_BOMBA_TR  12  // Relé 4: Bomba de trasvase 12V/2A a filtros y tótem

#define RELE_ON             LOW
#define RELE_OFF            HIGH

// ================= 3. PARÁMETROS ELECTROQUÍMICOS Y PROTECCIÓN =================
const float VOLUMEN_LOTE_L          = 117.3f; // 1 BBL
const float FACTOR_DIVISOR_V        = 4.30f;  // R1=33k, R2=10k

// Protección de Fuente 11A
const float I_MAX_LLENADO_A         = 9.50f;  // Pausa temporalmente solenoide para evitar sobrecorriente
const float I_REANUDAR_LLENADO_A    = 8.50f;  // Reanuda pulsos de llenado
const float I_CORTE_CRITICO_A       = 10.50f; // Disparo de paro si se supera por > 3 seg
const float V_CORTO_CRITICO_V       = 9.80f;  // Alerta de corto franco entre electrodos

// Ley de Faraday y Dosificación de Fierro
const float PESO_ATOMICO_FE         = 55.845f;
const float VALENCIA_FE             = 2.0f;   // Fe -> Fe2+ + 2e-
const float CONSTANTE_FARADAY       = 96485.0f;
const float DOSIS_BASE_MG_L         = 50.0f;  // 50 mg/L base para agua de 3,000 ppm
const float PPM_REFERENCIA          = 3000.0f;
const float CONSTANTE_CALIBRACION_G = 3811.7f;// Factor de conductancia a PPM

// Tiempos de Proceso
const uint32_t INTERVALO_PECERA_MS  = 300000; // 5 minutos entre barridos
const uint32_t DURACION_PECERA_MS   = 35000;  // 35 segundos de barrido
const uint32_t TIEMPO_REPOSO_MS     = 2700000;// 45 minutos de reposo estricto

// ================= 4. VARIABLES DE ESTADO Y SENSORES =================
enum EstadoSistema {
  ESTADO_ESPERA = 0,            // Standby, midiendo V_vacio de fuente 11A
  ESTADO_LLENADO_ADAPTATIVO = 1,// Pulsos de solenoide vigilando amperaje hasta nivel alto
  ESTADO_CALCULO_FARADAY = 2,   // Estimación de TDS y cálculo estequiométrico de Fe meta
  ESTADO_FLOCULANDO = 3,        // Electrólisis activa con odómetro de culombios
  ESTADO_BARRIDO_H2 = 4,        // Bombita de pecera activa desgasificando electrodos
  ESTADO_REPOSO_FORZADO = 5,    // Bloqueo total: decantación de flóculos en cono
  ESTADO_TRASVASE_FILTRADO = 6, // Bomba 12V activa hacia filtros Malla 80 -> Sedimentos -> Carbón
  ESTADO_LOTE_COMPLETADO = 7,   // Proceso finalizado con éxito
  ESTADO_ALERTA_PROTECCION = 8  // Paro por sobrecorriente o falla de hardware
};

EstadoSistema estadoActual = ESTADO_ESPERA;

// Variables eléctricas
float vVacio              = 11.60f;
float vCargaActual        = 11.60f;
float deltaVActual        = 0.00f;
float corrienteActualA    = 0.0f;
uint8_t contadorSegSobreI = 0;

// Variables Faraday y Química
float tdsEstimadoPPM      = 3000.0f;
float dosisFeMgL          = 50.0f;
float gramosFeMeta        = 5.865f;
float gramosFeDisueltos   = 0.0f;
float pctFaraday          = 0.0f;

// Variables de Nivel y Solenoide
bool nivelAltoDetectado   = false;
bool solenoidePausadaI    = false;
bool pulsoSolenoideAbierto= false;
unsigned long tUltimoCambioSV = 0;

// Variables de Bombita de Pecera
unsigned long tUltimoBarridoPecera = 0;
uint16_t totalBarridosRealizados   = 0;

// Variables de Reposo y Tiempos
unsigned long tInicioEtapa         = 0;
unsigned long tUltimo1Hz           = 0;

// Variables Sensor Hall y Trasvase
volatile unsigned long pulsosHall = 0;
float caudalLPM                   = 0.0f;
float litrosTrasvasados           = 0.0f;
unsigned long tUltimoPulsoHall    = 0;
uint8_t segSinFlujoEnTrasvase     = 0;

// Notificaciones dinámicas al usuario
String zonaSeguridad   = "VERDE";
String mensajeUsuario  = "Sistema en espera. Fuente 12V/11A en vacío lista. Presione 'Iniciar Lote'.";

// Rutina de Interrupción para Caudalímetro Hall
void IRAM_ATTR ISR_contarPulsosHall() {
  pulsosHall++;
  tUltimoPulsoHall = millis();
}

// ================= 5. FUNCIONES DE HARDWARE =================
void apagarTodosLosReles() {
  digitalWrite(PIN_RELAY_POWER, RELE_OFF);
  digitalWrite(PIN_RELAY_PUMP_H2, RELE_OFF);
  digitalWrite(PIN_RELAY_SV_LLENA, RELE_OFF);
  digitalWrite(PIN_RELAY_BOMBA_TR, RELE_OFF);
}

float leerVoltajeFuente11A() {
  uint32_t sumaMV = 0;
  for (int i = 0; i < 32; i++) {
    sumaMV += analogReadMilliVolts(PIN_ADC_VOLTAJE);
    delayMicroseconds(100);
  }
  return ((sumaMV / 32.0f) / 1000.0f) * FACTOR_DIVISOR_V;
}

bool leerNivelAltoPuntasCobre() {
  // Puntas de cobre con INPUT_PULLUP:
  // En seco (aire): pin lee HIGH (1).
  // Con agua tocando ambas puntas: puente a tierra -> pin lee LOW (0).
  return (digitalRead(PIN_NIVEL_ALTO) == LOW);
}

String obtenerNombreEstado() {
  switch (estadoActual) {
    case ESTADO_ESPERA:            return "STANDBY / EN ESPERA";
    case ESTADO_LLENADO_ADAPTATIVO:return "1. LLENADO ADAPTATIVO (CONTROL AMPERAJE)";
    case ESTADO_CALCULO_FARADAY:   return "2. CÁLCULO QUÍMICO FARADAY";
    case ESTADO_FLOCULANDO:        return "3. ELECTROFLOCULACIÓN ACTIVA (FARADAY)";
    case ESTADO_BARRIDO_H2:        return "3B. BARRIDO MICROBURBUJAS H2 (BOMBA PECERA)";
    case ESTADO_REPOSO_FORZADO:    return "4. REPOSO FORZADO (DECANTACIÓN LOBOS)";
    case ESTADO_TRASVASE_FILTRADO: return "5. TRASVASE Y FILTRADO (MALLA 80 + CTO)";
    case ESTADO_LOTE_COMPLETADO:   return "6. LOTE COMPLETADO CON ÉXITO";
    case ESTADO_ALERTA_PROTECCION: return "ALERTA DE SEGURIDAD / PARO";
    default:                       return "DESCONOCIDO";
  }
}

// ================= 6. MOTOR DE REGLAS 1 HZ =================
void ejecutarCiclo1Hz() {
  unsigned long ahora = millis();
  if (ahora - tUltimo1Hz < 1000) return;
  tUltimo1Hz = ahora;

  vCargaActual = leerVoltajeFuente11A();
  nivelAltoDetectado = leerNivelAltoPuntasCobre();

  // Cálculo de corriente en estados activos
  if (estadoActual == ESTADO_LLENADO_ADAPTATIVO || estadoActual == ESTADO_FLOCULANDO || estadoActual == ESTADO_BARRIDO_H2) {
    deltaVActual = max(0.0f, vVacio - vCargaActual);
    // Estimación empírica de corriente basada en la caída interna de la fuente de 11A
    corrienteActualA = constrain(deltaVActual * 9.8f, 0.0f, 13.0f);
  } else {
    deltaVActual = 0.0f;
    corrienteActualA = 0.0f;
  }

  // Protección térmica y sobrecorriente para Fuente de 11A
  if (corrienteActualA >= I_CORTE_CRITICO_A || (vCargaActual < V_CORTO_CRITICO_V && estadoActual == ESTADO_FLOCULANDO)) {
    contadorSegSobreI++;
    zonaSeguridad = "ROJA";
    if (contadorSegSobreI >= 3) {
      apagarTodosLosReles();
      estadoActual = ESTADO_ALERTA_PROTECCION;
      mensajeUsuario = "PARO DE EMERGENCIA: Corriente excesiva (" + String(corrienteActualA, 1) + " A) o corto detectado. Fuente 11A protegida.";
      return;
    }
  } else if (corrienteActualA >= I_MAX_LLENADO_A) {
    contadorSegSobreI = 0;
    zonaSeguridad = "AMARILLA";
  } else {
    contadorSegSobreI = 0;
    zonaSeguridad = "VERDE";
  }

  // --- MÁQUINA DE ESTADOS ---
  switch (estadoActual) {

    // --- 0. ESPERA ---
    case ESTADO_ESPERA:
      vVacio = vCargaActual;
      zonaSeguridad = "VERDE";
      mensajeUsuario = "Sistema en espera. V_vacío: " + String(vVacio, 2) + "V. Nivel alto: " + (nivelAltoDetectado ? "MOJADO" : "SECO");
      break;

    // --- 1. LLENADO ADAPTATIVO ---
    case ESTADO_LLENADO_ADAPTATIVO:
      // Si las puntas de cobre detectan que el agua llegó al tope:
      if (nivelAltoDetectado) {
        digitalWrite(PIN_RELAY_SV_LLENA, RELE_OFF); // Cierra solenoide inmediatamente
        estadoActual = ESTADO_CALCULO_FARADAY;
        tInicioEtapa = ahora;
        mensajeUsuario = "¡Nivel alto alcanzado! Solenoide cerrada. Calculando dosificación química de Faraday...";
        break;
      }

      // Regulación de corriente durante el llenado para no sobrecargar la fuente
      if (corrienteActualA >= I_MAX_LLENADO_A) {
        solenoidePausadaI = true;
        digitalWrite(PIN_RELAY_SV_LLENA, RELE_OFF);
        mensajeUsuario = "Pausa preventiva: Amperaje alto (" + String(corrienteActualA, 1) + " A). Esperando estabilización...";
      } else if (solenoidePausadaI && corrienteActualA <= I_REANUDAR_LLENADO_A) {
        solenoidePausadaI = false;
      }

      // Pulsos de solenoide (3 seg ON, 2 seg OFF) si no está pausada
      if (!solenoidePausadaI) {
        if (pulsoSolenoideAbierto && (ahora - tUltimoCambioSV >= 3000)) {
          digitalWrite(PIN_RELAY_SV_LLENA, RELE_OFF);
          pulsoSolenoideAbierto = false;
          tUltimoCambioSV = ahora;
        } else if (!pulsoSolenoideAbierto && (ahora - tUltimoCambioSV >= 2000)) {
          digitalWrite(PIN_RELAY_SV_LLENA, RELE_ON);
          pulsoSolenoideAbierto = true;
          tUltimoCambioSV = ahora;
        }
        mensajeUsuario = "Llenando adaptativo... Nivel subiendo. Corriente: " + String(corrienteActualA, 1) + " A / 11 A max.";
      }
      break;

    // --- 2. CÁLCULO QUÍMICO DE FARADAY ---
    case ESTADO_CALCULO_FARADAY:
      if (vCargaActual > 5.0f && corrienteActualA > 1.0f) {
        float conductancia = corrienteActualA / vCargaActual;
        tdsEstimadoPPM = constrain(conductancia * CONSTANTE_CALIBRACION_G, 1500.0f, 5000.0f);
      } else {
        tdsEstimadoPPM = PPM_REFERENCIA;
      }
      dosisFeMgL = DOSIS_BASE_MG_L * (tdsEstimadoPPM / PPM_REFERENCIA);
      gramosFeMeta = (dosisFeMgL * VOLUMEN_LOTE_L) / 1000.0f;
      gramosFeDisueltos = 0.0f;
      pctFaraday = 0.0f;

      estadoActual = ESTADO_FLOCULANDO;
      tInicioEtapa = ahora;
      tUltimoBarridoPecera = ahora;
      totalBarridosRealizados = 0;
      mensajeUsuario = "TDS estimado: " + String(tdsEstimadoPPM, 0) + " ppm. Meta Faraday: Sacrificar " + String(gramosFeMeta, 2) + " g de Fe dulce.";
      break;

    // --- 3. FLOCULANDO (CONTEO DE CULOMBIOS) ---
    case ESTADO_FLOCULANDO:
    case ESTADO_BARRIDO_H2: {
      // Integración segundo a segundo: dM = (I * dt * M) / (z * F)
      float deltaGramoFe = (corrienteActualA * 1.0f * PESO_ATOMICO_FE) / (VALENCIA_FE * CONSTANTE_FARADAY);
      gramosFeDisueltos += deltaGramoFe;
      pctFaraday = constrain((gramosFeDisueltos / gramosFeMeta) * 100.0f, 0.0f, 100.0f);

      // Control del Barrido con Bombita de Pecera (cada 5 min por 35 s)
      if (estadoActual == ESTADO_FLOCULANDO && (ahora - tUltimoBarridoPecera >= INTERVALO_PECERA_MS)) {
        digitalWrite(PIN_RELAY_PUMP_H2, RELE_ON);
        estadoActual = ESTADO_BARRIDO_H2;
        tUltimoBarridoPecera = ahora;
        totalBarridosRealizados++;
        mensajeUsuario = "Barrido #" + String(totalBarridosRealizados) + ": Bombita de pecera activa (35s) barriendo microburbujas H2...";
      } else if (estadoActual == ESTADO_BARRIDO_H2 && (ahora - tUltimoBarridoPecera >= DURACION_PECERA_MS)) {
        digitalWrite(PIN_RELAY_PUMP_H2, RELE_OFF);
        estadoActual = ESTADO_FLOCULANDO;
        tUltimoBarridoPecera = ahora;
        mensajeUsuario = "Barrido #" + String(totalBarridosRealizados) + " terminado. Nata intacta. Floculación continua.";
      }

      if (estadoActual == ESTADO_FLOCULANDO) {
        mensajeUsuario = "Floculando: " + String(gramosFeDisueltos, 2) + " / " + String(gramosFeMeta, 2) + " g Fe (" + String(pctFaraday, 1) + "%). I=" + String(corrienteActualA, 1) + "A.";
      }

      // Verificación de fin de electrofloculación por Ley de Faraday
      if (gramosFeDisueltos >= gramosFeMeta) {
        apagarTodosLosReles();
        estadoActual = ESTADO_REPOSO_FORZADO;
        tInicioEtapa = ahora;
        mensajeUsuario = "¡Meta química de Faraday alcanzada (" + String(gramosFeDisueltos, 2) + " g Fe)! Iniciando 45 min de REPOSO FORZADO.";
      }
      break;
    }

    // --- 4. REPOSO FORZADO ---
    case ESTADO_REPOSO_FORZADO: {
      apagarTodosLosReles(); // Bloqueo físico absoluto
      uint32_t tiempoTranscurrido = ahora - tInicioEtapa;
      if (tiempoTranscurrido >= TIEMPO_REPOSO_MS) {
        // Fin de reposo -> Iniciar trasvase
        digitalWrite(PIN_RELAY_BOMBA_TR, RELE_ON); // Enciende bomba 12V con su fuente de 2A
        pulsosHall = 0;
        litrosTrasvasados = 0.0f;
        segSinFlujoEnTrasvase = 0;
        estadoActual = ESTADO_TRASVASE_FILTRADO;
        tInicioEtapa = ahora;
        mensajeUsuario = "Reposo de 45 min concluido. Bomba de trasvase activa: Malla 80 Inox -> Sedimentos -> Carbón -> Tótem.";
      } else {
        uint32_t segRestantes = (TIEMPO_REPOSO_MS - tiempoTranscurrido) / 1000UL;
        uint32_t minR = segRestantes / 60;
        uint32_t segR = segRestantes % 60;
        mensajeUsuario = "REPOSO FORZADO ACTIVO: " + String(minR) + "m " + String(segR) + "s restantes. No mover el agua mientras sedimenta.";
      }
      break;
    }

    // --- 5. TRASVASE Y FILTRADO ---
    case ESTADO_TRASVASE_FILTRADO: {
      // Cálculo de flujo con sensor Hall (Aprox 7.5 pulsos por segundo = 1 LPM)
      noInterrupts();
      unsigned long pulsos = pulsosHall;
      pulsosHall = 0;
      interrupts();

      caudalLPM = (pulsos / 7.5f) / 1.0f; // Flujo en 1 segundo
      litrosTrasvasados += (caudalLPM / 60.0f);

      if (caudalLPM < 0.2f) {
        segSinFlujoEnTrasvase++;
      } else {
        segSinFlujoEnTrasvase = 0;
      }

      mensajeUsuario = "Trasvasando: Caudal " + String(caudalLPM, 1) + " LPM | Total trasvasado: " + String(litrosTrasvasados, 1) + " L hacia Tótem.";

      // Si el caudal cae a cero por > 10 segundos y ya trasvasó la mayor parte del barril:
      if (segSinFlujoEnTrasvase >= 10 && litrosTrasvasados >= 95.0f) {
        apagarTodosLosReles();
        estadoActual = ESTADO_LOTE_COMPLETADO;
        mensajeUsuario = "¡LOTE FINALIZADO CON ÉXITO! " + String(litrosTrasvasados, 1) + " Litros filtrados y depositados en el Tótem.";
      }
      break;
    }

    // --- 6. LOTE COMPLETADO ---
    case ESTADO_LOTE_COMPLETADO:
      apagarTodosLosReles();
      zonaSeguridad = "VERDE";
      break;

    // --- ALERTA ---
    case ESTADO_ALERTA_PROTECCION:
      apagarTodosLosReles();
      zonaSeguridad = "ROJA";
      break;
  }
}

// ================= 7. INTERFAZ WEB EMBEBIDA EN FLASH (HTML/CSS/JS) =================
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>AquaResiliencia | Control 1 BBL Electrofloculación</title>
    <style>
        :root {
            --bg: #090d16;
            --card: rgba(16, 24, 39, 0.85);
            --border: rgba(56, 189, 248, 0.2);
            --cyan: #00f0ff;
            --green: #10b981;
            --yellow: #f59e0b;
            --red: #ef4444;
            --text: #f8fafc;
            --text-dim: #94a3b8;
        }
        * { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; }
        body { background: var(--bg); color: var(--text); padding: 14px; min-height: 100vh; }
        .wrapper { max-width: 900px; margin: 0 auto; display: flex; flex-direction: column; gap: 14px; }
        
        header {
            background: var(--card);
            border: 1px solid var(--border);
            border-radius: 16px;
            padding: 14px 18px;
            display: flex;
            justify-content: space-between;
            align-items: center;
            flex-wrap: wrap;
            gap: 10px;
        }
        .brand h1 { font-size: 18px; font-weight: 800; letter-spacing: 0.5px; }
        .brand span { color: var(--cyan); }
        .brand p { font-size: 11px; color: var(--text-dim); }
        
        .pill {
            padding: 5px 12px;
            border-radius: 20px;
            font-size: 12px;
            font-weight: 700;
            display: flex;
            align-items: center;
            gap: 6px;
            border: 1px solid rgba(255,255,255,0.1);
            background: rgba(0,0,0,0.4);
        }
        .pill-dot { width: 8px; height: 8px; border-radius: 50%; }
        .VERDE .pill-dot { background: var(--green); box-shadow: 0 0 8px var(--green); }
        .AMARILLA .pill-dot { background: var(--yellow); box-shadow: 0 0 8px var(--yellow); }
        .ROJA .pill-dot { background: var(--red); box-shadow: 0 0 10px var(--red); }
        
        /* BANNER HERO DE ACCIÓN EN VIVO */
        .hero-banner {
            background: linear-gradient(135deg, rgba(2,132,199,0.2), rgba(15,23,42,0.9));
            border: 1px solid var(--cyan);
            border-radius: 16px;
            padding: 18px;
            text-align: center;
            box-shadow: 0 8px 32px rgba(0,240,255,0.15);
        }
        .hero-stage { font-size: 13px; text-transform: uppercase; letter-spacing: 1.5px; color: var(--cyan); font-weight: 700; }
        .hero-desc { font-size: 17px; font-weight: 600; margin: 8px 0; color: #fff; line-height: 1.4; }
        
        /* GRID DE MÉTRICAS */
        .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(180px, 1fr)); gap: 12px; }
        .card {
            background: var(--card);
            border: 1px solid var(--border);
            border-radius: 14px;
            padding: 14px;
            display: flex;
            flex-direction: column;
            gap: 6px;
        }
        .card-label { font-size: 11px; color: var(--text-dim); text-transform: uppercase; font-weight: 600; }
        .card-value { font-size: 24px; font-weight: 800; font-variant-numeric: tabular-nums; }
        .card-sub { font-size: 11px; color: var(--text-dim); }
        
        /* BARRA DE PROGRESO DE FARADAY */
        .progress-box {
            background: var(--card);
            border: 1px solid var(--border);
            border-radius: 14px;
            padding: 16px;
            display: flex;
            flex-direction: column;
            gap: 10px;
        }
        .p-bar-outer { width: 100%; height: 16px; background: rgba(255,255,255,0.06); border-radius: 8px; overflow: hidden; }
        .p-bar-inner { width: 0%; height: 100%; background: linear-gradient(90deg, #0284c7, var(--cyan)); transition: width 0.4s ease; }
        
        /* BOTONES DE CONTROL */
        .actions { display: grid; grid-template-columns: repeat(auto-fit, minmax(140px, 1fr)); gap: 10px; }
        button {
            padding: 14px;
            border-radius: 12px;
            border: none;
            font-size: 13px;
            font-weight: 700;
            cursor: pointer;
            transition: all 0.2s ease;
            text-transform: uppercase;
        }
        .btn-start { background: linear-gradient(135deg, #059669, #10b981); color: #fff; }
        .btn-reposo { background: linear-gradient(135deg, #d97706, #f59e0b); color: #fff; }
        .btn-trasvase { background: linear-gradient(135deg, #0284c7, #38bdf8); color: #fff; }
        .btn-stop { background: linear-gradient(135deg, #dc2626, #ef4444); color: #fff; }
        button:active { transform: scale(0.97); }
    </style>
</head>
<body>
    <div class="wrapper">
        <header>
            <div class="brand">
                <h1>AGUA DE GIA <span>1 BBL</span></h1>
                <p>Electrofloculación Adaptativa & Faraday</p>
            </div>
            <div id="statusPill" class="pill VERDE">
                <div class="pill-dot"></div>
                <span id="pillText">NORMAL</span>
            </div>
        </header>

        <div class="hero-banner">
            <div id="heroStage" class="hero-stage">INICIALIZANDO...</div>
            <div id="heroDesc" class="hero-desc">Conectando con ESP32...</div>
        </div>

        <div class="progress-box">
            <div style="display:flex; justify-content:space-between; align-items:center;">
                <span class="card-label">Odómetro de Fierro (Ley de Faraday)</span>
                <span id="faradayPct" style="font-weight:800; color:var(--cyan);">0.0%</span>
            </div>
            <div class="p-bar-outer">
                <div id="faradayBar" class="p-bar-inner"></div>
            </div>
            <div style="display:flex; justify-content:space-between; font-size:12px; color:var(--text-dim);">
                <span>Disuelto: <b id="feDisuelto" style="color:#fff;">0.00</b> g Fe</span>
                <span>Meta: <b id="feMeta" style="color:#fff;">0.00</b> g Fe</span>
                <span>TDS Estimado: <b id="tdsEst" style="color:#fff;">3,000</b> ppm</span>
            </div>
        </div>

        <div class="grid">
            <div class="card">
                <span class="card-label">Corriente (Fuente 11A)</span>
                <span id="valAmps" class="card-value" style="color:#38bdf8;">0.0 A</span>
                <span class="card-sub">Límite Seguro: 9.5 A</span>
            </div>
            <div class="card">
                <span class="card-label">Voltaje Electrofloculación</span>
                <span id="valVoltaje" class="card-value">11.6 V</span>
                <span id="valDeltaV" class="card-sub">ΔV: 0.00 V</span>
            </div>
            <div class="card">
                <span class="card-label">Nivel Alto (Puntas Cobre)</span>
                <span id="valNivel" class="card-value" style="color:var(--yellow);">SECO</span>
                <span class="card-sub">Puente de agua superior</span>
            </div>
            <div class="card">
                <span class="card-label">Caudalímetro Hall (Salida)</span>
                <span id="valCaudal" class="card-value" style="color:var(--green);">0.0 LPM</span>
                <span id="valLitros" class="card-sub">Total: 0.0 L trasvasados</span>
            </div>
        </div>

        <div class="actions">
            <button class="btn-start" onclick="enviarComando('iniciar_lote')">▶ Iniciar Lote</button>
            <button class="btn-reposo" onclick="enviarComando('forzar_reposo')">⏸ Forzar Reposo</button>
            <button class="btn-trasvase" onclick="enviarComando('forzar_trasvase')">💧 Forzar Trasvase</button>
            <button class="btn-stop" onclick="enviarComando('paro')">⏹ PARO TOTAL</button>
        </div>
    </div>

    <script>
        function actualizarTelemetria() {
            fetch('/status')
                .then(r => r.json())
                .then(d => {
                    document.getElementById('heroStage').innerText = d.estado;
                    document.getElementById('heroDesc').innerText = d.mensaje;
                    
                    document.getElementById('valAmps').innerText = d.amps.toFixed(1) + ' A';
                    document.getElementById('valVoltaje').innerText = d.voltaje.toFixed(2) + ' V';
                    document.getElementById('valDeltaV').innerText = 'ΔV: ' + d.deltaV.toFixed(2) + ' V (V_vacío: ' + d.vVacio.toFixed(2) + 'V)';
                    
                    document.getElementById('valNivel').innerText = d.nivelAlto ? 'TOPE LLENO' : 'SECO';
                    document.getElementById('valNivel').style.color = d.nivelAlto ? 'var(--cyan)' : 'var(--yellow)';
                    
                    document.getElementById('valCaudal').innerText = d.caudalLPM.toFixed(1) + ' LPM';
                    document.getElementById('valLitros').innerText = 'Total: ' + d.litros.toFixed(1) + ' L hacia Tótem';
                    
                    document.getElementById('faradayPct').innerText = d.pctFaraday.toFixed(1) + '%';
                    document.getElementById('faradayBar').style.width = d.pctFaraday.toFixed(1) + '%';
                    document.getElementById('feDisuelto').innerText = d.feDisuelto.toFixed(2);
                    document.getElementById('feMeta').innerText = d.feMeta.toFixed(2);
                    document.getElementById('tdsEst').innerText = Math.round(d.tdsPPM);
                    
                    const pill = document.getElementById('statusPill');
                    pill.className = 'pill ' + d.zona;
                    document.getElementById('pillText').innerText = d.zona;
                })
                .catch(err => console.error(err));
        }

        function enviarComando(cmd) {
            fetch('/cmd?accion=' + cmd)
                .then(r => r.json())
                .then(() => actualizarTelemetria())
                .catch(err => console.error(err));
        }

        setInterval(actualizarTelemetria, 1000);
        actualizarTelemetria();
    </script>
</body>
</html>
)rawliteral";

// ================= 8. RUTAS WEB Y JSON =================
void handleRoot() {
  server.send(200, "text/html", INDEX_HTML);
}

void handleStatusJSON() {
  String json = "{";
  json += "\"etapaIdx\":" + String((int)estadoActual) + ",";
  json += "\"estado\":\"" + obtenerNombreEstado() + "\",";
  json += "\"zona\":\"" + zonaSeguridad + "\",";
  json += "\"mensaje\":\"" + mensajeUsuario + "\",";
  json += "\"voltaje\":" + String(vCargaActual, 2) + ",";
  json += "\"vVacio\":" + String(vVacio, 2) + ",";
  json += "\"deltaV\":" + String(deltaVActual, 2) + ",";
  json += "\"amps\":" + String(corrienteActualA, 1) + ",";
  json += "\"nivelAlto\":" + String(nivelAltoDetectado ? "true" : "false") + ",";
  json += "\"feDisuelto\":" + String(gramosFeDisueltos, 3) + ",";
  json += "\"feMeta\":" + String(gramosFeMeta, 3) + ",";
  json += "\"pctFaraday\":" + String(pctFaraday, 1) + ",";
  json += "\"tdsPPM\":" + String(tdsEstimadoPPM, 0) + ",";
  json += "\"caudalLPM\":" + String(caudalLPM, 1) + ",";
  json += "\"litros\":" + String(litrosTrasvasados, 1) + ",";
  json += "\"barridosH2\":" + String(totalBarridosRealizados);
  json += "}";
  server.send(200, "application/json", json);
}

void handleCmd() {
  String a = server.arg("accion");
  unsigned long ahora = millis();

  if (a == "iniciar_lote") {
    apagarTodosLosReles();
    vVacio = leerVoltajeFuente11A();
    digitalWrite(PIN_RELAY_POWER, RELE_ON); // Energiza electrodos
    estadoActual = ESTADO_LLENADO_ADAPTATIVO;
    tInicioEtapa = ahora;
    tUltimoCambioSV = ahora;
    pulsoSolenoideAbierto = false;
    solenoidePausadaI = false;
    mensajeUsuario = "Iniciando lote: Electrodos energizados. Abriendo solenoide por pulsos vigilando amperaje...";
  } else if (a == "forzar_reposo") {
    apagarTodosLosReles();
    estadoActual = ESTADO_REPOSO_FORZADO;
    tInicioEtapa = ahora;
    mensajeUsuario = "Reposo forzado activado manualmente (45 min de bloqueo para decantación).";
  } else if (a == "forzar_trasvase") {
    apagarTodosLosReles();
    digitalWrite(PIN_RELAY_BOMBA_TR, RELE_ON);
    estadoActual = ESTADO_TRASVASE_FILTRADO;
    tInicioEtapa = ahora;
    pulsosHall = 0;
    litrosTrasvasados = 0.0f;
    segSinFlujoEnTrasvase = 0;
    mensajeUsuario = "Trasvase forzado manualmente: Bomba activa hacia tren de filtros Malla 80 -> Sedimentos -> Carbón.";
  } else if (a == "paro") {
    apagarTodosLosReles();
    estadoActual = ESTADO_ESPERA;
    zonaSeguridad = "VERDE";
    mensajeUsuario = "Paro general ejecutado. Todos los relés y fuentes apagados.";
  }

  handleStatusJSON();
}

// ================= 9. SETUP & LOOP =================
void setup() {
  Serial.begin(115200);
  delay(500);

  // Configuración de Pines
  pinMode(PIN_RELAY_POWER, OUTPUT);
  pinMode(PIN_RELAY_PUMP_H2, OUTPUT);
  pinMode(PIN_RELAY_SV_LLENA, OUTPUT);
  pinMode(PIN_RELAY_BOMBA_TR, OUTPUT);
  apagarTodosLosReles();

  pinMode(PIN_NIVEL_ALTO, INPUT_PULLUP);
  pinMode(PIN_SENSOR_HALL, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_SENSOR_HALL), ISR_contarPulsosHall, FALLING);

  analogReadResolution(12);
  analogSetPinAttenuation(PIN_ADC_VOLTAJE, ADC_11db);

  vVacio = leerVoltajeFuente11A();

  // Configuración Wi-Fi (Modo Dual: Punto de Acceso + Red Local)
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(AP_SSID, AP_PASS);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("==========================================");
  Serial.println("AquaResiliencia: Agua de GIA - Planta 1 BBL");
  Serial.println("AP Wi-Fi SSID: " + String(AP_SSID));
  Serial.println("IP Access Point: " + WiFi.softAPIP().toString());
  Serial.println("==========================================");

  // Servidor Web
  server.on("/", handleRoot);
  server.on("/status", handleStatusJSON);
  server.on("/cmd", handleCmd);
  server.begin();
}

void loop() {
  server.handleClient();
  ejecutarCiclo1Hz();
}
