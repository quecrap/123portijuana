#include "washing_cycle_fsm.h"
#include "hal_electric.h"

static WasherCycleStatus s_status;

// Timers internos no bloqueantes (millis) de la máquina de estados
static uint32_t s_stageStartMs              = 0;
static uint32_t s_lastFaradayTickMs         = 0;
static uint32_t s_lastSecondTickMs          = 0;
static uint32_t s_anodeTouchStartMs         = 0;
static uint32_t s_zeroFlowStartMs           = 0;
static bool     s_autoMagicCalculated       = false;

// Temporizadores del muestreo periódico del Ánodo Pasivo (cada 60s -> pausa de 1.5s en P33)
static uint32_t s_electrolysisRunAccMs      = 0; // Milisegundos acumulados de electrólisis desde el último muestreo
static uint32_t s_passivePauseStartMs       = 0; // Timestamp de inicio de la pausa de 1.5s (0 = sin pausa)
static bool     s_passiveSampleTakenInPause = false;

// ============================================================================
// 1. TABLAS DE TEXTOS RETRO DE LAS 7 POSICIONES DE LA PERILLA
// ============================================================================
static const char* getStageTitleByPos(WasherKnobPos pos) {
  switch (pos) {
    case POS_1_APAGADO:           return "APAGADO / LISTA PARA LAVAR";
    case POS_2_LLENANDO_TINA:     return "LLENANDO TINA (CONO 1 BBL · 117 L)";
    case POS_3_TALLADO_JABON:     return "ELECTROFLOCULACIÓN POR CULOMBIOS (7.60 g Fe)";
    case POS_4_REMOJO_CALMA:      return "REMOJO EN CALMA (DECANTACIÓN 60°)";
    case POS_5_ENJUAGUE_FILTROS:  return "ENJUAGUE TRIPLE FILTRO (TRASVASE P5)";
    case POS_6_EXPRIMIDO_CASA_60: return "EXPRIMIDO CASA (60 PSI · P15 + P23)";
    case POS_7_SECADO_SOL_RO_100: return "ÓSMOSIS INVERSA 100 PSI (P25 + P13 + MG996R)";
    default:                      return "APAGADO / LISTA PARA LAVAR";
  }
}

static const char* getStageSubtitleByPos(WasherKnobPos pos) {
  switch (pos) {
    case POS_1_APAGADO:
      return "En espera. Selecciona Tamaño de Carga, Dosis y gira la perilla.";
    case POS_2_LLENANDO_TINA:
      return "SSR Fotek #2 (P27) llenando cono. Bomba Pecera (P26) protegida hasta alcanzar el último tercio (>=66.7%).";
    case POS_3_TALLADO_JABON:
      return "SSR Fotek #1 (P33 ATX 15A) + Whirlpool (P26). Integrando Culombios hasta 7.60g Fe con muestreo VP (P36) cada 60s.";
    case POS_4_REMOJO_CALMA:
      return "Todo apagado en silencio. El lodo floculado precipita al fondo del cono de 60°.";
    case POS_5_ENJUAGUE_FILTROS:
      return "Bixolon #2 M1 (P5 Relé 30A / Bomba Trasvase) enviando agua clara por Malla 80, 50/5 micras y Carbón.";
    case POS_6_EXPRIMIDO_CASA_60:
      return "Presurizando línea de Casa (Relé #2 P15 + Bomba Diafragma #2 Manifold P23).";
    case POS_7_SECADO_SOL_RO_100:
      return "Flushing RO 15s (Bixolon #1 M2 P13) y producción RO (Pre-RO P25 + Servo MG996R P16 a 100 PSI).";
    default:
      return "";
  }
}

// ============================================================================
// 2. RECÁLCULO DE VOLUMEN Y META DE GRAMOS DE HIERRO (LEY DE FARADAY)
// ============================================================================
static void recalculateLoadAndSoapTargets() {
  uint8_t loadIdx = (uint8_t)s_status.loadSize;
  if (loadIdx > 2) loadIdx = 2;

  s_status.targetVolumeLiters = LOAD_VOLUME_LITERS[loadIdx];
  s_status.targetLevelPct = constrain((s_status.targetVolumeLiters / VOLUMEN_NOMINAL_1BBL_L) * 95.0f, 30.0f, 95.0f);

  float volumeScale = s_status.targetVolumeLiters / VOLUMEN_NOMINAL_1BBL_L;

  switch (s_status.soapType) {
    case SOAP_DELICATE_2G5:
      s_status.estimatedWaterPpm = 1280.0f;
      s_status.targetFeGrams = 3.80f * volumeScale;
      break;

    case SOAP_NORMAL_4G0:
      s_status.estimatedWaterPpm = 2050.0f;
      s_status.targetFeGrams = 5.70f * volumeScale;
      break;

    case SOAP_HEAVY_5G86:
    case SOAP_AUTO_MAGIC:
    default:
      // Regla oficial Nodo 2: 7.60 gramos exactos de Hierro disuelto para 117.3 Litros (~29-35 min a 15A)
      s_status.estimatedWaterPpm = 2400.0f;
      s_status.targetFeGrams = META_FE_GRAMOS_117L * (s_status.loadSize == LOAD_FULL_117L ? 1.0f : volumeScale);
      break;
  }

  if (s_status.targetFeGrams < 0.1f) {
    s_status.targetFeGrams = META_FE_GRAMOS_117L;
  }
  s_status.faradayProgressPct = constrain((s_status.dissolvedFeGrams / s_status.targetFeGrams) * 100.0f, 0.0f, 100.0f);
}

// ============================================================================
// 3. CONTROL AUTÓNOMO DEL TÓTEM SUPERIOR 1 (GPIO 19 -> BOMBA EVANS GPIO 14)
// ============================================================================
static void updateAutonomousTotemSupply() {
  if (s_status.uiEngineerMode || s_status.emergencyStop || !s_status.cycleRunning || s_status.cyclePaused) {
    return;
  }

  const MultimeterTelemetry& tel = halGetTelemetry();
  if (s_status.knobPos == POS_2_LLENANDO_TINA || s_status.knobPos == POS_3_TALLADO_JABON) {
    if (!tel.lsh01Closed) {
      halSetActuator(ACT_K1_M1, true);  // GPIO 14: Bixolon #2 MOSFET 2 (Bomba Evans 24V / Timer ASL-52)
    } else {
      halSetActuator(ACT_K1_M1, false);
    }
  } else {
    halSetActuator(ACT_K1_M1, false);
  }
}

// ============================================================================
// 4. TRANSICIÓN INTERNA ENTRE POSICIONES DE LA PERILLA
// ============================================================================
static void enterKnobStage(WasherKnobPos newPos) {
  halTurnOffAllActuators();

  s_status.knobPos              = newPos;
  s_status.knobAngleDeg         = KNOB_ANGLE_DEG[(uint8_t)newPos];
  s_status.stageTitle           = getStageTitleByPos(newPos);
  s_status.stageSubtitle        = getStageSubtitleByPos(newPos);
  s_status.stageElapsedSec      = 0;
  s_status.waitingManualValve   = false;
  s_status.roFlushingActive     = false;
  s_status.roFlushRemainingSec  = 0;
  s_status.zeroFlowTimerSec     = 0;
  s_status.passivePauseActive   = false;

  s_stageStartMs                = millis();
  s_lastFaradayTickMs           = millis();
  s_lastSecondTickMs            = millis();
  s_anodeTouchStartMs           = 0;
  s_zeroFlowStartMs             = 0;
  s_electrolysisRunAccMs        = 0;
  s_passivePauseStartMs         = 0;
  s_passiveSampleTakenInPause   = false;

  if (newPos == POS_1_APAGADO) {
    s_status.cycleRunning    = false;
    s_status.cyclePaused     = false;
    s_status.whirlpoolPrimed = false;
    halSetServoAngle(SERVO_ANGULO_REPOSO);
  } else if (newPos == POS_2_LLENANDO_TINA) {
    s_status.dissolvedFeGrams    = 0.0f;
    s_status.accumulatedCoulombs = 0.0f;
    s_status.faradayProgressPct  = 0.0f;
    s_status.whirlpoolPrimed     = false;
    s_autoMagicCalculated        = false;
    recalculateLoadAndSoapTargets();
  } else if (newPos == POS_3_TALLADO_JABON) {
    // Toma una lectura base inicial en GPIO 36 antes de encender la fuente ATX
    halSampleAndLogPassiveAnode(0, s_status.accumulatedCoulombs, s_status.dissolvedFeGrams);
  } else if (newPos == POS_4_REMOJO_CALMA) {
    s_status.soakRemainingSec = s_status.soakTotalSec;
  } else if (newPos == POS_7_SECADO_SOL_RO_100) {
    s_status.roFlushingActive    = true;
    s_status.roFlushRemainingSec = TIEMPO_FLUSHING_BYPASS_RO_MS / 1000;
    halSetServoAngle(SERVO_ANGULO_FLUSHING_ABIERTO);
  }

  Serial.printf("[LAVADORA FSM] Perilla -> Pos %d (%s)\n", (int)newPos, s_status.stageTitle);
}

// ============================================================================
// 5. INICIALIZACIÓN DE LA MÁQUINA DE ESTADOS
// ============================================================================
void fsmInit() {
  s_status.uiEngineerMode        = false;
  s_status.cycleRunning          = false;
  s_status.cyclePaused           = false;
  s_status.dryRunSimulation      = false;
  s_status.loadSize              = LOAD_FULL_117L;
  s_status.soapType              = SOAP_AUTO_MAGIC;
  s_status.measuredInitialR0Ohms = 5.0f;
  s_status.estimatedWaterPpm     = 2400.0f;
  s_status.dissolvedFeGrams      = 0.0f;
  s_status.accumulatedCoulombs   = 0.0f;
  s_status.faradayProgressPct    = 0.0f;
  s_status.whirlpoolPrimed       = false;
  s_status.passivePauseActive    = false;
  s_status.soakTotalSec          = TIEMPO_REMOJO_SEGUNDOS_DEF;
  s_status.soakRemainingSec      = TIEMPO_REMOJO_SEGUNDOS_DEF;
  s_status.emergencyStop         = false;
  s_status.emergencyReason       = "";
  s_status.bellDingCounter       = 0;

  recalculateLoadAndSoapTargets();
  enterKnobStage(POS_1_APAGADO);
}

// ============================================================================
// 6. BUCLE PRINCIPAL DE LA MÁQUINA DE ESTADOS (NO BLOQUEANTE CON MILLIS)
// ============================================================================
void fsmUpdate() {
  const MultimeterTelemetry& tel = halGetTelemetry();
  uint32_t now = millis();

  // --------------------------------------------------------------------------
  // A. VIGILANCIA PERMANENTE DE SEGURIDAD (REBOSE GPIO 21 Y ATASCO DC GPIO 35)
  // --------------------------------------------------------------------------
  if (tel.lshh02Closed) {
    if (!s_status.emergencyStop) {
      fsmTriggerEmergency("PARO DE EMERGENCIA LSHH-02: Rebose detectado en tapa del cono (GPIO 21).");
    }
    return;
  }

  if (tel.ct2DeadHead && !s_status.uiEngineerMode) {
    if (!s_status.emergencyStop) {
      fsmTriggerEmergency("PARO POR SOBRECORRIENTE CT2: Bomba de diafragma DC en sobrecorriente (GPIO 35).");
    }
    return;
  }

  if (s_status.uiEngineerMode || s_status.emergencyStop || !s_status.cycleRunning || s_status.cyclePaused) {
    if (s_status.uiEngineerMode && halIsActuatorOn(ACT_SSR1_40A)) {
      if (now - s_lastFaradayTickMs >= 200) {
        uint32_t dtMs = now - s_lastFaradayTickMs;
        float dtSec   = (float)dtMs / 1000.0f;
        s_lastFaradayTickMs = now;
        float currentDcAmps = tel.ct1Amps;
        float deltaCoulombs = currentDcAmps * dtSec;
        float deltaFeGrams  = deltaCoulombs * FARADAY_FE_G_PER_AMP_SEC;
        s_status.accumulatedCoulombs += deltaCoulombs;
        s_status.dissolvedFeGrams    += deltaFeGrams;
        s_status.faradayProgressPct   = constrain(
          (s_status.dissolvedFeGrams / s_status.targetFeGrams) * 100.0f, 0.0f, 100.0f
        );
      }
    } else {
      s_lastFaradayTickMs = now;
    }
    s_lastSecondTickMs  = now;
    return;
  }

  // --------------------------------------------------------------------------
  // B. CONTROL AUTÓNOMO DEL TÓTEM 1 (GPIO 19 -> BOMBA EVANS GPIO 14)
  // --------------------------------------------------------------------------
  updateAutonomousTotemSupply();

  // --------------------------------------------------------------------------
  // C. ACTUALIZACIÓN DE RELOJ DE ETAPA CADA 1 SEGUNDO
  // --------------------------------------------------------------------------
  if (now - s_lastSecondTickMs >= 1000) {
    uint32_t deltaSec = (now - s_lastSecondTickMs) / 1000;
    s_lastSecondTickMs += deltaSec * 1000;
    s_status.stageElapsedSec += deltaSec;

    if (s_status.knobPos == POS_4_REMOJO_CALMA && !s_status.waitingManualValve) {
      if (s_status.soakRemainingSec > deltaSec) {
        s_status.soakRemainingSec -= deltaSec;
      } else {
        s_status.soakRemainingSec = 0;
        s_status.waitingManualValve = true;
        halRingCycleEndBell();
        s_status.bellDingCounter++;
        Serial.println("[LAVADORA FSM] ¡Ding! Remojo terminado. Listo para Enjuague Triple Filtro.");
      }
    }
  }

  uint32_t elapsedStageMs = now - s_stageStartMs;

  // --------------------------------------------------------------------------
  // D. LÓGICA DE LAS 7 POSICIONES DE LA PERILLA CENTRAL
  // --------------------------------------------------------------------------
  switch (s_status.knobPos) {

    // ========================================================================
    // POSICIÓN 1: APAGADO / LISTA PARA LAVAR
    // ========================================================================
    case POS_1_APAGADO:
      halTurnOffAllActuators();
      s_status.cycleRunning = false;
      break;

    // ========================================================================
    // POSICIÓN 2: LLENANDO TINA
    // Regla de Protección de la Bomba de Pecera (GPIO 26):
    // NO se enciende al iniciar el llenado para evitar correr en seco;
    // solo se enciende cuando el nivel alcanza el último tercio (>= 66.7%).
    // ========================================================================
    case POS_2_LLENANDO_TINA: {
      float currentAmps  = tel.ct1Amps;
      float currentLevel = tel.pt1LevelPct;

      if (s_status.dryRunSimulation && currentAmps < 0.1f) {
        currentAmps  = 11.5f + 1.5f * sinf((float)elapsedStageMs / 2000.0f);
        currentLevel = constrain((float)elapsedStageMs / 180.0f, 0.0f, s_status.targetLevelPct);
      }

      // Protección estricta de la Bomba de Pecera (Whirlpool GPIO 26):
      // APAGADA hasta que el nivel de agua alcance el último tercio (>= 66.7%) para cebado por gravedad
      if (currentLevel >= NIVEL_CEBADO_WHIRLPOOL_PCT) {
        s_status.whirlpoolPrimed = true;
        halSetActuator(ACT_K3_M2, true);  // GPIO 26 ON (cebada por gravedad desde espiga de 1/8")
      } else {
        s_status.whirlpoolPrimed = false;
        halSetActuator(ACT_K3_M2, false); // GPIO 26 OFF (protección contra giro en seco)
      }

      // Modulación de Solenoide de Bronce 3/4" Llenado (SSR Fotek #2 - GPIO 27)
      bool yv1State = halGetActuatorTable()[ACT_K2_YV1].state;
      if (currentAmps >= CORRIENTE_MAX_LLENADO_A) {
        halSetActuator(ACT_K2_YV1, false);
      } else if (currentAmps <= CORRIENTE_HISTERESIS_A || !yv1State) {
        halSetActuator(ACT_K2_YV1, true);
      }

      // Cuando el nivel alcanza la meta (o tras llenado en modo simulación), pasa a Posición 3
      bool levelReached = (currentLevel >= s_status.targetLevelPct);
      if (levelReached) {
        s_status.whirlpoolPrimed = true;
        enterKnobStage(POS_3_TALLADO_JABON);
      }
      break;
    }

    // ========================================================================
    // POSICIÓN 3: ELECTROFLOCULACIÓN POR CULOMBIOS
    // 1. Integrador de Faraday: Suma I_DC (estimada desde I_AC en GPIO 34) * dt
    //    hasta alcanzar exactamente 7.60 gramos de Hierro disuelto (~29-35 min a 15A).
    // 2. Protección de Bomba de Pecera (GPIO 26): Activa solo con último tercio cebado.
    // 3. Muestreo del Ánodo Pasivo (GPIO 36 / VP): Cada 60s apaga P33 por 1.5s,
    //    lee potencial galvánico (~0.8V base) y guarda en LittleFS CSV.
    // ========================================================================
    case POS_3_TALLADO_JABON: {
      float currentLevel = tel.pt1LevelPct;
      if (s_status.dryRunSimulation || currentLevel >= NIVEL_CEBADO_WHIRLPOOL_PCT || s_status.whirlpoolPrimed) {
        s_status.whirlpoolPrimed = true;
        halSetActuator(ACT_K3_M2, true); // GPIO 26: Bomba Pecera Whirlpool ON (ya cebada en último tercio)
      } else {
        halSetActuator(ACT_K3_M2, false); // Protección contra corrida en seco si nivel cae bajo 66.7%
      }
      halSetActuator(ACT_K2_YV1, false); // Solenoide de llenado cerrada en electrólisis principal

      // --- A. Máquina de Muestreo del Ánodo Pasivo cada 60s (Pausa de 1.5s con P33 = OFF) ---
      if (s_passivePauseStartMs != 0) {
        uint32_t pauseElapsed = now - s_passivePauseStartMs;
        s_status.passivePauseActive = true;
        halSetActuator(ACT_SSR1_40A, false); // GPIO 33 = OFF durante los 1.5 segundos

        // A los 1.2s de pausa (electrolito en calma sin fuente ATX), muestrea GPIO 36 y escribe en LittleFS CSV
        if (!s_passiveSampleTakenInPause && pauseElapsed >= INSTANTE_LECTURA_VP_MS) {
          s_passiveSampleTakenInPause = true;
          halSampleAndLogPassiveAnode(s_status.stageElapsedSec,
                                      s_status.accumulatedCoulombs,
                                      s_status.dissolvedFeGrams);
        }

        // Al cumplirse exactamente 1.5 segundos (1500 ms), reanuda la fuente ATX (P33 = ON)
        if (pauseElapsed >= PAUSA_SEGURIDAD_ANODO_MS) {
          s_passivePauseStartMs       = 0;
          s_passiveSampleTakenInPause = false;
          s_status.passivePauseActive = false;
          s_electrolysisRunAccMs      = 0;
          s_lastFaradayTickMs         = now;
          halSetActuator(ACT_SSR1_40A, true); // Reanuda SSR Fotek #1 (GPIO 33 = ON)
        }
        break; // No integra corriente mientras la fuente ATX está en pausa de 1.5s
      }

      // Electrólisis activa: SSR Fotek #1 encendido (GPIO 33 = ON)
      halSetActuator(ACT_SSR1_40A, true);
      s_status.passivePauseActive = false;

      // --- B. Integrador de Faraday por Culombios cada 200 ms ---
      if (now - s_lastFaradayTickMs >= 200) {
        uint32_t dtMs = now - s_lastFaradayTickMs;
        float dtSec   = (float)dtMs / 1000.0f;
        s_lastFaradayTickMs = now;
        s_electrolysisRunAccMs += dtMs;

        // Corriente DC estimada desde el Sensor de Corriente #1 AC (GPIO 34)
        float currentDcAmps = tel.ct1Amps;
        if (s_status.dryRunSimulation && currentDcAmps < 0.1f) {
          currentDcAmps = CORRIENTE_NOMINAL_ATX_DC_A; // 15.0 A nominales en simulación
        }

        float simMultiplier = (s_status.dryRunSimulation && tel.ct1Amps < 0.1f) ? 15.0f : 1.0f;
        float deltaCoulombs = currentDcAmps * dtSec * simMultiplier;
        float deltaFeGrams  = deltaCoulombs * FARADAY_FE_G_PER_AMP_SEC;

        s_status.accumulatedCoulombs += deltaCoulombs;
        s_status.dissolvedFeGrams    += deltaFeGrams;
        s_status.faradayProgressPct   = constrain(
          (s_status.dissolvedFeGrams / s_status.targetFeGrams) * 100.0f, 0.0f, 100.0f
        );

        // ¿Se alcanzó la meta exacta de 7.60 g de Hierro disuelto?
        if (s_status.dissolvedFeGrams >= s_status.targetFeGrams) {
          s_status.dissolvedFeGrams   = s_status.targetFeGrams;
          s_status.faradayProgressPct = 100.0f;
          halSetActuator(ACT_SSR1_40A, false); // Corta SSR Fotek #1 (P33 = OFF)
          halSampleAndLogPassiveAnode(s_status.stageElapsedSec,
                                      s_status.accumulatedCoulombs,
                                      s_status.dissolvedFeGrams);
          Serial.printf("[FARADAY] Meta alcanzada: %.2f g Fe (%.1f C). Pasando a Remojo.\n",
                        s_status.dissolvedFeGrams, s_status.accumulatedCoulombs);
          enterKnobStage(POS_4_REMOJO_CALMA);
          break;
        }

        // ¿Se cumplieron 60 segundos de electrólisis? -> Inicia pausa de 1.5s para muestrear Ánodo Pasivo (GPIO 36)
        if (s_electrolysisRunAccMs >= PERIODO_MUESTREO_ANODO_MS) {
          s_passivePauseStartMs       = now;
          s_passiveSampleTakenInPause = false;
          s_status.passivePauseActive = true;
          halSetActuator(ACT_SSR1_40A, false); // Apaga fuente ATX (P33 = OFF) por 1.5s
        }
      }
      break;
    }

    // ========================================================================
    // POSICIÓN 4: REMOJO EN CALMA (Decantación en cono de 60°)
    // ========================================================================
    case POS_4_REMOJO_CALMA: {
      halSetActuator(ACT_SSR1_40A, false);
      halSetActuator(ACT_K2_YV1,   false);
      halSetActuator(ACT_K3_M2,    false);

      if (s_status.waitingManualValve && s_status.dryRunSimulation && s_status.stageElapsedSec >= (s_status.soakTotalSec + 6)) {
        enterKnobStage(POS_5_ENJUAGUE_FILTROS);
      }
      break;
    }

    // ========================================================================
    // POSICIÓN 5: ENJUAGUE TRIPLE FILTRO (Bixolon #2 M1 GPIO 5 hasta FT1 = 0 x 10s)
    // ========================================================================
    case POS_5_ENJUAGUE_FILTROS: {
      halSetActuator(ACT_K4_M3, true); // GPIO 5: Bixolon #2 MOSFET 1 (Relé Automotriz 30A / Bomba Trasvase)

      if (elapsedStageMs >= 4000) {
        if (tel.ft1FlowLpm <= 0.15f) {
          if (s_zeroFlowStartMs == 0) {
            s_zeroFlowStartMs = now;
          }
          uint32_t zeroElapsedMs = now - s_zeroFlowStartMs;
          s_status.zeroFlowTimerSec = zeroElapsedMs / 1000;

          if (zeroElapsedMs >= TIEMPO_FLUJO_CERO_FIN_MS) {
            halSetActuator(ACT_K4_M3, false);
            enterKnobStage(POS_6_EXPRIMIDO_CASA_60);
          }
        } else {
          s_zeroFlowStartMs = 0;
          s_status.zeroFlowTimerSec = 0;
        }
      }
      break;
    }

    // ========================================================================
    // POSICIÓN 6: EXPRIMIDO CASA (Relé Lógico #2 GPIO 15 + Bomba Manifold GPIO 23)
    // ========================================================================
    case POS_6_EXPRIMIDO_CASA_60: {
      halSetActuator(ACT_K6_YV2,    true);  // GPIO 15: Solenoide Bronce 3/4" Casa
      halSetActuator(ACT_K5_M4_UV1, true);  // GPIO 23: Bomba Diafragma #2 Manifold 12V DC
      halSetActuator(ACT_K7_YV3,    false); // GPIO 25: Pre-RO OFF
      halSetActuator(ACT_K8_YV4,    false); // GPIO 13: Bypass RO OFF

      if (elapsedStageMs >= TIEMPO_EXPRIMIDO_CASA_MS) {
        halSetActuator(ACT_K6_YV2, false);
        enterKnobStage(POS_7_SECADO_SOL_RO_100);
      }
      break;
    }

    // ========================================================================
    // POSICIÓN 7: ÓSMOSIS INVERSA 100 PSI (GPIO 23 + GPIO 25 + GPIO 13 + Servo GPIO 16)
    // ========================================================================
    case POS_7_SECADO_SOL_RO_100: {
      halSetActuator(ACT_K5_M4_UV1, true);  // GPIO 23: Bomba Diafragma #2 Manifold 12V DC
      halSetActuator(ACT_K7_YV3,    true);  // GPIO 25: Bixolon #1 MOSFET 1 (Solenoide 1/2" Pre-RO 12V DC)
      halSetActuator(ACT_K6_YV2,    false); // GPIO 15: Solenoide Casa OFF

      if (elapsedStageMs < TIEMPO_FLUSHING_BYPASS_RO_MS) {
        s_status.roFlushingActive = true;
        s_status.roFlushRemainingSec = (TIEMPO_FLUSHING_BYPASS_RO_MS - elapsedStageMs + 999) / 1000;
        halSetActuator(ACT_K8_YV4, true); // GPIO 13: Bixolon #1 MOSFET 2 (Solenoide 1/2" Bypass RO)
        if (halGetServoAngle() != SERVO_ANGULO_FLUSHING_ABIERTO) {
          halSetServoAngle(SERVO_ANGULO_FLUSHING_ABIERTO);
        }
      } else {
        s_status.roFlushingActive = false;
        s_status.roFlushRemainingSec = 0;
        halSetActuator(ACT_K8_YV4, false);
        if (halGetServoAngle() != SERVO_ANGULO_RO_100_PSI) {
          halSetServoAngle(SERVO_ANGULO_RO_100_PSI);
        }

        bool roTankSatisfied = (!tel.psl03Closed && elapsedStageMs >= 25000);
        if (roTankSatisfied || elapsedStageMs >= TIEMPO_MAX_SECADO_RO_MS) {
          halTurnOffAllActuators();
          halSetServoAngle(SERVO_ANGULO_REPOSO);
          halRingCycleEndBell();
          s_status.bellDingCounter++;
          Serial.println("[LAVADORA FSM] ¡Ding Final! Ciclo de Purificación Nodo 2 completado.");
          enterKnobStage(POS_1_APAGADO);
        }
      }
      break;
    }
  }
}

// ============================================================================
// 7. CONTROLES EXTERNOS (API WEB / BOTONES DE LA LAVADORA)
// ============================================================================
void fsmSetEngineerMode(bool enabled) {
  s_status.uiEngineerMode = enabled;
  if (enabled) {
    s_status.cycleRunning = false;
    s_status.cyclePaused  = false;
    halTurnOffAllActuators();
    Serial.println("[MODO INGENIERO] Ciclo automático desactivado para comisionamiento manual.");
  } else {
    halTurnOffAllActuators();
    enterKnobStage(POS_1_APAGADO);
    Serial.println("[MODO CLIENTE] Interfaz Lavadora Años 70 activa.");
  }
}

void fsmSetLoadSize(LoadSizeMode sizeMode) {
  s_status.loadSize = sizeMode;
  recalculateLoadAndSoapTargets();
}

void fsmSetSoapType(SoapTypeMode soapMode) {
  s_status.soapType = soapMode;
  if (soapMode != SOAP_AUTO_MAGIC) {
    s_autoMagicCalculated = true;
  } else {
    s_autoMagicCalculated = false;
  }
  recalculateLoadAndSoapTargets();
}

void fsmSetKnobPosition(WasherKnobPos pos, bool startImmediately) {
  if (s_status.emergencyStop) return;
  s_status.uiEngineerMode = false;
  enterKnobStage(pos);
  if (pos != POS_1_APAGADO && startImmediately) {
    s_status.cycleRunning = true;
    s_status.cyclePaused  = false;
  }
}

void fsmStartOrResume() {
  if (s_status.emergencyStop) return;
  s_status.uiEngineerMode = false;
  if (s_status.knobPos == POS_1_APAGADO) {
    enterKnobStage(POS_2_LLENANDO_TINA);
  }
  s_status.cycleRunning = true;
  s_status.cyclePaused  = false;
  s_lastFaradayTickMs   = millis();
  s_lastSecondTickMs    = millis();
}

void fsmPause() {
  s_status.cyclePaused = true;
  halTurnOffAllActuators();
}

void fsmAdvanceNextStage() {
  if (s_status.emergencyStop) return;
  s_status.uiEngineerMode = false;
  uint8_t next = (uint8_t)s_status.knobPos + 1;
  if (next > 7) {
    enterKnobStage(POS_1_APAGADO);
  } else {
    enterKnobStage((WasherKnobPos)next);
    s_status.cycleRunning = true;
    s_status.cyclePaused  = false;
  }
}

void fsmSetDryRunSimulation(bool enabled) {
  s_status.dryRunSimulation = enabled;
}

void fsmSetSoakDurationSeconds(uint32_t seconds) {
  s_status.soakTotalSec = constrain(seconds, 10UL, 7200UL);
  if (s_status.knobPos == POS_4_REMOJO_CALMA) {
    s_status.soakRemainingSec = s_status.soakTotalSec;
    s_status.waitingManualValve = false;
  }
}

void fsmTriggerEmergency(const char* reason) {
  s_status.emergencyStop   = true;
  s_status.cycleRunning    = false;
  s_status.cyclePaused     = false;
  s_status.emergencyReason = String(reason);
  halTurnOffAllActuators();
  halRingCycleEndBell();
  s_status.bellDingCounter++;
  Serial.printf("[EMERGENCIA] %s\n", reason);
}

void fsmClearEmergency() {
  s_status.emergencyStop   = false;
  s_status.emergencyReason = "";
  halTurnOffAllActuators();
  enterKnobStage(POS_1_APAGADO);
}

const WasherCycleStatus& fsmGetStatus() {
  return s_status;
}
