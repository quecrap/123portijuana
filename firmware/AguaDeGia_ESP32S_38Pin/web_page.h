#ifndef WEB_PAGE_H
#define WEB_PAGE_H

#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <title>Agua de Gía - Control SCADA ESP32S</title>
  <style>
    :root {
      --bg: #090d16;
      --card-bg: #131b2e;
      --card-border: #1e293b;
      --text: #f1f5f9;
      --text-muted: #94a3b8;
      --cyan: #06b6d4;
      --cyan-glow: rgba(6, 182, 212, 0.4);
      --green: #10b981;
      --green-glow: rgba(16, 185, 129, 0.45);
      --red: #ef4444;
      --red-glow: rgba(239, 68, 68, 0.5);
      --amber: #f59e0b;
      --purple: #8b5cf6;
      --dark-btn: #1e293b;
      --dark-btn-hover: #334155;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif; -webkit-tap-highlight-color: transparent; }
    body { background-color: var(--bg); color: var(--text); padding-bottom: 40px; font-size: 14px; }
    
    /* Header */
    header { background: #0c1322; border-bottom: 1px solid var(--card-border); padding: 12px 16px; position: sticky; top: 0; z-index: 100; display: flex; justify-content: space-between; align-items: center; }
    .brand { display: flex; align-items: center; gap: 8px; }
    .brand-icon { width: 28px; height: 28px; background: linear-gradient(135deg, #06b6d4, #3b82f6); border-radius: 8px; display: flex; align-items: center; justify-content: center; font-size: 16px; font-weight: bold; }
    .brand-title { font-size: 15px; font-weight: 700; color: #fff; letter-spacing: 0.5px; }
    .brand-sub { font-size: 9.5px; color: var(--cyan); text-transform: uppercase; font-weight: 600; }
    .net-pill { display: flex; align-items: center; gap: 6px; background: rgba(16, 185, 129, 0.15); border: 1px solid rgba(16, 185, 129, 0.3); padding: 4px 10px; border-radius: 999px; font-size: 11px; font-weight: 600; color: #34d399; }
    .net-dot { width: 7px; height: 7px; background: #10b981; border-radius: 50%; box-shadow: 0 0 8px #10b981; animation: pulse-green 2s infinite; }
    
    @keyframes pulse-green {
      0% { opacity: 1; transform: scale(1); }
      50% { opacity: 0.4; transform: scale(0.85); }
      100% { opacity: 1; transform: scale(1); }
    }
    @keyframes pulse-red {
      0% { opacity: 1; box-shadow: 0 0 10px var(--red); }
      50% { opacity: 0.4; box-shadow: 0 0 25px var(--red); }
      100% { opacity: 1; box-shadow: 0 0 10px var(--red); }
    }

    /* Tabs Bar (3 pestañas móviles) */
    .tab-bar { display: flex; background: #0c1322; padding: 6px 10px; gap: 6px; border-bottom: 1px solid var(--card-border); overflow-x: auto; }
    .tab-btn { flex: 1; min-width: 90px; padding: 9px 4px; background: transparent; border: 1px solid transparent; border-radius: 8px; color: var(--text-muted); font-size: 11px; font-weight: 700; cursor: pointer; display: flex; align-items: center; justify-content: center; gap: 4px; transition: all 0.2s; white-space: nowrap; }
    .tab-btn.active { background: #1e293b; color: #fff; border-color: #334155; box-shadow: 0 2px 8px rgba(0,0,0,0.4); }
    .tab-btn.active.tab-auto { color: var(--cyan); border-color: rgba(6, 182, 212, 0.4); }
    .tab-btn.active.tab-bench { color: var(--amber); border-color: rgba(245, 158, 11, 0.4); }
    .tab-btn.active.tab-pins { color: #a78bfa; border-color: rgba(167, 139, 250, 0.4); }

    .container { padding: 14px; max-width: 760px; margin: 0 auto; }
    
    /* Emergency Alert Banner */
    .emergency-banner { display: none; background: rgba(239, 68, 68, 0.2); border: 2px solid var(--red); border-radius: 10px; padding: 12px; margin-bottom: 14px; animation: pulse-red 1.2s infinite; text-align: center; }
    .emergency-banner.show { display: block; }
    .emergency-title { font-weight: 800; color: #fca5a5; font-size: 14px; text-transform: uppercase; letter-spacing: 0.5px; }
    .emergency-desc { font-size: 12px; color: #fecaca; margin-top: 4px; }

    /* Section Cards */
    .card { background: var(--card-bg); border: 1px solid var(--card-border); border-radius: 12px; padding: 14px; margin-bottom: 14px; box-shadow: 0 4px 12px rgba(0,0,0,0.25); }
    .card-header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 12px; }
    .card-title { font-size: 13px; font-weight: 700; color: #e2e8f0; text-transform: uppercase; letter-spacing: 0.5px; display: flex; align-items: center; gap: 6px; }
    .card-badge { font-size: 10px; padding: 2px 8px; border-radius: 4px; font-weight: 700; background: #1e293b; color: var(--text-muted); }

    /* Stepper */
    .stepper { display: flex; flex-direction: column; gap: 8px; }
    .step-item { display: flex; align-items: center; gap: 10px; padding: 8px 10px; border-radius: 8px; background: #0c1322; border: 1px solid #1e293b; font-size: 12px; font-weight: 500; }
    .step-item.active { border-color: var(--cyan); background: rgba(6, 182, 212, 0.1); font-weight: 700; color: #fff; }
    .step-item.done { border-color: rgba(16, 185, 129, 0.3); color: #34d399; }
    .step-num { width: 22px; height: 22px; border-radius: 50%; background: #1e293b; display: flex; align-items: center; justify-content: center; font-size: 10px; font-weight: 800; color: var(--text-muted); flex-shrink: 0; }
    .step-item.active .step-num { background: var(--cyan); color: #000; box-shadow: 0 0 8px var(--cyan); }
    .step-item.done .step-num { background: var(--green); color: #000; }

    /* Telemetry Grid */
    .telem-grid { display: grid; grid-template-columns: repeat(2, 1fr); gap: 10px; }
    .telem-box { background: #0c1322; border: 1px solid #1e293b; border-radius: 8px; padding: 10px; display: flex; flex-direction: column; justify-content: space-between; }
    .telem-label { font-size: 11px; color: var(--text-muted); text-transform: uppercase; font-weight: 600; }
    .telem-value { font-size: 20px; font-weight: 800; color: #fff; margin: 4px 0; font-family: monospace; }
    .telem-unit { font-size: 12px; font-weight: 500; color: var(--cyan); margin-left: 2px; }
    .telem-sub { font-size: 10px; color: #64748b; }

    /* Progress bar */
    .prog-track { width: 100%; height: 8px; background: #1e293b; border-radius: 999px; overflow: hidden; margin-top: 6px; }
    .prog-fill { height: 100%; background: linear-gradient(90deg, #06b6d4, #3b82f6); width: 0%; transition: width 0.3s; }
    .prog-fill.amber { background: linear-gradient(90deg, #f59e0b, #ef4444); }

    /* Master Buttons */
    .btn-row { display: flex; gap: 8px; margin-top: 10px; }
    .btn { flex: 1; padding: 12px; border-radius: 8px; border: none; font-size: 12px; font-weight: 700; cursor: pointer; display: flex; align-items: center; justify-content: center; gap: 6px; transition: all 0.15s; text-transform: uppercase; letter-spacing: 0.5px; }
    .btn:active { transform: scale(0.97); }
    .btn-primary { background: linear-gradient(135deg, #06b6d4, #0284c7); color: #fff; box-shadow: 0 4px 12px var(--cyan-glow); }
    .btn-success { background: linear-gradient(135deg, #10b981, #059669); color: #fff; box-shadow: 0 4px 12px var(--green-glow); }
    .btn-danger { background: linear-gradient(135deg, #ef4444, #dc2626); color: #fff; box-shadow: 0 4px 12px var(--red-glow); }
    .btn-dark { background: var(--dark-btn); color: var(--text); border: 1px solid #334155; }
    .btn-amber { background: linear-gradient(135deg, #f59e0b, #d97706); color: #000; font-weight: 800; }

    /* TEST BENCH STYLES */
    .bench-intro { background: rgba(245, 158, 11, 0.08); border: 1px solid rgba(245, 158, 11, 0.3); border-radius: 8px; padding: 10px 12px; margin-bottom: 14px; font-size: 12px; color: #fde68a; line-height: 1.4; }
    
    /* Live Inputs Cards */
    .inputs-grid { display: grid; grid-template-columns: 1fr; gap: 8px; margin-bottom: 14px; }
    @media(min-width: 480px) { .inputs-grid { grid-template-columns: repeat(2, 1fr); } }
    
    .input-card { background: #0c1322; border: 2px solid #1e293b; border-radius: 10px; padding: 10px 12px; display: flex; justify-content: space-between; align-items: center; transition: all 0.2s; }
    .input-info { display: flex; flex-direction: column; gap: 2px; }
    .input-name { font-size: 12px; font-weight: 700; color: #fff; }
    .input-pin { font-size: 10px; color: #64748b; font-family: monospace; }
    
    .status-badge { padding: 5px 10px; border-radius: 6px; font-size: 11px; font-weight: 800; text-transform: uppercase; letter-spacing: 0.5px; transition: all 0.2s; text-align: center; }
    .badge-dry { background: #1e293b; color: #94a3b8; border: 1px solid #334155; }
    .badge-active-green { background: #064e3b; color: #6ee7b7; border: 1px solid #10b981; box-shadow: 0 0 14px rgba(16, 185, 129, 0.7); animation: pulse-green 1s infinite; }
    .badge-active-red { background: #7f1d1d; color: #fca5a5; border: 1px solid #ef4444; box-shadow: 0 0 16px rgba(239, 68, 68, 0.8); animation: pulse-red 0.8s infinite; }
    .badge-open { background: #1e293b; color: #cbd5e1; border: 1px solid #475569; }
    .badge-closed { background: #1e3a8a; color: #93c5fd; border: 1px solid #3b82f6; }

    /* Actuators Grid */
    .actuators-grid { display: flex; flex-direction: column; gap: 10px; }
    .act-card { background: #0c1322; border: 1px solid #1e293b; border-radius: 10px; padding: 12px; transition: border-color 0.2s; }
    .act-card.is-on { border-color: rgba(16, 185, 129, 0.4); background: #0d1a29; }
    .act-top { display: flex; justify-content: space-between; align-items: flex-start; margin-bottom: 8px; }
    .act-title-box { display: flex; flex-direction: column; gap: 2px; }
    .act-title { font-size: 13px; font-weight: 700; color: #f1f5f9; }
    .act-desc { font-size: 11px; color: #94a3b8; }
    .act-pin-tag { font-family: monospace; font-size: 10px; background: #1e293b; padding: 1px 6px; border-radius: 4px; color: var(--cyan); align-self: flex-start; margin-top: 2px; }
    .act-state-pill { padding: 4px 8px; border-radius: 4px; font-size: 11px; font-weight: 800; font-family: monospace; }
    .pill-off { background: #1e293b; color: #64748b; }
    .pill-on { background: #065f46; color: #34d399; box-shadow: 0 0 8px rgba(16, 185, 129, 0.4); }
    
    .act-actions { display: grid; grid-template-columns: 1fr 1fr 1.3fr; gap: 6px; }
    .btn-act { padding: 8px 4px; border-radius: 6px; border: 1px solid transparent; font-size: 11px; font-weight: 700; cursor: pointer; text-align: center; }
    .btn-act-on { background: rgba(16, 185, 129, 0.15); border-color: rgba(16, 185, 129, 0.4); color: #34d399; }
    .btn-act-on:hover, .btn-act-on:active { background: #10b981; color: #000; }
    .btn-act-off { background: rgba(239, 68, 68, 0.1); border-color: rgba(239, 68, 68, 0.3); color: #fca5a5; }
    .btn-act-off:hover, .btn-act-off:active { background: #ef4444; color: #fff; }
    .btn-act-pulse { background: rgba(6, 182, 212, 0.15); border-color: rgba(6, 182, 212, 0.4); color: var(--cyan); position: relative; overflow: hidden; }
    .btn-act-pulse:hover, .btn-act-pulse:active { background: var(--cyan); color: #000; }
    .btn-act-pulse.pulsing { background: var(--cyan); color: #000; font-weight: 800; }

    /* Servo Calibrator */
    .servo-box { display: flex; flex-direction: column; gap: 12px; }
    .servo-display { display: flex; justify-content: space-between; align-items: center; }
    .servo-angle-text { font-size: 26px; font-weight: 900; color: var(--cyan); font-family: monospace; }
    .servo-slider { width: 100%; height: 10px; border-radius: 5px; background: #1e293b; outline: none; -webkit-appearance: none; }
    .servo-slider::-webkit-slider-thumb { -webkit-appearance: none; appearance: none; width: 26px; height: 26px; border-radius: 50%; background: var(--cyan); cursor: pointer; box-shadow: 0 0 10px var(--cyan); }
    .servo-presets { display: grid; grid-template-columns: repeat(5, 1fr); gap: 6px; }
    .preset-btn { padding: 8px 2px; border-radius: 6px; border: 1px solid #334155; background: #1e293b; color: #e2e8f0; font-size: 11px; font-weight: 700; cursor: pointer; text-align: center; }
    .preset-btn.active { background: var(--cyan); color: #000; border-color: var(--cyan); }

    /* PESTAÑA 3: DIAGRAMA DE PINES EN MÓVIL */
    .pin-group { margin-bottom: 12px; }
    .pin-group-title { font-size: 12px; font-weight: 800; text-transform: uppercase; margin-bottom: 6px; display: flex; align-items: center; gap: 6px; }
    .pin-row { background: #0c1322; border: 1px solid #1e293b; border-radius: 8px; padding: 8px 10px; margin-bottom: 6px; display: flex; justify-content: space-between; align-items: center; }
    .pin-badge { font-family: monospace; font-size: 11px; font-weight: 800; padding: 2px 6px; border-radius: 4px; }
    .pin-b-green { background: rgba(16, 185, 129, 0.2); color: #34d399; border: 1px solid rgba(16, 185, 129, 0.4); }
    .pin-b-yellow { background: rgba(245, 158, 11, 0.2); color: #fbbf24; border: 1px solid rgba(245, 158, 11, 0.4); }
    .pin-b-red { background: rgba(239, 68, 68, 0.2); color: #f87171; border: 1px solid rgba(239, 68, 68, 0.4); }
    .pin-b-cyan { background: rgba(6, 182, 212, 0.2); color: #38bdf8; border: 1px solid rgba(6, 182, 212, 0.4); }
    .pin-b-purple { background: rgba(168, 85, 247, 0.2); color: #c084fc; border: 1px solid rgba(168, 85, 247, 0.4); }

    /* Footer */
    footer { text-align: center; font-size: 11px; color: #475569; margin-top: 20px; }
  </style>
</head>
<body>

  <!-- Header -->
  <header>
    <div class="brand">
      <div class="brand-icon">💧</div>
      <div>
        <div class="brand-title">AGUA DE GÍA</div>
        <div class="brand-sub">NodeMCU ESP32S · 38 Pines</div>
      </div>
    </div>
    <div class="net-pill" id="net-pill">
      <div class="net-dot" id="net-dot"></div>
      <span id="ap-label">Comprobando enlace...</span>
    </div>
  </header>

  <!-- Navigation Tabs (3 pestañas) -->
  <div class="tab-bar">
    <button class="tab-btn tab-auto active" id="btn-tab-auto" onclick="switchTab('auto')">
      📊 Dashboard
    </button>
    <button class="tab-btn tab-bench" id="btn-tab-bench" onclick="switchTab('bench')">
      🛠️ Test Bench
    </button>
    <button class="tab-btn tab-pins" id="btn-tab-pins" onclick="switchTab('pins')">
      📐 Guía Pines
    </button>
  </div>

  <div class="container">
    
    <!-- Emergency Warning Banner -->
    <div class="emergency-banner" id="emergencyBanner">
      <div class="emergency-title">🚨 PARO DE EMERGENCIA GENERAL ACTIVADO</div>
      <div class="emergency-desc" id="emergencyReason">Rebose de seguridad detectado en electrodos o comando manual.</div>
      <div class="btn-row" style="margin-top: 8px;">
        <button class="btn btn-danger" onclick="resetEmergency()">REARMAR / REINICIAR ALARMA</button>
      </div>
    </div>

    <!-- ======================================================== -->
    <!-- PUENTE EN VIVO CELULAR <-> ANTIGRAVITY & BITÁCORA CAMPO  -->
    <!-- ======================================================== -->
    <div class="card" style="border-color: rgba(6, 182, 212, 0.65); background: linear-gradient(180deg, #0f1b33 0%, #131b2e 100%);">
      <div class="card-header">
        <div class="card-title" style="color:#38bdf8;">📲 Antigravity en Celular · Bitácora Multímetro</div>
        <div class="card-badge" id="agLogBadge" style="background:#064e3b; color:#34d399;">EN LÍNEA</div>
      </div>

      <!-- Respuesta en vivo de Antigravity -->
      <div style="background:#081020; border:1px solid rgba(6,182,212,0.45); border-radius:8px; padding:10px 12px; margin-bottom:10px;">
        <div style="font-size:10px; font-weight:800; color:#38bdf8; text-transform:uppercase; margin-bottom:3px;">🤖 Respuesta / Análisis de Antigravity:</div>
        <div id="agMessageBox" style="font-size:12.5px; color:#f1f5f9; line-height:1.45; font-weight:600;">
          Listo en línea. Mide con tu multímetro y envía cada lectura o mensaje desde aquí; yo lo recibo en la PC y te respondo en esta pantalla.
        </div>
      </div>

      <!-- Chips rápidos de etapa -->
      <div style="display:flex; gap:5px; overflow-x:auto; padding-bottom:6px; margin-bottom:8px;">
        <button class="preset-btn" style="padding:5px 8px; white-space:nowrap;" onclick="setNivelChip('En seco (0 cm)')">En seco</button>
        <button class="preset-btn" style="padding:5px 8px; white-space:nowrap;" onclick="setNivelChip('1er Toque Punta (1 cm)')">1er Toque</button>
        <button class="preset-btn" style="padding:5px 8px; white-space:nowrap;" onclick="setNivelChip('1/4 Varilla (6 pulg)')">1/4 Varilla</button>
        <button class="preset-btn" style="padding:5px 8px; white-space:nowrap;" onclick="setNivelChip('1/2 Varilla (12 pulg)')">1/2 Varilla</button>
        <button class="preset-btn" style="padding:5px 8px; white-space:nowrap;" onclick="setNivelChip('Cono Lleno 1BBL (Min 0)')">Lleno Min 0</button>
        <button class="preset-btn" style="padding:5px 8px; white-space:nowrap;" onclick="setNivelChip('Floculacion Min 5')">Min 5</button>
        <button class="preset-btn" style="padding:5px 8px; white-space:nowrap;" onclick="setNivelChip('Floculacion Min 15')">Min 15</button>
      </div>

      <!-- Formulario móvil de captura rápida -->
      <div style="display:grid; grid-template-columns: 1fr 1fr 1fr; gap:6px; margin-bottom:8px;">
        <div>
          <label style="font-size:10px; color:#94a3b8; font-weight:700;">MULTÍMETRO (V)</label>
          <input type="number" step="0.001" id="inpVMult" placeholder="Ej: 0.80" style="width:100%; margin-top:3px; padding:9px; border-radius:6px; border:1px solid #334155; background:#090d16; color:#fff; font-weight:800; font-size:14px;">
        </div>
        <div>
          <label style="font-size:10px; color:#94a3b8; font-weight:700;">NIVEL / ETAPA</label>
          <input type="text" id="inpNivel" placeholder="Ej: 10 pulg" style="width:100%; margin-top:3px; padding:9px; border-radius:6px; border:1px solid #334155; background:#090d16; color:#fff; font-weight:700; font-size:13px;">
        </div>
        <div>
          <label style="font-size:10px; color:#94a3b8; font-weight:700;">PINZA AC (A)</label>
          <input type="number" step="0.01" id="inpIAc" placeholder="Opcional" style="width:100%; margin-top:3px; padding:9px; border-radius:6px; border:1px solid #334155; background:#090d16; color:#fff; font-weight:700; font-size:13px;">
        </div>
      </div>

      <div style="margin-bottom:8px;">
        <input type="text" id="inpNota" placeholder="Escribe un mensaje o pregunta para Antigravity (ej. Ya tocó el agua, qué sigue)..." style="width:100%; padding:10px; border-radius:6px; border:1px solid #334155; background:#090d16; color:#fff; font-size:13px;">
      </div>

      <div class="btn-row" style="margin-top:4px;">
        <button class="btn btn-primary" id="btnSendBitacora" onclick="submitBitacora()">📨 ENVIAR A ANTIGRAVITY + GUARDAR</button>
        <a href="/api/bitacora.csv" class="btn btn-dark" style="flex:0.45; text-decoration:none; font-size:11px;">⬇️ CSV</a>
      </div>

      <!-- Mini tabla de últimas lecturas -->
      <div id="bitacoraListBox" style="margin-top:10px; max-height:130px; overflow-y:auto; font-size:11px; color:#cbd5e1; border-top:1px solid #1e293b; padding-top:6px;">
        Sin lecturas registradas aún en esta corrida.
      </div>
    </div>

    <!-- ======================================================== -->
    <!-- TAB 1: DASHBOARD MAESTRO AUTOMÁTICO                     -->
    <!-- ======================================================== -->
    <div id="tabContentAuto">
      
      <!-- Current Stage Banner -->
      <div class="card">
        <div class="card-header">
          <div class="card-title">🔄 Ciclo Hidráulico y Electroquímico</div>
          <div class="card-badge" id="autoModeBadge">AUTOMÁTICO</div>
        </div>
        
        <div class="stepper" id="stepperList">
          <div class="step-item" id="step0" style="cursor:pointer;" title="Clic para saltar a esta etapa" onclick="jumpToStage(0)"><div class="step-num">0</div><span>STANDBY / REPOSO INICIAL</span></div>
          <div class="step-item" id="step1" style="cursor:pointer;" title="Clic para saltar a esta etapa" onclick="jumpToStage(1)"><div class="step-num">1</div><span>LLENADO ADAPTATIVO CONO (Pulsos)</span></div>
          <div class="step-item" id="step2" style="cursor:pointer;" title="Clic para saltar a esta etapa" onclick="jumpToStage(2)"><div class="step-num">2</div><span>CÁLCULO QUÍMICO FARADAY (TDS inicial)</span></div>
          <div class="step-item" id="step3" style="cursor:pointer;" title="Clic para saltar a esta etapa" onclick="jumpToStage(3)"><div class="step-num">3</div><span>ELECTROFLOCULACIÓN ACTIVA (12V / Celda)</span></div>
          <div class="step-item" id="step4" style="cursor:pointer;" title="Clic para saltar a esta etapa" onclick="jumpToStage(4)"><div class="step-num">4</div><span>REPOSO FORZADO (45 MINUTOS)</span></div>
          <div class="step-item" id="step5" style="cursor:pointer;" title="Clic para saltar a esta etapa" onclick="jumpToStage(5)"><div class="step-num">5</div><span>TRASVASE Y FILTRADO MULTIETAPA</span></div>
          <div class="step-item" id="step6" style="cursor:pointer;" title="Clic para saltar a esta etapa" onclick="jumpToStage(6)"><div class="step-num">6</div><span>PRESURIZACIÓN MANIFOLD 50 PSI</span></div>
          <div class="step-item" id="step7" style="cursor:pointer;" title="Clic para saltar a esta etapa" onclick="jumpToStage(7)"><div class="step-num">7</div><span>ÓSMOSIS INVERSA 100 PSI (AGUA PURA)</span></div>
        </div>

        <!-- Muestra de TDS con Medidor Manual -->
        <div style="background: rgba(6, 182, 212, 0.08); border: 1px solid rgba(6, 182, 212, 0.35); border-radius: 10px; padding: 10px 14px; margin-top: 12px; margin-bottom: 6px; display: flex; align-items: center; justify-content: space-between; gap: 10px; flex-wrap: wrap;">
          <div style="font-size: 12px; font-weight: 700; color: #bae6fd; display: flex; align-items: center; gap: 6px;">
            <span>🧪 Muestra TDS (Medidor Físico):</span>
          </div>
          <div style="display: flex; align-items: center; gap: 6px;">
            <input type="number" id="inputTdsManual" placeholder="Ej. 850" min="50" max="8000" style="width: 100px; padding: 7px 8px; border-radius: 6px; border: 1px solid #06b6d4; background: #0c1322; color: #38bdf8; font-size: 14px; font-weight: 800; text-align: center; font-family: monospace;">
            <span style="font-size: 12px; color: #94a3b8; font-weight: 800;">PPM</span>
          </div>
        </div>

        <div class="btn-row">
          <button class="btn btn-success" onclick="sendAutoCmd('start')">▶ INICIAR LOTE</button>
          <button class="btn btn-amber" onclick="sendAutoCmd('pause')">⏸ PAUSAR</button>
          <button class="btn btn-danger" onclick="sendAutoCmd('stop')">🛑 PARO</button>
        </div>

        <!-- Selector Rápido para saltar a cualquier etapa -->
        <div style="margin-top:12px; padding-top:10px; border-top:1px solid #1e293b;">
          <div style="font-size:11px; font-weight:800; color:#38bdf8; margin-bottom:6px; display:flex; align-items:center; gap:6px;">
            <span>⚡ SALTO MANUAL DIRECTO A ETAPA:</span>
          </div>
          <div style="display:grid; grid-template-columns: repeat(4, 1fr); gap:6px;">
            <button class="btn btn-dark" style="font-size:10px; padding:6px 2px;" onclick="jumpToStage(0)">0: Standby</button>
            <button class="btn btn-dark" style="font-size:10px; padding:6px 2px;" onclick="jumpToStage(1)">1: Llenar</button>
            <button class="btn btn-dark" style="font-size:10px; padding:6px 2px;" onclick="jumpToStage(2)">2: Faraday</button>
            <button class="btn btn-dark" style="font-size:10px; padding:6px 2px;" onclick="jumpToStage(3)">3: Floc 12V</button>
            <button class="btn btn-dark" style="font-size:10px; padding:6px 2px;" onclick="jumpToStage(4)">4: Reposo</button>
            <button class="btn btn-dark" style="font-size:10px; padding:6px 2px;" onclick="jumpToStage(5)">5: Trasvase</button>
            <button class="btn btn-dark" style="font-size:10px; padding:6px 2px;" onclick="jumpToStage(6)">6: Manifold</button>
            <button class="btn btn-dark" style="font-size:10px; padding:6px 2px;" onclick="jumpToStage(7)">7: Ósmosis</button>
          </div>
        </div>
      </div>

      <!-- Live Process Telemetry -->
      <div class="card">
        <div class="card-header">
          <div class="card-title">⚡ Telemetría de Proceso en Vivo</div>
          <div class="card-badge" id="uptimeBadge">UP: 00:00</div>
        </div>

        <div class="telem-grid">
          <div class="telem-box">
            <div class="telem-label">Corriente Electrodos</div>
            <div class="telem-value" id="valCurrent40A">0.0<span class="telem-unit">A</span></div>
            <div class="telem-sub">Fuente 40A · Ánodo Fe</div>
            <div class="prog-track"><div class="prog-fill" id="barCurrent40A"></div></div>
          </div>

          <div class="telem-box">
            <div class="telem-label">Masa Fe Disuelta</div>
            <div class="telem-value" id="valFeMass">0.0<span class="telem-unit">/ 5.86g</span></div>
            <div class="telem-sub">Progreso Faraday: <span id="valFaradayPct">0%</span></div>
            <div class="prog-track"><div class="prog-fill" id="barFaraday"></div></div>
          </div>

          <div class="telem-box">
            <div class="telem-label">Caudalímetro Hall</div>
            <div class="telem-value" id="valFlowLpm">0.0<span class="telem-unit">L/min</span></div>
            <div class="telem-sub">Volumen: <span id="valTotalLiters">0.0</span> L</div>
            <div class="prog-track"><div class="prog-fill" id="barFlow"></div></div>
          </div>

          <div class="telem-box">
            <div class="telem-label">Corriente Bombas DC</div>
            <div class="telem-value" id="valCurrentDC">0.0<span class="telem-unit">A</span></div>
            <div class="telem-sub" id="labelPumpJam">Carga Nominal</div>
            <div class="prog-track"><div class="prog-fill amber" id="barCurrentDC"></div></div>
          </div>

          <div class="telem-box">
            <div class="telem-label">Nivel Cono (1.10m · 100 PSI)</div>
            <div class="telem-value" id="valHydroLevel">0.0<span class="telem-unit">cm</span></div>
            <div class="telem-sub" id="subHydroDetails">0.0% · 0.0 L · 0.00 mV (P32)</div>
            <div class="prog-track"><div class="prog-fill" id="barHydroLevel"></div></div>
            <div style="display:flex; gap:4px; margin-top:6px;">
              <button class="btn-act btn-act-on" style="flex:1; padding:4px; font-size:10px;" onclick="tarePt1Zero()">🎯 Cero (0cm)</button>
              <button class="btn-act btn-act-pulse" style="flex:1; padding:4px; font-size:10px;" onclick="calibratePt1HeightPrompt()">📏 Calibrar cm</button>
            </div>
          </div>

          <div class="telem-box">
            <div class="telem-label">Temperatura Agua</div>
            <div class="telem-value" id="valTemp">--<span class="telem-unit">°C</span></div>
            <div class="telem-sub">DS18B20 P4</div>
            <div class="prog-track"><div class="prog-fill" id="barTemp"></div></div>
          </div>

          <div class="telem-box" style="grid-column: span 2; border-color: rgba(6, 182, 212, 0.45);">
            <div style="display:flex; justify-content:space-between; align-items:center;">
              <div class="telem-label" style="color:#38bdf8;">🧪 Celda Electroquímica & Divisor 4:1 (Pin P36 / 3x10k + 1x10k)</div>
              <button class="btn-act btn-act-pulse" style="padding:4px 10px;" onclick="triggerSampleAnode()">⚡ PAUSA 1.5s MUESTREO P36</button>
            </div>
            <div style="display:grid; grid-template-columns: repeat(3, 1fr); gap:8px; margin-top:8px;">
              <div>
                <div class="telem-sub">Voltaje Celda Real (x4)</div>
                <div class="telem-value" id="valAnodeRod" style="font-size:18px;">0.000<span class="telem-unit">V</span></div>
                <div class="telem-sub" id="valAnodePin">Pin P36: 0.000 V</div>
              </div>
              <div>
                <div class="telem-sub">Con ATX ON / OFF(50ms)</div>
                <div class="telem-value" id="valAnodeOnOff" style="font-size:16px;">0.00 / 0.00<span class="telem-unit">V</span></div>
                <div class="telem-sub" id="valAnode1400">Remanente 1.4s: 0.000 V</div>
              </div>
              <div>
                <div class="telem-sub">Caída Óhmica (dV_IR) / Estado</div>
                <div class="telem-value" id="valAnodeIr" style="font-size:18px; color:#fbbf24;">0.000<span class="telem-unit">V</span></div>
                <div class="telem-sub" id="valAnodeStatus">Divisor 4:1 Conectado</div>
              </div>
            </div>
          </div>
        </div>
      </div>

    </div>

    <!-- ======================================================== -->
    <!-- TAB 2: TEST BENCH / MÓDULO DE PRUEBA Y CABLEADO MANUAL  -->
    <!-- ======================================================== -->
    <div id="tabContentBench" style="display: none;">
      
      <div class="bench-intro">
        🛠️ <b>BANCO DE PRUEBAS EN CAMPO:</b> Toca o puentea sensores para verificar continuidad instantánea (500ms) y usa <b>Pulso 3s</b> para escuchar el chasquido físico de cada relé.
      </div>

      <!-- Live Inputs Realtime Grid -->
      <div class="card">
        <div class="card-header">
          <div class="card-title">🔍 Monitor de Entradas en Vivo (500 ms)</div>
          <div class="card-badge" style="background:#064e3b; color:#34d399;">LIVE POLLING</div>
        </div>

        <div class="inputs-grid">
          <div class="input-card" id="cardTotemLevel">
            <div class="input-info">
              <div class="input-name">Nivel Alto Tótem 1</div>
              <div class="input-pin">Pin P19 · GPIO 19 (PULLUP)</div>
            </div>
            <div class="status-badge badge-dry" id="badgeTotemLevel">⚪ SECO / ABIERTO</div>
          </div>

          <div class="input-card" id="cardOverflowStop">
            <div class="input-info">
              <div class="input-name">Paro Emergencia Rebose Cono</div>
              <div class="input-pin">Pin P21 · GPIO 21 (2 Hilos Cobre)</div>
            </div>
            <div class="status-badge badge-dry" id="badgeOverflowStop">🟢 DESPEJADO</div>
          </div>

          <div class="input-card" id="cardRoPressure">
            <div class="input-info">
              <div class="input-name">Presostato Demanda RO (20L)</div>
              <div class="input-pin">Pin P22 · GPIO 22 (20-40 PSI)</div>
            </div>
            <div class="status-badge badge-open" id="badgeRoPressure">⚪ SIN DEMANDA</div>
          </div>

          <div class="input-card" id="cardHallSensor">
            <div class="input-info">
              <div class="input-name">Caudalímetro Hall (Flujo)</div>
              <div class="input-pin">Pin P18 · GPIO 18 (Interrupción)</div>
            </div>
            <div class="status-badge badge-open" id="badgeHallFlow">0.0 L/m (0 Hz)</div>
          </div>

          <div class="input-card">
            <div class="input-info">
              <div class="input-name">ADC Corriente Electrodos</div>
              <div class="input-pin">Pin P34 · GPIO 34 (ADC1_CH6)</div>
            </div>
            <div class="status-badge badge-open" id="badgeAdc40A">Raw: 0 | 0.0A</div>
          </div>

          <div class="input-card">
            <div class="input-info">
              <div class="input-name">ADC Corriente Bombas DC</div>
              <div class="input-pin">Pin P35 · GPIO 35 (ADC1_CH7)</div>
            </div>
            <div class="status-badge badge-open" id="badgeAdcDC">Raw: 0 | 0.0A</div>
          </div>

          <div class="input-card">
            <div class="input-info">
              <div class="input-name">Campana Hidrostática Cono</div>
              <div class="input-pin">Pin P32 · GPIO 32 (ADC1_CH4)</div>
            </div>
            <div class="status-badge badge-open" id="badgeAdcHydro">Raw: 0 | 0%</div>
          </div>

          <div class="input-card">
            <div class="input-info">
              <div class="input-name">Temperatura Agua DS18B20</div>
              <div class="input-pin">Pin P4 · GPIO 4 (OneWire)</div>
            </div>
            <div class="status-badge badge-open" id="badgeAdcTemp">-- °C</div>
          </div>

          <div class="input-card" style="grid-column: 1 / -1; border-color: rgba(6, 182, 212, 0.5);">
            <div class="input-info">
              <div class="input-name">🧪 Celda Electroquímica (Divisor 4:1 · 3x10k + 1x10k)</div>
              <div class="input-pin">Pin 03 · GPIO 36 (VP / ADC1_CH0)</div>
            </div>
            <div class="status-badge badge-open" id="badgeAdcAnode">Varilla: 0.000V | P36: 0.000V</div>
          </div>
        </div>
      </div>

      <!-- Needle Valve PWM Servo Calibrator -->
      <div class="card">
        <div class="card-header">
          <div class="card-title">🎛️ Calibrador Servo Válvula Aguja RO</div>
          <div class="card-badge">Pin P16 · GPIO 16 (PWM)</div>
        </div>

        <div class="servo-box">
          <div class="servo-display">
            <span style="font-size:12px; color:var(--text-muted); font-weight:600;">Posición Angular Actuador:</span>
            <div class="servo-angle-text" id="servoAngleDisplay">90°</div>
          </div>

          <input type="range" min="0" max="180" value="90" class="servo-slider" id="servoSlider" oninput="updateServoDisplay(this.value)" onchange="sendServoAngle(this.value)">

          <div class="servo-presets">
            <button class="preset-btn" onclick="setServoPreset(0)">0°<br><span style="font-size:9px;">Cerrada</span></button>
            <button class="preset-btn" onclick="setServoPreset(45)">45°<br><span style="font-size:9px;">1/4</span></button>
            <button class="preset-btn" onclick="setServoPreset(90)">90°<br><span style="font-size:9px;">1/2</span></button>
            <button class="preset-btn" onclick="setServoPreset(135)">135°<br><span style="font-size:9px;">3/4</span></button>
            <button class="preset-btn" onclick="setServoPreset(180)">180°<br><span style="font-size:9px;">Abierta</span></button>
          </div>
        </div>
      </div>

      <!-- Manual Actuator Controls (9 Relays + Buzzer) -->
      <div class="card">
        <div class="card-header">
          <div class="card-title">⚡ Control Manual Individual de Actuadores</div>
          <div class="card-badge">9 Relés + Buzzer</div>
        </div>

        <div class="actuators-grid" id="actuatorsContainer"></div>
      </div>

    </div>

    <!-- ======================================================== -->
    <!-- TAB 3: DIAGRAMA DE PINES Y GUÍA DE CABLEADO RÁPIDO      -->
    <!-- ======================================================== -->
    <div id="tabContentPins" style="display: none;">
      
      <div class="card">
        <div class="card-header">
          <div class="card-title">📐 Asignación Física ESP32S (38 Pines)</div>
          <div class="card-badge" style="color:#a78bfa;">MAPA DE PINES</div>
        </div>

        <!-- Grupo 1: Salidas de Potencia (SSRs, MOSFETs Bixolon y Relés - ACTIVE-HIGH) -->
        <div class="pin-group">
          <div class="pin-group-title" style="color:#34d399;">⚡ Salidas de Potencia (Active-HIGH)</div>
          <div class="pin-row"><div><b>P14 (GPIO 14)</b> · Actuador 1: Bomba Evans Pozo 24V (Bixolon #2 Ch2)</div><span class="pin-badge pin-b-green">ACT 1</span></div>
          <div class="pin-row"><div><b>P27 (GPIO 27)</b> · Actuador 2: Solenoide Llenado Cono 120VAC (SSR 2)</div><span class="pin-badge pin-b-green">ACT 2</span></div>
          <div class="pin-row"><div><b>P33 (GPIO 33)</b> · Actuador 3: Fuente ATX Electrofloculación (SSR 1)</div><span class="pin-badge pin-b-yellow">ACT 3 (ATX)</span></div>
          <div class="pin-row"><div><b>P26 (GPIO 26)</b> · Actuador 4: Bomba Pecera Whirlpool 120VAC (SSR 3)</div><span class="pin-badge pin-b-green">ACT 4</span></div>
          <div class="pin-row"><div><b>P25 (GPIO 25)</b> · Actuador 5: Bomba Trasvase 12V vía Relé 30A (Bixolon #2 Ch1)</div><span class="pin-badge pin-b-green">ACT 5</span></div>
          <div class="pin-row"><div><b>P23 (GPIO 23)</b> · Actuador 6: Bomba Manifold + UV</div><span class="pin-badge pin-b-green">ACT 6</span></div>
          <div class="pin-row"><div><b>P15 (GPIO 15)</b> · Actuador 7: Solenoide Vía A Casa 12V</div><span class="pin-badge pin-b-green">ACT 7</span></div>
          <div class="pin-row"><div><b>P5 (GPIO 5)</b> · Actuador 8: Solenoide Vía B Pre-RO 12V (Bixolon #1 Ch1)</div><span class="pin-badge pin-b-green">ACT 8</span></div>
          <div class="pin-row"><div><b>P13 (GPIO 13)</b> · Actuador 9: Solenoide Bypass Lavado RO 12V (Bixolon #1 Ch2)</div><span class="pin-badge pin-b-green">ACT 9</span></div>
          <div class="pin-row"><div><b>P17 (GPIO 17)</b> · Actuador 10: Buzzer Alarma</div><span class="pin-badge pin-b-red">ACT 10</span></div>
        </div>

        <!-- Grupo 2: Entradas Digitales -->
        <div class="pin-group">
          <div class="pin-group-title" style="color:#38bdf8;">🔍 Entradas Digitales (Pull-Up a GND)</div>
          <div class="pin-row"><div><b>P21</b> · 2 Hilos Cobre Tapa Cono (Rebose)</div><span class="pin-badge pin-b-red">PARO TOTAL</span></div>
          <div class="pin-row"><div><b>P19</b> · Electrodo Nivel Alto Tótem 1</div><span class="pin-badge pin-b-cyan">PULLUP</span></div>
          <div class="pin-row"><div><b>P22</b> · Presostato Demanda RO 20L (20-40 PSI)</div><span class="pin-badge pin-b-cyan">PULLUP</span></div>
          <div class="pin-row"><div><b>P18</b> · Caudalímetro Turbina Hall (L/min)</div><span class="pin-badge pin-b-purple">ISR PULSOS</span></div>
          <div class="pin-row"><div><b>P4</b> · Sensor Temperatura DS18B20 (4.7k a 3.3V)</div><span class="pin-badge pin-b-yellow">ONEWIRE</span></div>
        </div>

        <!-- Grupo 3: Analógicas ADC1 -->
        <div class="pin-group">
          <div class="pin-group-title" style="color:#fbbf24;">📊 Entradas Analógicas ADC1 (WiFi OK)</div>
          <div class="pin-row"><div><b>Pin 03 (P36 / VP)</b> · Divisor 4:1 (3x10k + 1x10k) Voltaje Celda / Amperaje</div><span class="pin-badge pin-b-cyan">0-14V DC</span></div>
          <div class="pin-row"><div><b>Pin 05 (P34)</b> · Sensor ACS712-05B AC Primaria 120V Fuente ATX</div><span class="pin-badge pin-b-yellow">0-5A AC</span></div>
          <div class="pin-row"><div><b>Pin 06 (P35)</b> · Sensor Corriente Bombas DC (Atasco)</div><span class="pin-badge pin-b-yellow">0-10A DC</span></div>
          <div class="pin-row"><div><b>Pin 07 (P32)</b> · Transductor Presión PT1 100 PSI (Puente desde P22)</div><span class="pin-badge pin-b-yellow">0-1.10m NIVEL</span></div>
        </div>

        <!-- Grupo 4: PWM y Audio -->
        <div class="pin-group">
          <div class="pin-group-title" style="color:#f472b6;">🎛️ Control Servo y Audio</div>
          <div class="pin-row"><div><b>P16 (GPIO 16)</b> · Señal PWM Servo 180° Válvula Aguja</div><span class="pin-badge" style="background:#831843; color:#f472b6;">PWM 50Hz</span></div>
          <div class="pin-row"><div><b>P17 (GPIO 17)</b> · Actuador 10: Buzzer Alarma</div><span class="pin-badge" style="background:#831843; color:#f472b6;">BIP AUDIO</span></div>
        </div>

        <!-- ESQUEMA ELÉCTRICO TOTAL Y DISTRIBUCIÓN DE POTENCIA -->
        <div class="pin-group" style="margin-top:14px;">
          <div class="pin-group-title" style="color:#fbbf24;">⚡ Diagrama Eléctrico Total: Fuentes y Cargas</div>
          
          <div style="background:#080e1a; border:1px solid #1e293b; border-radius:8px; padding:10px; font-size:11.5px; line-height:1.6; color:#cbd5e1; margin-bottom:8px;">
            <b style="color:#f87171;">1. Red 120V AC & Breaker 20A:</b><br>
            • Entrada principal protegida con termomagnético de 20A.<br>
            • Alimenta directamente: Fuente ATX 12V (SSR 1 - P33), Solenoide Llenado Cono (SSR 2 - P27), Bomba Pecera Whirlpool (SSR 3 - P26), Fuente 24V, Fuente 12V y Fuente 5V.<br><br>

            <b style="color:#fbbf24;">2. Fuentes DC & Calibres de Cable:</b><br>
            • <b>Fuente ATX 12V DC:</b> Alimenta Ánodo Fe y Cátodo controlada por SSR 1 (P33) (Cable <b>AWG 10</b> grueso).<br>
            • <b>Fuente 24V DC:</b> Alimenta Bomba Pozo Evans vía Bixolon #2 Ch2 (P14) (Cable <b>AWG 18</b>).<br>
            • <b>Fuente 12V DC:</b> Alimenta Bomba Trasvase vía Relé 30A / Bixolon #2 Ch1 (P25) y Solenoides (P15, P5, P13).<br>
            • <b>Fuente 5V / 3A DC:</b> Alimenta pin VIN del ESP32, etapas de control y Servo (P16).<br><br>

            <b style="color:#34d399;">3. Control de Electrofloculación (SSR 1):</b><br>
            • El <b>Actuador 3 (GPIO 33 / P33)</b> dispara en <b>Active-HIGH</b> el SSR 1 que conmuta la Fuente ATX de 12V.<br>
            • Todos los actuadores operan en lógica <b>Active-HIGH</b> (HIGH = Encendido, LOW = Apagado).<br><br>

            <b style="color:#38bdf8;">4. Barra Equipotencial de Tierra (Star GND):</b><br>
            • Une en un solo punto: GND ESP32, (-) 5V, (-) 12V, (-) 24V y (-) ATX 12V al chasis.<br>
            • Evita bucles de tierra y estabiliza las lecturas del convertidor analógico ADC1.
          </div>
        </div>

        <!-- Regla de Tierras -->
        <div class="bench-intro" style="background:rgba(6,182,212,0.1); border-color:var(--cyan); color:#e0f2fe;">
          ⚡ <b>TIERRA COMÚN OBLIGATORIA:</b> Une el pin <b>GND</b> del ESP32 con el negativo (-) de la fuente de 12V y 5V para que todas las señales compartan la misma referencia de 0V.
        </div>
      </div>

    </div>

    <footer>
      Agua de Gía · Arquitectura Resiliente · Tijuana B.C. 2026<br>
      ESP32S v1.1 - 38 Pines (USB-C)
    </footer>

  </div>

  <script>
    const ACTUATORS = [
      { id: 1,  pin: 14, name: "Actuador 1: Bomba Evans Pozo 24V - Bixolon #2 Ch2", desc: "Bomba superficial de succión pozo 24V (Bixolon #2 Ch2)" },
      { id: 2,  pin: 27, name: "Actuador 2: Solenoide Llenado Cono 120VAC - SSR 2", desc: "Válvula solenoide llenado 1 BBL 120V AC (SSR 2)" },
      { id: 3,  pin: 33, name: "Actuador 3: Fuente ATX Electrofloculación - SSR 1", desc: "Fuente ATX 12V electrodos Ánodo/Cátodo (SSR 1)" },
      { id: 4,  pin: 26, name: "Actuador 4: Bomba Pecera Whirlpool 120VAC - SSR 3", desc: "Agitación Whirlpool y barrido microburbujas H2 (SSR 3)" },
      { id: 5,  pin: 25, name: "Actuador 5: Bomba Trasvase 12V vía Relé 30A - Bixolon #2 Ch1", desc: "Post-racking arm a filtros sedimentos 12V (Bixolon #2 Ch1)" },
      { id: 6,  pin: 23, name: "Actuador 6: Bomba Manifold + UV", desc: "Presurización Manifold 50 PSI y lámpara germicida UV" },
      { id: 7,  pin: 15, name: "Actuador 7: Solenoide Vía A Casa 12V", desc: "NC Llenado barril casa 60L (12V DC)" },
      { id: 8,  pin: 5,  name: "Actuador 8: Solenoide Vía B Pre-RO 12V - Bixolon #1 Ch1", desc: "NC Alimentación membrana de ósmosis 12V (Bixolon #1 Ch1)" },
      { id: 9,  pin: 13, name: "Actuador 9: Solenoide Bypass Lavado RO 12V - Bixolon #1 Ch2", desc: "NC Purga rápida / lavado RO 1/2\" 12V (Bixolon #1 Ch2)" },
      { id: 10, pin: 17, name: "Actuador 10: Buzzer Alarma", desc: "Zumbador piezoeléctrico de advertencias y fallas" }
    ];

    let currentTab = 'auto';
    let pulseTimers = {};

    function switchTab(tab) {
      currentTab = tab;
      const tabAuto = document.getElementById('tabContentAuto');
      const tabBench = document.getElementById('tabContentBench');
      const tabPins = document.getElementById('tabContentPins');
      const btnAuto = document.getElementById('btn-tab-auto');
      const btnBench = document.getElementById('btn-tab-bench');
      const btnPins = document.getElementById('btn-tab-pins');

      tabAuto.style.display = 'none';
      tabBench.style.display = 'none';
      tabPins.style.display = 'none';
      btnAuto.classList.remove('active');
      btnBench.classList.remove('active');
      btnPins.classList.remove('active');

      if (tab === 'auto') {
        tabAuto.style.display = 'block';
        btnAuto.classList.add('active');
      } else if (tab === 'bench') {
        tabBench.style.display = 'block';
        btnBench.classList.add('active');
      } else if (tab === 'pins') {
        tabPins.style.display = 'block';
        btnPins.classList.add('active');
      }
    }

    function renderActuatorCards() {
      const container = document.getElementById('actuatorsContainer');
      container.innerHTML = '';
      
      ACTUATORS.forEach(act => {
        const card = document.createElement('div');
        card.className = 'act-card';
        card.id = `actCard_${act.id}`;
        card.innerHTML = `
          <div class="act-top">
            <div class="act-title-box">
              <div class="act-title">${act.name}</div>
              <div class="act-desc">${act.desc}</div>
              <div class="act-pin-tag">GPIO ${act.pin} (P${act.pin})</div>
            </div>
            <div class="act-state-pill pill-off" id="actPill_${act.id}">OFF</div>
          </div>
          <div class="act-actions">
            <button class="btn-act btn-act-on" onclick="setRelay(${act.id}, 1)">ENCENDER</button>
            <button class="btn-act btn-act-off" onclick="setRelay(${act.id}, 0)">APAGAR</button>
            <button class="btn-act btn-act-pulse" id="btnPulse_${act.id}" onclick="triggerPulse(${act.id}, 3000)">⚡ PULSO 3s</button>
          </div>
        `;
        container.appendChild(card);
      });
    }

    function updateServoDisplay(val) {
      document.getElementById('servoAngleDisplay').innerText = val + '°';
    }

    function sendServoAngle(val) {
      updateServoDisplay(val);
      fetch(`/api/servo?angle=${val}`, { method: 'POST' });
    }

    function setServoPreset(val) {
      document.getElementById('servoSlider').value = val;
      sendServoAngle(val);
    }

    function setRelay(id, state) {
      fetch(`/api/relay?id=${id}&state=${state}`, { method: 'POST' })
        .then(() => pollStatus());
    }

    function triggerPulse(id, durationMs) {
      const btn = document.getElementById(`btnPulse_${id}`);
      if (btn) {
        btn.classList.add('pulsing');
        let remaining = Math.round(durationMs / 1000);
        btn.innerText = `⏳ ${remaining}s...`;
        
        if (pulseTimers[id]) clearInterval(pulseTimers[id]);
        
        pulseTimers[id] = setInterval(() => {
          remaining--;
          if (remaining > 0) {
            btn.innerText = `⏳ ${remaining}s...`;
          } else {
            clearInterval(pulseTimers[id]);
            btn.classList.remove('pulsing');
            btn.innerText = '⚡ PULSO 3s';
          }
        }, 1000);
      }
      fetch(`/api/pulse?id=${id}&duration=${durationMs}`, { method: 'POST' })
        .then(() => pollStatus());
    }

    function sendAutoCmd(cmd) {
      let url = `/api/auto?cmd=${cmd}`;
      if (cmd === 'start') {
        const inp = document.getElementById('inputTdsManual');
        const val = inp ? parseFloat(inp.value) : 0;
        if (!val || isNaN(val) || val < 50 || val > 8000) {
          alert('⚠️ MEDICIÓN OBLIGATORIA DE TDS:\n\nPor favor ingresa la lectura en PPM de tu medidor físico (ej. entre 50 y 8,000 ppm) en la casilla de Muestra TDS antes de iniciar el lote.');
          if (inp) {
            inp.focus();
            inp.style.borderColor = '#ef4444';
            setTimeout(() => { inp.style.borderColor = '#06b6d4'; }, 2000);
          }
          return;
        }
        url += `&tds=${val}`;
      }
      fetch(url, { method: 'POST' })
        .then(async r => {
          if (!r.ok) {
            const errTxt = await r.text();
            alert('Aviso de Seguridad: ' + errTxt);
          }
          pollStatus();
        })
        .catch(e => alert('Error de enlace al enviar comando: ' + e));
    }

    function resetEmergency() {
      fetch('/api/emergency?active=0', { method: 'POST' })
        .then(() => pollStatus());
    }

    function triggerSampleAnode() {
      fetch('/api/sample_anode', { method: 'POST' })
        .then(() => pollStatus());
    }

    function tarePt1Zero() {
      fetch('/api/pt1/cal?cmd=tare', { method: 'POST' })
        .then(r => r.json())
        .then(j => {
          alert(`¡Tara 0 cm guardada! Zero = ${(j.zero_mv || 0).toFixed(2)} mV`);
          pollStatus();
        });
    }

    function calibratePt1HeightPrompt() {
      const val = prompt('Ingresa la altura real del agua en cm (desde la punta baja del cono, máx 110 cm = 35 cm cono + 75 cm cilindro):', '35');
      if (!val) return;
      const cm = parseFloat(val);
      if (isNaN(cm) || cm < 0) return;
      fetch(`/api/pt1/cal?cmd=height&cm=${cm}`, { method: 'POST' })
        .then(r => r.json())
        .then(j => {
          alert(`¡Calibrado a ${cm} cm! Pendiente = ${(j.mv_per_cm || 0).toFixed(4)} mV/cm | Volumen = ${(j.liters || 0).toFixed(1)} L`);
          pollStatus();
        });
    }

    let lastLogCount = -1;

    function setNivelChip(txt) {
      document.getElementById('inpNivel').value = txt;
    }

    async function submitBitacora() {
      const btn = document.getElementById('btnSendBitacora');
      const vMult = document.getElementById('inpVMult').value || '0';
      const nivel = encodeURIComponent(document.getElementById('inpNivel').value || '-');
      const iAc   = document.getElementById('inpIAc').value || '0';
      const nota  = encodeURIComponent(document.getElementById('inpNota').value || '');

      btn.innerText = '⏳ ENVIANDO...';
      try {
        await fetch(`/api/bitacora_add?v_mult=${vMult}&nivel=${nivel}&i_ac=${iAc}&nota=${nota}`, { method: 'POST' });
        document.getElementById('inpNota').value = '';
        btn.innerText = '✅ ¡ENVIADO A ANTIGRAVITY!';
        setTimeout(() => { btn.innerText = '📨 ENVIAR A ANTIGRAVITY + GUARDAR'; }, 1800);
        await refreshBitacoraRows();
        await pollStatus();
      } catch (e) {
        btn.innerText = '⚠️ ERROR AL ENVIAR';
        setTimeout(() => { btn.innerText = '📨 ENVIAR A ANTIGRAVITY + GUARDAR'; }, 2000);
      }
    }

    async function jumpToStage(st) {
      const names = [
        "0: Standby", "1: Llenado Cono", "2: Faraday",
        "3: Electrofloculación", "4: Reposo 45m",
        "5: Trasvase/Filtrado", "6: Presurización Manifold", "7: Ósmosis Inversa"
      ];
      if (!confirm(`¿Saltar directamente a la Etapa ${names[st] || st}?`)) return;
      try {
        const res = await fetch(`/api/auto?cmd=set_stage&stage=${st}`, { method: 'POST' });
        if (res.ok) {
          pollStatus();
        } else {
          alert('Error al saltar de etapa: ' + res.status);
        }
      } catch (e) {
        alert('Error de conexión: ' + e);
      }
    }

    async function refreshBitacoraRows() {
      try {
        const res = await fetch('/api/bitacora');
        if (!res.ok) return;
        const b = await res.json();
        const box = document.getElementById('bitacoraListBox');
        if (!b.rows || b.rows.length === 0) {
          box.innerHTML = 'Sin lecturas registradas aún en esta corrida.';
          return;
        }
        let html = '';
        for (let i = b.rows.length - 1; i >= 0; i--) {
          const r = b.rows[i];
          const etapaStr = r.etapa ? `<span style="background:#1e293b; color:#38bdf8; padding:1px 5px; border-radius:4px; font-weight:700;">E${r.etapa_id || 0}: ${r.etapa}</span> ` : '';
          const iDcStr = (r.i_dc !== undefined && r.i_dc > 0.05) ? ` | <span style="color:#4ade80;">I_DC: ${r.i_dc.toFixed(1)}A</span>` : '';
          const iAcStr = (r.i_ac_prim !== undefined && r.i_ac_prim > 0.05) ? ` (AC: ${r.i_ac_prim.toFixed(2)}A)` : '';
          const nivelStr = (r.nivel_cm !== undefined && r.nivel_cm > 0.1) ? ` | Nivel: ${r.nivel_cm.toFixed(1)}cm (${(r.litros || 0).toFixed(0)}L)` : (r.nivel ? ` [${r.nivel}]` : '');
          html += `<div style="padding:5px 0; border-bottom:1px solid #1e293b;">
            <b style="color:#38bdf8;">#${r.id}</b> ${etapaStr} · <b>Celda: ${(r.v_rod || 0).toFixed(2)}V</b> (P36: ${(r.v_pin || 0).toFixed(2)}V)${iDcStr}${iAcStr}${nivelStr} | ATX: ${r.ssr1_on ? 'ON' : 'OFF'}
            ${r.v_mult > 0.01 ? `<br><span style="color:#94a3b8;">Multímetro Físico: ${r.v_mult.toFixed(3)}V</span>` : ''}
            ${r.nota ? `<br><span style="color:#fde68a;">💬 "${r.nota}"</span>` : ''}
          </div>`;
        }
        box.innerHTML = html;
      } catch (e) {}
    }

    let consecutiveFails = 0;

    async function pollStatus() {
      const tStart = performance.now();
      try {
        const res = await fetch('/api/status', { cache: 'no-store' });
        const lagMs = Math.round(performance.now() - tStart);
        if (!res.ok) throw new Error('HTTP ' + res.status);
        const d = await res.json();
        consecutiveFails = 0;

        // Actualizar Indicador de Conexión y Lag Time en Vivo
        const netDot = document.getElementById('net-dot');
        const apLabel = document.getElementById('ap-label');
        const netPill = document.getElementById('net-pill');

        let redNombre = "AP Local";
        let ipActiva = (d.wifi && d.wifi.ap_ip) ? d.wifi.ap_ip : "192.168.4.1";
        if (d.wifi && d.wifi.sta_connected && d.wifi.sta_ip && d.wifi.sta_ip !== "0.0.0.0") {
          redNombre = d.wifi.sta_ssid || "Wi-Fi";
          ipActiva = d.wifi.sta_ip;
        }

        if (apLabel) {
          apLabel.innerHTML = `<span style="color:#e2e8f0;">${redNombre}: ${ipActiva}</span> · <span style="font-weight:800; color:${lagMs < 120 ? '#34d399' : (lagMs < 250 ? '#fbbf24' : '#f87171')};">⚡ ${lagMs}ms</span>`;
        }
        if (netDot) {
          netDot.style.background = lagMs < 150 ? '#10b981' : (lagMs < 300 ? '#f59e0b' : '#ef4444');
          netDot.style.boxShadow = `0 0 8px ${netDot.style.background}`;
        }
        if (netPill) {
          netPill.style.borderColor = 'rgba(16, 185, 129, 0.4)';
          netPill.style.background = 'rgba(6, 78, 59, 0.25)';
        }

        const banner = document.getElementById('emergencyBanner');
        if (d.emergency) {
          banner.classList.add('show');
          document.getElementById('emergencyReason').innerText = d.emergencyReason || "Corte preventivo activado";
        } else {
          banner.classList.remove('show');
        }

        for (let i = 0; i <= 7; i++) {
          const stepEl = document.getElementById(`step${i}`);
          if (stepEl) {
            stepEl.classList.remove('active', 'done');
            if (i < d.stage) stepEl.classList.add('done');
            else if (i === d.stage) stepEl.classList.add('active');
          }
        }
        document.getElementById('autoModeBadge').innerText = d.mode === 'AUTO' ? 'CICLO AUTOMÁTICO' : 'MANUAL / BANCO';

        const i40A = d.inputs.adc_i_electrodes.current_A.toFixed(1);
        document.getElementById('valCurrent40A').innerHTML = `${i40A}<span class="telem-unit">A</span>`;
        document.getElementById('barCurrent40A').style.width = Math.min(100, (i40A / 40.0) * 100) + '%';

        const feDiss = d.faraday.fe_dissolved_g.toFixed(2);
        const feTarg = d.faraday.fe_target_g.toFixed(2);
        const fePct = d.faraday.pct.toFixed(0);
        document.getElementById('valFeMass').innerHTML = `${feDiss}<span class="telem-unit">/ ${feTarg}g</span>`;
        document.getElementById('valFaradayPct').innerText = `${fePct}%`;
        document.getElementById('barFaraday').style.width = Math.min(100, fePct) + '%';

        const lpm = d.inputs.flow_hall.lpm.toFixed(1);
        document.getElementById('valFlowLpm').innerHTML = `${lpm}<span class="telem-unit">L/m</span>`;
        document.getElementById('valTotalLiters').innerText = d.inputs.flow_hall.total_liters.toFixed(1);
        document.getElementById('barFlow').style.width = Math.min(100, (lpm / 20.0) * 100) + '%';

        const iDC = d.inputs.adc_i_dc_pumps.current_A.toFixed(1);
        document.getElementById('valCurrentDC').innerHTML = `${iDC}<span class="telem-unit">A</span>`;
        document.getElementById('barCurrentDC').style.width = Math.min(100, (iDC / 10.0) * 100) + '%';
        if (iDC > 7.0) {
          document.getElementById('labelPumpJam').innerText = '⚠️ Sobrecorriente Bomba';
          document.getElementById('labelPumpJam').style.color = 'var(--red)';
        } else {
          document.getElementById('labelPumpJam').innerText = 'Carga Nominal';
          document.getElementById('labelPumpJam').style.color = '#64748b';
        }

        const hydObj = d.inputs.adc_hydrostatic || {};
        const hydroPct = (hydObj.level_pct || 0).toFixed(1);
        const hydroCm  = (hydObj.height_cm || 0).toFixed(1);
        const hydroL   = (hydObj.liters || 0).toFixed(1);
        const hydroMv  = (hydObj.mv || 0).toFixed(2);
        const hydroPsi = (hydObj.psi || 0).toFixed(3);
        const hydroConn = !!hydObj.connected;
        document.getElementById('valHydroLevel').innerHTML = `${hydroCm}<span class="telem-unit">cm / 110cm</span>`;
        document.getElementById('subHydroDetails').innerText = hydroConn
          ? `${hydroPct}% · ${hydroL} L · ${hydroMv} mV (${hydroPsi} PSI · P${hydObj.pin || 32})`
          : `⚠️ P32=${hydroMv} mV (Une puente P22 → P32 para leer ADC1)`;
        document.getElementById('barHydroLevel').style.width = Math.min(100, parseFloat(hydroPct)) + '%';

        const temp = d.inputs.temp_c > -50 ? d.inputs.temp_c.toFixed(1) : '--';
        document.getElementById('valTemp').innerHTML = `${temp}<span class="telem-unit">°C</span>`;
        if (temp !== '--') {
          document.getElementById('barTemp').style.width = Math.min(100, (temp / 50.0) * 100) + '%';
        }

        const mins = Math.floor(d.uptime_s / 60);
        const secs = d.uptime_s % 60;
        document.getElementById('uptimeBadge').innerText = `UP: ${mins < 10 ? '0' : ''}${mins}:${secs < 10 ? '0' : ''}${secs}`;

        const bTotem = document.getElementById('badgeTotemLevel');
        if (d.inputs.totem_level.active) {
          bTotem.className = 'status-badge badge-active-green';
          bTotem.innerText = '🟢 ¡AGUA DETECTADA!';
        } else {
          bTotem.className = 'status-badge badge-dry';
          bTotem.innerText = '⚪ SECO / ABIERTO';
        }

        const bOver = document.getElementById('badgeOverflowStop');
        if (d.inputs.overflow_stop.active) {
          bOver.className = 'status-badge badge-active-red';
          bOver.innerText = '🚨 ¡REBOSE ACTIVADO!';
        } else {
          bOver.className = 'status-badge badge-active-green';
          bOver.innerText = '🟢 DESPEJADO';
        }

        const bRo = document.getElementById('badgeRoPressure');
        if (d.inputs.ro_pressure_sw.active) {
          bRo.className = 'status-badge badge-closed';
          bRo.innerText = '🔵 DEMANDA ACTIVA';
        } else {
          bRo.className = 'status-badge badge-open';
          bRo.innerText = '⚪ SIN DEMANDA';
        }

        document.getElementById('badgeHallFlow').innerText = `${d.inputs.flow_hall.lpm.toFixed(1)} L/m (${d.inputs.flow_hall.hz || 0} Hz)`;
        document.getElementById('badgeAdc40A').innerText = `ADC: ${d.inputs.adc_i_electrodes.raw} | ${i40A} A`;
        document.getElementById('badgeAdcDC').innerText = `ADC: ${d.inputs.adc_i_dc_pumps.raw} | ${iDC} A`;
        document.getElementById('badgeAdcHydro').innerText = `P${hydObj.pin || 32}: ${hydroMv}mV | ${hydroCm}cm (${hydroL}L)`;
        document.getElementById('badgeAdcTemp').innerText = `${temp} °C`;

        if (d.inputs.passive_anode) {
          const pa = d.inputs.passive_anode;
          document.getElementById('valAnodeRod').innerHTML = `${pa.v_rod.toFixed(3)}<span class="telem-unit">V</span>`;
          document.getElementById('valAnodePin').innerText = pa.floating
            ? `Pin P36: AL AIRE (Ruido 60Hz: ${pa.noise_pp})`
            : `Pin P36: ${pa.v_pin.toFixed(3)} V (Raw: ${pa.raw})`;
          document.getElementById('valAnodeOnOff').innerHTML = `${pa.v_on.toFixed(2)} / ${pa.v_off_50ms.toFixed(2)}<span class="telem-unit">V</span>`;
          document.getElementById('valAnode1400').innerText = `Remanente 1.4s: ${pa.v_off_1400ms.toFixed(3)} V (#${pa.sample_count})`;
          document.getElementById('valAnodeIr').innerHTML = `${pa.delta_v_ir.toFixed(3)}<span class="telem-unit">V</span>`;
          document.getElementById('valAnodeStatus').innerText = pa.sampling_pause
            ? '⏳ PAUSA 1.5s (P33=OFF)...'
            : (pa.floating ? '⚪ PIN AL AIRE (SIN DIVISOR 10k)' : (pa.v_rod > 0.12 ? '🟢 ÁNODO EN CONTACTO CON AGUA' : '⚪ SECO / 0.00V CONECTADO'));
          const bAnode = document.getElementById('badgeAdcAnode');
          if (bAnode) {
            bAnode.className = (!pa.floating && pa.v_rod > 0.12) ? 'status-badge badge-active-green' : 'status-badge badge-open';
            bAnode.innerText = pa.floating
              ? `AL AIRE (0.000V)`
              : `Varilla: ${pa.v_rod.toFixed(3)}V | P36: ${pa.v_pin.toFixed(3)}V`;
          }
        }

        if (d.relays && Array.isArray(d.relays)) {
          d.relays.forEach(r => {
            const pill = document.getElementById(`actPill_${r.id}`);
            const card = document.getElementById(`actCard_${r.id}`);
            if (pill) {
              if (r.state) {
                pill.className = 'act-state-pill pill-on';
                pill.innerText = 'ON';
                if (card) card.classList.add('is-on');
              } else {
                pill.className = 'act-state-pill pill-off';
                pill.innerText = 'OFF';
                if (card) card.classList.remove('is-on');
              }
            }
          });
        }

        if (d.servo && d.servo.angle !== undefined) {
          if (!document.getElementById('servoSlider').matches(':active')) {
            document.getElementById('servoSlider').value = d.servo.angle;
            updateServoDisplay(d.servo.angle);
          }
        }

        if (d.antigravity) {
          const agBox = document.getElementById('agMessageBox');
          if (agBox && d.antigravity.msg) {
            agBox.innerText = d.antigravity.msg;
          }
          document.getElementById('agLogBadge').innerText = `REGISTROS: ${d.antigravity.log_count}`;
          if (d.antigravity.log_count !== lastLogCount) {
            lastLogCount = d.antigravity.log_count;
            refreshBitacoraRows();
          }
        }

      } catch (err) {
        consecutiveFails++;
        const netDot = document.getElementById('net-dot');
        const apLabel = document.getElementById('ap-label');
        const netPill = document.getElementById('net-pill');
        if (apLabel) {
          apLabel.innerHTML = `<b style="color:#f87171;">🔴 DESCONECTADO</b> (Intento #${consecutiveFails})`;
        }
        if (netDot) {
          netDot.style.background = '#ef4444';
          netDot.style.boxShadow = '0 0 10px #ef4444';
        }
        if (netPill) {
          netPill.style.borderColor = '#ef4444';
          netPill.style.background = 'rgba(127, 29, 29, 0.4)';
        }
        console.warn('Poll error:', err);
      }
    }

    window.addEventListener('DOMContentLoaded', () => {
      renderActuatorCards();
      pollStatus();
      setInterval(pollStatus, 500);
    });
  </script>
</body>
</html>
)rawliteral";

#endif
