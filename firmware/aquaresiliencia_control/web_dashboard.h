/**
 * ============================================================================
 * PROYECTO: AquaResiliencia Tijuana — Sistema Automatizado de Gestión Hídrica
 * FIRMWARE: ESP32 DevKit v1 (WROOM-32)
 * ARCHIVO:  web_dashboard.h
 * ============================================================================
 * Panel Web Responsivo Embebido en Flash (PROGMEM) con WebSockets & Web OTA.
 * No requiere internet ni librerías externas (opera en campo en 192.168.4.1).
 * ============================================================================
 */

#ifndef WEB_DASHBOARD_H
#define WEB_DASHBOARD_H

#include <Arduino.h>

const char PAGE_INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>AquaResiliencia | Panel de Control Hidrológico</title>
    <script type="module" src="https://ajax.googleapis.com/ajax/libs/model-viewer/3.5.0/model-viewer.min.js"></script>
    <style>
        :root {
            --bg-base: #070b13;
            --bg-card: rgba(15, 23, 42, 0.75);
            --bg-card-hover: rgba(30, 41, 59, 0.85);
            --border: rgba(255, 255, 255, 0.08);
            --cyan: #00f0ff;
            --cyan-glow: rgba(0, 240, 255, 0.35);
            --emerald: #10b981;
            --emerald-glow: rgba(16, 185, 129, 0.3);
            --amber: #f59e0b;
            --crimson: #ef4444;
            --crimson-glow: rgba(239, 68, 68, 0.45);
            --text-main: #f8fafc;
            --text-muted: #94a3b8;
        }
        * { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; }
        body { background: var(--bg-base); color: var(--text-main); min-height: 100vh; padding: 16px; -webkit-font-smoothing: antialiased; }
        .container { max-width: 1200px; margin: 0 auto; display: flex; flex-direction: column; gap: 16px; }
        
        /* HEADER */
        header {
            background: linear-gradient(135deg, rgba(15, 23, 42, 0.9) 0%, rgba(30, 41, 59, 0.7) 100%);
            border: 1px solid var(--border);
            border-radius: 16px;
            padding: 16px 20px;
            display: flex;
            justify-content: space-between;
            align-items: center;
            flex-wrap: wrap;
            gap: 12px;
            box-shadow: 0 10px 25px rgba(0,0,0,0.5);
            backdrop-filter: blur(10px);
        }
        .brand { display: flex; align-items: center; gap: 12px; }
        .brand-icon { font-size: 28px; filter: drop-shadow(0 0 8px var(--cyan-glow)); }
        .brand h1 { font-size: 20px; font-weight: 800; letter-spacing: 1px; color: #fff; }
        .brand span { color: var(--cyan); }
        .brand p { font-size: 11px; color: var(--text-muted); text-transform: uppercase; letter-spacing: 1.5px; }
        
        .header-status { display: flex; align-items: center; gap: 10px; flex-wrap: wrap; }
        .pill {
            padding: 6px 12px;
            border-radius: 20px;
            font-size: 11px;
            font-weight: 700;
            letter-spacing: 0.5px;
            display: flex;
            align-items: center;
            gap: 6px;
            border: 1px solid var(--border);
            background: rgba(0,0,0,0.3);
        }
        .pill-dot { width: 8px; height: 8px; border-radius: 50%; }
        .pill-green .pill-dot { background: var(--emerald); box-shadow: 0 0 8px var(--emerald); }
        .pill-red .pill-dot { background: var(--crimson); box-shadow: 0 0 8px var(--crimson); }
        .pill-orange .pill-dot { background: var(--amber); box-shadow: 0 0 8px var(--amber); }

        /* ALERTA DE EMERGENCIA */
        #emergencyBanner {
            display: none;
            background: linear-gradient(90deg, #7f1d1d, #b91c1c);
            color: #fff;
            padding: 14px 20px;
            border-radius: 12px;
            align-items: center;
            justify-content: space-between;
            animation: pulseBanner 2s infinite;
            border: 1px solid #f87171;
        }
        @keyframes pulseBanner {
            0%, 100% { box-shadow: 0 0 15px rgba(239, 68, 68, 0.4); }
            50% { box-shadow: 0 0 30px rgba(239, 68, 68, 0.8); }
        }

        /* GRID PRINCIPAL */
        .grid-telemetry {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
            gap: 16px;
        }

        .card {
            background: var(--bg-card);
            border: 1px solid var(--border);
            border-radius: 14px;
            padding: 18px;
            display: flex;
            flex-direction: column;
            justify-content: space-between;
            gap: 12px;
            backdrop-filter: blur(8px);
            transition: transform 0.2s, border-color 0.2s;
        }
        .card:hover { border-color: rgba(255,255,255,0.18); transform: translateY(-2px); }
        .card-header { display: flex; justify-content: space-between; align-items: center; }
        .card-title { font-size: 13px; font-weight: 600; color: var(--text-muted); text-transform: uppercase; letter-spacing: 0.5px; }
        .card-tag { font-size: 10px; padding: 2px 8px; border-radius: 6px; background: rgba(255,255,255,0.06); color: var(--cyan); font-weight: 700; }
        
        .metric-value { font-size: 32px; font-weight: 800; color: #fff; display: flex; align-items: baseline; gap: 6px; }
        .metric-unit { font-size: 14px; font-weight: 500; color: var(--text-muted); }
        .metric-footer { font-size: 12px; display: flex; align-items: center; justify-content: space-between; }
        
        .badge {
            display: inline-flex;
            align-items: center;
            gap: 5px;
            padding: 4px 10px;
            border-radius: 8px;
            font-size: 11px;
            font-weight: 700;
        }
        .badge-ok { background: rgba(16, 185, 129, 0.15); color: var(--emerald); border: 1px solid rgba(16, 185, 129, 0.3); }
        .badge-danger { background: rgba(239, 68, 68, 0.15); color: var(--crimson); border: 1px solid rgba(239, 68, 68, 0.3); }
        .badge-warn { background: rgba(245, 158, 11, 0.15); color: var(--amber); border: 1px solid rgba(245, 158, 11, 0.3); }
        .badge-info { background: rgba(0, 240, 255, 0.15); color: var(--cyan); border: 1px solid rgba(0, 240, 255, 0.3); }

        /* BARRAS DE PROGRESO / PRESIÓN */
        .progress-bar-bg { width: 100%; height: 8px; background: rgba(255,255,255,0.06); border-radius: 4px; overflow: hidden; margin-top: 6px; }
        .progress-bar-fill { height: 100%; width: 0%; border-radius: 4px; transition: width 0.4s ease, background-color 0.3s; }
        .fill-cyan { background: linear-gradient(90deg, #0284c7, var(--cyan)); }
        .fill-green { background: linear-gradient(90deg, #059669, var(--emerald)); }
        .fill-amber { background: linear-gradient(90deg, #d97706, var(--amber)); }

        /* PANEL DE RELÉS Y CONTROL */
        .controls-section {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(320px, 1fr));
            gap: 16px;
        }

        .panel {
            background: var(--bg-card);
            border: 1px solid var(--border);
            border-radius: 14px;
            padding: 20px;
            display: flex;
            flex-direction: column;
            gap: 16px;
        }
        .panel-heading {
            display: flex;
            justify-content: space-between;
            align-items: center;
            border-bottom: 1px solid var(--border);
            padding-bottom: 12px;
        }
        .panel-heading h2 { font-size: 15px; font-weight: 700; color: #fff; letter-spacing: 0.5px; }

        /* RELAY ITEMS */
        .relay-list { display: flex; flex-direction: column; gap: 12px; }
        .relay-item {
            background: rgba(255,255,255,0.02);
            border: 1px solid var(--border);
            border-radius: 10px;
            padding: 12px 14px;
            display: flex;
            justify-content: space-between;
            align-items: center;
            transition: background 0.2s;
        }
        .relay-item.active { background: rgba(16, 185, 129, 0.08); border-color: rgba(16, 185, 129, 0.3); }
        .relay-info h4 { font-size: 13px; font-weight: 600; color: #fff; }
        .relay-info p { font-size: 11px; color: var(--text-muted); }

        /* SWITCH TOGGLE */
        .switch { position: relative; display: inline-block; width: 48px; height: 26px; }
        .switch input { opacity: 0; width: 0; height: 0; }
        .slider-round {
            position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0;
            background-color: #334155; transition: .3s; border-radius: 26px;
        }
        .slider-round:before {
            position: absolute; content: ""; height: 20px; width: 20px; left: 3px; bottom: 3px;
            background-color: white; transition: .3s; border-radius: 50%;
        }
        input:checked + .slider-round { background-color: var(--emerald); box-shadow: 0 0 10px var(--emerald-glow); }
        input:checked + .slider-round:before { transform: translateX(22px); }

        /* ESTOP BOTÓN GIGANTE */
        .btn-estop {
            background: linear-gradient(135deg, #b91c1c, #ef4444);
            color: #fff;
            border: none;
            border-radius: 12px;
            padding: 14px;
            font-size: 15px;
            font-weight: 800;
            letter-spacing: 1px;
            cursor: pointer;
            box-shadow: 0 6px 20px rgba(239, 68, 68, 0.35);
            transition: transform 0.1s, box-shadow 0.2s;
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 10px;
        }
        .btn-estop:active { transform: scale(0.98); }

        /* SLIDER DE FLOCULACIÓN */
        .floc-slider-container { display: flex; flex-direction: column; gap: 14px; }
        .floc-value-display {
            display: flex;
            align-items: baseline;
            justify-content: space-between;
            background: rgba(0, 240, 255, 0.05);
            border: 1px solid rgba(0, 240, 255, 0.2);
            border-radius: 10px;
            padding: 12px 16px;
        }
        .floc-number { font-size: 28px; font-weight: 800; color: var(--cyan); }
        .floc-range-input {
            width: 100%;
            height: 10px;
            border-radius: 5px;
            background: #334155;
            outline: none;
            -webkit-appearance: none;
        }
        .floc-range-input::-webkit-slider-thumb {
            -webkit-appearance: none;
            appearance: none;
            width: 24px;
            height: 24px;
            border-radius: 50%;
            background: var(--cyan);
            cursor: pointer;
            box-shadow: 0 0 12px var(--cyan-glow);
        }
        .quick-presets { display: flex; gap: 8px; }
        .btn-preset {
            flex: 1;
            padding: 8px;
            background: rgba(255,255,255,0.05);
            border: 1px solid var(--border);
            border-radius: 8px;
            color: #fff;
            font-size: 12px;
            font-weight: 600;
            cursor: pointer;
            transition: background 0.2s;
        }
        .btn-preset:hover { background: rgba(0, 240, 255, 0.15); border-color: var(--cyan); }

        .btn-action-primary {
            background: linear-gradient(135deg, #0284c7, #00f0ff);
            color: #04101e;
            border: none;
            border-radius: 10px;
            padding: 12px;
            font-size: 13px;
            font-weight: 700;
            cursor: pointer;
            transition: opacity 0.2s;
        }
        .btn-action-danger {
            background: rgba(239, 68, 68, 0.15);
            border: 1px solid rgba(239, 68, 68, 0.4);
            color: #fca5a5;
            border-radius: 10px;
            padding: 12px;
            font-size: 13px;
            font-weight: 700;
            cursor: pointer;
        }

        /* WEB OTA UPDATE FORM */
        .ota-box {
            background: rgba(255,255,255,0.02);
            border: 1px dashed rgba(255,255,255,0.15);
            border-radius: 10px;
            padding: 14px;
            display: flex;
            flex-direction: column;
            gap: 10px;
        }
        .ota-box input[type="file"] {
            font-size: 12px;
            color: var(--text-muted);
        }
        .ota-btn {
            background: linear-gradient(135deg, #6366f1, #8b5cf6);
            color: #fff;
            border: none;
            padding: 10px;
            border-radius: 8px;
            font-weight: 700;
            font-size: 12px;
            cursor: pointer;
        }

        /* LOG TERMINAL */
        .terminal-box {
            background: #020408;
            border: 1px solid var(--border);
            border-radius: 10px;
            padding: 12px;
            font-family: monospace;
            font-size: 11px;
            color: #38bdf8;
            height: 120px;
            overflow-y: auto;
            display: flex;
            flex-direction: column;
            gap: 4px;
        }
        .log-entry { line-height: 1.4; border-bottom: 1px solid rgba(255,255,255,0.03); padding-bottom: 2px; }

        .hotspot-badge {
            background: rgba(15, 23, 42, 0.92);
            border: 1px solid var(--cyan);
            border-radius: 8px;
            color: #fff;
            padding: 4px 10px;
            font-size: 11px;
            font-weight: 700;
            box-shadow: 0 0 12px var(--cyan-glow);
            pointer-events: none;
            backdrop-filter: blur(6px);
            white-space: nowrap;
        }

        @media (max-width: 600px) {
            header { flex-direction: column; align-items: flex-start; }
            .metric-value { font-size: 26px; }
        }
    </style>
</head>
<body>
    <div class="container">
        <!-- HEADER -->
        <header>
            <div class="brand">
                <div class="brand-icon">💧</div>
                <div>
                    <h1>AQUA<span>RESILIENCIA</span></h1>
                    <p>Nodo de Control Hidráulico & Purificación</p>
                </div>
            </div>
            <div class="header-status">
                <div id="wsPill" class="pill pill-orange">
                    <span class="pill-dot"></span>
                    <span id="wsText">CONECTANDO...</span>
                </div>
                <div class="pill">
                    <span>⏱ Uptime:</span>
                    <span id="uptimeText" style="color:var(--cyan);">00:00:00</span>
                </div>
                <div class="pill">
                    <span>Modo:</span>
                    <span id="modeBadge" style="color:var(--emerald); font-weight:800;">AUTO</span>
                </div>
            </div>
        </header>

        <!-- BANNER PARO DE EMERGENCIA -->
        <div id="emergencyBanner">
            <div>
                <strong>🚨 ALERTA: PARO DE EMERGENCIA ACTIVADO</strong>
                <p style="font-size:12px; opacity:0.9;">Todos los actuadores y bombas han sido cortados por seguridad física.</p>
            </div>
            <button onclick="sendAction('emergency_reset')" class="btn-preset" style="background:#fff; color:#b91c1c; font-weight:800; padding:8px 16px;">
                REARMAR SISTEMA
            </button>
        </div>

        <!-- GEMELO DIGITAL 3D (SKETCHUP / GLTF 3D VIEWER) -->
        <div class="panel" style="padding:16px;">
            <div class="panel-heading" style="margin-bottom:12px;">
                <div style="display:flex; align-items:center; gap:10px;">
                    <span style="font-size:22px;">🌐</span>
                    <div>
                        <h2>GEMELO DIGITAL 3D — PLANTA AQUARESILIENCIA</h2>
                        <p style="font-size:11px; color:var(--text-muted);">Visualización SketchUp 360° en tiempo real con telemetría</p>
                    </div>
                </div>
                <div style="display:flex; gap:8px;">
                    <label class="btn-preset" style="background:linear-gradient(135deg, #0284c7, #00f0ff); color:#000; font-weight:800; cursor:pointer; padding:8px 16px; font-size:11px; border-radius:8px; display:inline-flex; align-items:center; gap:6px;">
                        📂 CARGAR 3D (.GLB)
                        <input type="file" id="glbFileInput" accept=".glb,.gltf" style="display:none;" onchange="loadCustomGlb(event)">
                    </label>
                </div>
            </div>
            
            <div id="modelContainer" style="position:relative; width:100%; height:440px; background:#040711; border-radius:12px; overflow:hidden; border:1px solid rgba(255,255,255,0.08); display:flex; align-items:center; justify-content:center;">
                <model-viewer id="plantViewer"
                    alt="Planta 3D AquaResiliencia"
                    camera-controls
                    auto-rotate
                    auto-rotate-delay="4000"
                    rotation-per-second="12deg"
                    shadow-intensity="1.5"
                    exposure="1.1"
                    style="width: 100%; height: 100%;">
                    
                    <!-- PINES Y ETIQUETAS FLOTANTES 3D CONECTADAS A WEBSOCKETS -->
                    <div id="markerBomba" class="hotspot-badge" slot="hotspot-bomba" data-position="-1.2 0.3 0" data-normal="0 1 0">
                        🔵 Bomba Pozo: <span id="mTagBomba">--</span>
                    </div>
                    <div id="markerTotem2" class="hotspot-badge" slot="hotspot-totem2" data-position="0.5 1.2 1.6" data-normal="0 1 0">
                        🛢️ Tótem 2: <span id="mTagTotem2">--</span>
                    </div>
                    <div id="markerOsmosis" class="hotspot-badge" slot="hotspot-osmosis" data-position="0.5 0.5 -0.1" data-normal="0 0 -1">
                        ⚙️ Ósmosis Evans: <span id="mTagOsmosis">--</span>
                    </div>
                    <div id="markerBarril" class="hotspot-badge" slot="hotspot-barril" data-position="0.5 0.6 -0.55" data-normal="0 1 0">
                        🍺 Barril Sanke: <span id="mTagBarril">--</span>
                    </div>
                </model-viewer>
                
                <!-- Overlay de Instrucción inicial -->
                <div id="modelPrompt" style="position:absolute; pointer-events:none; text-align:center; padding:22px; background:rgba(15,23,42,0.9); border-radius:12px; border:1px solid var(--border); max-width:400px;">
                    <div style="font-size:36px; margin-bottom:8px;">🏗️</div>
                    <h3 style="font-size:16px; color:#fff; margin-bottom:6px;">Gemelo Digital 3D (SketchUp)</h3>
                    <p style="font-size:12px; color:var(--text-muted); line-height:1.4;">Toca arriba en <b>"CARGAR 3D (.GLB)"</b> y elige tu archivo <code style="color:var(--cyan);">sistema binaryfile.glb</code> para interactuar con la planta en 360° con telemetría en vivo.</p>
                </div>
            </div>
        </div>

        <!-- GRID DE TELEMETRÍA (SENSORES P-01 A P-08) -->
        <div class="grid-telemetry">
            <!-- [P-01] Nivel Pozo -->
            <div class="card">
                <div class="card-header">
                    <span class="card-title">Nivel / Agua en Pozo</span>
                    <span class="card-tag">[P-01]</span>
                </div>
                <div class="metric-value">
                    <span id="p01Value">--</span>
                </div>
                <div class="metric-footer">
                    <span id="p01Badge" class="badge badge-warn">Verificando</span>
                    <span id="p01Sub" style="color:var(--text-muted); font-size:11px;">Protección anti-cavitación</span>
                </div>
            </div>

            <!-- [P-02] Presostato Ósmosis (60-80 PSI) -->
            <div class="card">
                <div class="card-header">
                    <span class="card-title">Presión Membrana Ósmosis</span>
                    <span class="card-tag">[P-02]</span>
                </div>
                <div>
                    <div class="metric-value">
                        <span id="p02Psi">0.0</span> <span class="metric-unit">PSI</span>
                    </div>
                    <div class="progress-bar-bg">
                        <div id="p02Bar" class="progress-bar-fill fill-cyan" style="width: 0%;"></div>
                    </div>
                </div>
                <div class="metric-footer">
                    <span id="p02Badge" class="badge badge-info">Rango 60-80 PSI</span>
                    <span id="p02StatusText" style="font-size:11px; color:var(--text-muted);">Espera</span>
                </div>
            </div>

            <!-- [P-05] Tótem 2 Máximo (Tanque Superior) -->
            <div class="card">
                <div class="card-header">
                    <span class="card-title">Tótem 2 (Tanque Superior)</span>
                    <span class="card-tag">[P-05]</span>
                </div>
                <div class="metric-value">
                    <span id="p05Value">--</span>
                </div>
                <div class="metric-footer">
                    <span id="p05Badge" class="badge badge-ok">Capacidad Libre</span>
                    <span style="font-size:11px; color:var(--text-muted);">Corte Llenado 100%</span>
                </div>
            </div>

            <!-- [P-06] Tótem 1 Mínimo (Tanque Inferior) -->
            <div class="card">
                <div class="card-header">
                    <span class="card-title">Tótem 1 (Tanque Inferior)</span>
                    <span class="card-tag">[P-06]</span>
                </div>
                <div class="metric-value">
                    <span id="p06Value">--</span>
                </div>
                <div class="metric-footer">
                    <span id="p06Badge" class="badge badge-ok">>= 20 Litros OK</span>
                    <span style="font-size:11px; color:var(--text-muted);">Alimentación Ósmosis</span>
                </div>
            </div>

            <!-- [P-07] Barril Presurizado Auxiliar -->
            <div class="card">
                <div class="card-header">
                    <span class="card-title">Barril Presurizado Aux.</span>
                    <span class="card-tag">[P-07]</span>
                </div>
                <div class="metric-value">
                    <span id="p07Value">--</span>
                </div>
                <div class="metric-footer">
                    <span id="p07Badge" class="badge badge-info">Presostato 60 PSI</span>
                    <span style="font-size:11px; color:var(--text-muted);">Depósito final</span>
                </div>
            </div>

            <!-- [P-08] Caudalímetro en Vivo -->
            <div class="card" style="border-color: rgba(0, 240, 255, 0.25); background: rgba(0, 240, 255, 0.03);">
                <div class="card-header">
                    <span class="card-title">Caudal en Tiempo Real</span>
                    <span class="card-tag" style="background:rgba(0, 240, 255, 0.15);">[P-08]</span>
                </div>
                <div class="metric-value" style="color:var(--cyan);">
                    <span id="p08Flow">0.00</span> <span class="metric-unit" style="color:var(--cyan);">L/min</span>
                </div>
                <div class="metric-footer">
                    <div>
                        <span style="font-size:11px; color:var(--text-muted);">Total: </span>
                        <strong id="p08Total" style="color:#fff;">0.0</strong> <span style="font-size:10px;">L</span>
                    </div>
                    <div>
                        <span style="font-size:11px; color:var(--text-muted);">Lote Floc: </span>
                        <strong id="p08Batch" style="color:var(--cyan);">0.0</strong> <span style="font-size:10px;">L</span>
                    </div>
                </div>
            </div>
        </div>

        <!-- SECCIÓN DE CONTROLES Y ACTUADORES -->
        <div class="controls-section">
            <!-- PANEL DE 4 RELÉS -->
            <div class="panel">
                <div class="panel-heading">
                    <h2>Módulo de Relevadores (SRD-05VDC)</h2>
                    <div style="display:flex; gap:6px;">
                        <button onclick="setMode('AUTO')" class="btn-preset" id="btnAuto">AUTO</button>
                        <button onclick="setMode('MANUAL')" class="btn-preset" id="btnManual">MANUAL</button>
                    </div>
                </div>

                <div class="relay-list">
                    <!-- Relé 1: Bomba de Pozo -->
                    <div class="relay-item" id="relayCard0">
                        <div class="relay-info">
                            <h4>Relé 1: Bomba de Pozo (D26)</h4>
                            <p>Carga de potencia AC/DC — Succión Pozo 001</p>
                        </div>
                        <label class="switch">
                            <input type="checkbox" id="relaySwitch0" onchange="toggleRelay(0, this.checked)">
                            <span class="slider-round"></span>
                        </label>
                    </div>

                    <!-- Relé 2: Solenoide Ósmosis -->
                    <div class="relay-item" id="relayCard1">
                        <div class="relay-info">
                            <h4>Relé 2: Válvula Ósmosis (D25)</h4>
                            <p>Alimentación de línea presurizada RO</p>
                        </div>
                        <label class="switch">
                            <input type="checkbox" id="relaySwitch1" onchange="toggleRelay(1, this.checked)">
                            <span class="slider-round"></span>
                        </label>
                    </div>

                    <!-- Relé 3: Solenoide Gravedad Floculación -->
                    <div class="relay-item" id="relayCard2">
                        <div class="relay-info">
                            <h4>Relé 3: Solenoide Gravedad (D33)</h4>
                            <p>Dosificación Electrofloculación / Sedimentación</p>
                        </div>
                        <label class="switch">
                            <input type="checkbox" id="relaySwitch2" onchange="toggleRelay(2, this.checked)">
                            <span class="slider-round"></span>
                        </label>
                    </div>

                    <!-- Relé 4: Canal Auxiliar -->
                    <div class="relay-item" id="relayCard3">
                        <div class="relay-info">
                            <h4>Relé 4: Expansión Auxiliar (D32)</h4>
                            <p>Salida de potencia libre para accesorios</p>
                        </div>
                        <label class="switch">
                            <input type="checkbox" id="relaySwitch3" onchange="toggleRelay(3, this.checked)">
                            <span class="slider-round"></span>
                        </label>
                    </div>
                </div>

                <!-- BOTÓN DE PARO DE EMERGENCIA -->
                <button class="btn-estop" onclick="sendAction('emergency_stop')">
                    🛑 PARO DE EMERGENCIA (E-STOP)
                </button>
            </div>

            <!-- PANEL DE FLOCULACIÓN / SEDIMENTACIÓN POR CAUDAL -->
            <div class="panel">
                <div class="panel-heading">
                    <h2>Floculación por Caudal / Gravedad</h2>
                    <span id="flocStatusBadge" class="badge badge-info">Listo</span>
                </div>

                <div class="floc-slider-container">
                    <div class="floc-value-display">
                        <div>
                            <span style="font-size:11px; text-transform:uppercase; color:var(--text-muted);">Volumen Objetivo de Lote:</span>
                            <div class="floc-number"><span id="flocTargetDisplay">25.0</span> <span style="font-size:16px;">Litros</span></div>
                        </div>
                        <div style="text-align:right;">
                            <span style="font-size:11px; color:var(--text-muted);">Progreso de Lote:</span>
                            <div style="font-size:18px; font-weight:700; color:#fff;" id="flocProgText">0.0 %</div>
                        </div>
                    </div>

                    <!-- SLIDER INTERACTIVO DINÁMICO -->
                    <input type="range" min="1" max="100" step="1" value="25" id="flocSlider" class="floc-range-input" oninput="updateSliderUI(this.value)" onchange="sendFlocTarget(this.value)">

                    <!-- PRESETS RÁPIDOS -->
                    <div class="quick-presets">
                        <button class="btn-preset" onclick="setSliderPreset(10)">10 L</button>
                        <button class="btn-preset" onclick="setSliderPreset(25)">25 L</button>
                        <button class="btn-preset" onclick="setSliderPreset(50)">50 L</button>
                        <button class="btn-preset" onclick="setSliderPreset(100)">100 L</button>
                    </div>

                    <!-- BARRA DE PROGRESO DE LOTE -->
                    <div class="progress-bar-bg" style="height:10px;">
                        <div id="flocProgressBar" class="progress-bar-fill fill-green" style="width: 0%;"></div>
                    </div>

                    <!-- BOTONES DE ACCIÓN DE LOTE -->
                    <div style="display:flex; gap:10px;">
                        <button class="btn-action-primary" style="flex:2;" onclick="sendAction('start_floc_batch')">▶ INICIAR LOTE DE FLOCULACIÓN</button>
                        <button class="btn-action-danger" style="flex:1;" onclick="sendAction('stop_floc_batch')">⏹ DETENER</button>
                    </div>
                </div>

                <!-- ACTUALIZACIÓN DE FIRMWARE VIA WI-FI (WEB OTA) -->
                <div class="ota-box">
                    <span style="font-size:12px; font-weight:700; color:#cbd5e1;">⚡ Actualización de Firmware vía Wi-Fi (Web OTA)</span>
                    <form id="otaForm" method="POST" action="/update" enctype="multipart/form-data">
                        <input type="file" name="update" id="firmwareFile" accept=".bin" required>
                        <button type="submit" class="ota-btn" style="margin-top:8px;">Subir Binario (.bin) & Actualizar</button>
                    </form>
                    <div id="otaProgressBox" style="display:none;">
                        <div class="progress-bar-bg"><div id="otaBar" class="progress-bar-fill fill-amber" style="width:0%;"></div></div>
                        <span id="otaStatus" style="font-size:11px; color:var(--amber);">Cargando firmware...</span>
                    </div>
                </div>
            </div>
        </div>

        <!-- TERMINAL DE EVENTOS Y ACTIVIDAD EN VIVO -->
        <div class="panel">
            <div class="panel-heading">
                <h2>Registro de Eventos del Sistema (Live Log)</h2>
                <button onclick="sendAction('reset_total_volume')" class="btn-preset" style="padding:4px 10px; font-size:11px;">Reset Litros Totales</button>
            </div>
            <div class="terminal-box" id="logTerminal">
                <div class="log-entry">[0 s] Panel web iniciado. Esperando enlace WebSocket...</div>
            </div>
        </div>
    </div>

    <!-- JAVASCRIPT WEBSOCKET & CONTROL EN TIEMPO REAL -->
    <script>
        let ws;
        let isManualMode = false;
        let reconnectInterval = 2000;

        function connectWebSocket() {
            const host = window.location.hostname || "192.168.4.1";
            const wsUrl = `ws://${host}/ws`;
            console.log("Conectando a:", wsUrl);

            ws = new WebSocket(wsUrl);

            ws.onopen = () => {
                document.getElementById("wsPill").className = "pill pill-green";
                document.getElementById("wsText").innerText = "EN LÍNEA";
                addLog("Conexión WebSocket establecida con el ESP32.");
            };

            ws.onclose = () => {
                document.getElementById("wsPill").className = "pill pill-red";
                document.getElementById("wsText").innerText = "DESCONECTADO";
                addLog("Conexión perdida. Reintentando en 2s...");
                setTimeout(connectWebSocket, reconnectInterval);
            };

            ws.onerror = (err) => {
                console.error("WS Error:", err);
            };

            ws.onmessage = (event) => {
                try {
                    const data = JSON.parse(event.data);
                    if (data.type === "telemetry") {
                        updateDashboard(data);
                    }
                } catch (e) {
                    console.error("Error parseando telemetría JSON:", e);
                }
            };
        }

        function updateDashboard(d) {
            // Uptime
            if (d.control && d.control.uptime_sec !== undefined) {
                const s = d.control.uptime_sec;
                const hrs = Math.floor(s / 3600).toString().padStart(2, '0');
                const mins = Math.floor((s % 3600) / 60).toString().padStart(2, '0');
                const secs = (s % 60).toString().padStart(2, '0');
                document.getElementById("uptimeText").innerText = `${hrs}:${mins}:${secs}`;
            }

            // Modo y Emergencia
            const mode = d.control.mode || "AUTO";
            document.getElementById("modeBadge").innerText = mode;
            isManualMode = (mode === "MANUAL");
            document.getElementById("btnAuto").style.background = (mode === "AUTO") ? "rgba(16,185,129,0.3)" : "rgba(255,255,255,0.05)";
            document.getElementById("btnManual").style.background = (mode === "MANUAL") ? "rgba(0,240,255,0.3)" : "rgba(255,255,255,0.05)";

            const isEstop = d.control.emergency_stop;
            document.getElementById("emergencyBanner").style.display = isEstop ? "flex" : "none";

            // Sensores
            const s = d.sensors;
            // [P-01] Pozo
            if (s.p01_well_water) {
                document.getElementById("p01Value").innerText = "AGUA OK";
                document.getElementById("p01Value").style.color = "var(--emerald)";
                document.getElementById("p01Badge").className = "badge badge-ok";
                document.getElementById("p01Badge").innerText = "Nivel Seguro";
            } else {
                document.getElementById("p01Value").innerText = "¡SECO!";
                document.getElementById("p01Value").style.color = "var(--crimson)";
                document.getElementById("p01Badge").className = "badge badge-danger";
                document.getElementById("p01Badge").innerText = "Corte Anti-Cavitación";
            }

            // [P-02] Presión Ósmosis
            const psi = s.p02_ro_pressure_psi.toFixed(1);
            document.getElementById("p02Psi").innerText = psi;
            const psiPct = Math.min(100, Math.max(0, (s.p02_ro_pressure_psi / 100.0) * 100));
            document.getElementById("p02Bar").style.width = psiPct + "%";
            if (s.p02_ro_pressure_psi < 60) {
                document.getElementById("p02StatusText").innerText = "Baja (< 60 PSI, Requiere arranque)";
                document.getElementById("p02Badge").className = "badge badge-warn";
            } else if (s.p02_ro_pressure_psi >= 80) {
                document.getElementById("p02StatusText").innerText = "Límite corte (>= 80 PSI)";
                document.getElementById("p02Badge").className = "badge badge-danger";
            } else {
                document.getElementById("p02StatusText").innerText = "En Rango Óptimo (60–80 PSI)";
                document.getElementById("p02Badge").className = "badge badge-ok";
            }

            // [P-05] Tótem 2
            if (s.p05_totem2_full) {
                document.getElementById("p05Value").innerText = "LLENO 100%";
                document.getElementById("p05Value").style.color = "var(--amber)";
                document.getElementById("p05Badge").className = "badge badge-warn";
                document.getElementById("p05Badge").innerText = "Corte Bomba Pozo";
            } else {
                document.getElementById("p05Value").innerText = "DISPONIBLE";
                document.getElementById("p05Value").style.color = "var(--emerald)";
                document.getElementById("p05Badge").className = "badge badge-ok";
                document.getElementById("p05Badge").innerText = "Capacidad Libre";
            }

            // [P-06] Tótem 1
            if (s.p06_totem1_min_ok) {
                document.getElementById("p06Value").innerText = ">= 20 LITROS";
                document.getElementById("p06Value").style.color = "var(--emerald)";
                document.getElementById("p06Badge").className = "badge badge-ok";
                document.getElementById("p06Badge").innerText = "Reserva Aceptable";
            } else {
                document.getElementById("p06Value").innerText = "< 20L CRÍTICO";
                document.getElementById("p06Value").style.color = "var(--crimson)";
                document.getElementById("p06Badge").className = "badge badge-danger";
                document.getElementById("p06Badge").innerText = "Corte Ósmosis";
            }

            // [P-07] Barril Auxiliar
            if (s.p07_barrel_pressure_full) {
                document.getElementById("p07Value").innerText = "60 PSI (CORTE)";
                document.getElementById("p07Value").style.color = "var(--amber)";
                document.getElementById("p07Badge").className = "badge badge-warn";
                document.getElementById("p07Badge").innerText = "Depósito Lleno";
            } else {
                document.getElementById("p07Value").innerText = "CARGANDO";
                document.getElementById("p07Value").style.color = "var(--cyan)";
                document.getElementById("p07Badge").className = "badge badge-info";
                document.getElementById("p07Badge").innerText = "Presión Normal";
            }

            // [P-08] Caudalímetro
            document.getElementById("p08Flow").innerText = s.p08_flow_lpm.toFixed(2);
            document.getElementById("p08Total").innerText = s.p08_total_liters.toFixed(1);
            document.getElementById("p08Batch").innerText = s.p08_batch_liters.toFixed(2);

            // Relés
            const r = d.relays;
            updateRelayCard(0, r.relay1_pump);
            updateRelayCard(1, r.relay2_osmosis);
            updateRelayCard(2, r.relay3_gravity);
            updateRelayCard(3, r.relay4_aux);

            // Gemelo Digital 3D - Telemetría en marcadores flotantes
            const mb = document.getElementById("mTagBomba");
            if (mb) {
                mb.innerText = r.relay1_pump ? "ACTIVA" : "OFF";
                mb.style.color = r.relay1_pump ? "var(--emerald)" : "var(--text-muted)";
            }
            const mt2 = document.getElementById("mTagTotem2");
            if (mt2) {
                mt2.innerText = s.p05_totem2_full ? "100% LLENO" : "DISPONIBLE";
                mt2.style.color = s.p05_totem2_full ? "var(--amber)" : "var(--emerald)";
            }
            const mo = document.getElementById("mTagOsmosis");
            if (mo) {
                mo.innerText = r.relay2_osmosis ? "PURIFICANDO" : "OFF";
                mo.style.color = r.relay2_osmosis ? "var(--emerald)" : "var(--text-muted)";
            }
            const mk = document.getElementById("mTagBarril");
            if (mk) {
                mk.innerText = s.p07_barrel_pressure_full ? "40 PSI (LLENO)" : "PRESURIZANDO";
                mk.style.color = s.p07_barrel_pressure_full ? "var(--amber)" : "var(--cyan)";
            }

            // Floculación slider y progreso
            if (d.control.floc_target !== undefined && !sliderBeingDragged) {
                document.getElementById("flocSlider").value = d.control.floc_target;
                document.getElementById("flocTargetDisplay").innerText = d.control.floc_target.toFixed(1);
            }
            const prog = d.control.floc_progress_pct || 0;
            document.getElementById("flocProgressBar").style.width = prog.toFixed(1) + "%";
            document.getElementById("flocProgText").innerText = prog.toFixed(1) + " %";

            if (d.control.floc_batch_active) {
                document.getElementById("flocStatusBadge").className = "badge badge-ok";
                document.getElementById("flocStatusBadge").innerText = "Lote en Curso";
            } else {
                document.getElementById("flocStatusBadge").className = "badge badge-info";
                document.getElementById("flocStatusBadge").innerText = "Listo";
            }

            // Último evento
            if (d.control.last_event) {
                updateEventLog(d.control.last_event);
            }
        }

        // Cargar archivo 3D GLB/GLTF local en model-viewer
        function loadCustomGlb(event) {
            const file = event.target.files[0];
            if (file) {
                const url = URL.createObjectURL(file);
                const viewer = document.getElementById("plantViewer");
                if (viewer) {
                    viewer.src = url;
                    const prompt = document.getElementById("modelPrompt");
                    if (prompt) prompt.style.display = "none";
                    addLog("Gemelo 3D cargado con éxito: " + file.name);
                }
            }
        }

        function updateRelayCard(idx, state) {
            const card = document.getElementById("relayCard" + idx);
            const sw = document.getElementById("relaySwitch" + idx);
            if (state) {
                card.classList.add("active");
            } else {
                card.classList.remove("active");
            }
            sw.checked = state;
        }

        function toggleRelay(idx, state) {
            sendJson({ action: "toggle_relay", relay: idx + 1, state: state });
            addLog(`Comando manual: Relé ${idx + 1} -> ${state ? 'ENCENDIDO' : 'APAGADO'}`);
        }

        function setMode(m) {
            sendJson({ action: "set_mode", mode: m });
            addLog(`Solicitado cambio de modo: ${m}`);
        }

        function sendAction(act) {
            sendJson({ action: act });
            addLog(`Acción enviada: ${act}`);
        }

        let sliderBeingDragged = false;
        function updateSliderUI(val) {
            sliderBeingDragged = true;
            document.getElementById("flocTargetDisplay").innerText = parseFloat(val).toFixed(1);
        }

        function sendFlocTarget(val) {
            sliderBeingDragged = false;
            const target = parseFloat(val);
            sendJson({ action: "set_floc_target", value: target });
            addLog(`Nuevo setpoint de floculación enviado: ${target} Litros.`);
        }

        function setSliderPreset(liters) {
            document.getElementById("flocSlider").value = liters;
            updateSliderUI(liters);
            sendFlocTarget(liters);
        }

        function sendJson(obj) {
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send(JSON.stringify(obj));
            } else {
                alert("WebSocket desconectado. Espera a que se restablezca el enlace WiFi.");
            }
        }

        let lastLoggedEvent = "";
        function updateEventLog(evt) {
            if (evt && evt !== lastLoggedEvent) {
                lastLoggedEvent = evt;
                addLog(evt);
            }
        }

        function addLog(msg) {
            const term = document.getElementById("logTerminal");
            const entry = document.createElement("div");
            entry.className = "log-entry";
            entry.innerText = msg;
            term.appendChild(entry);
            term.scrollTop = term.scrollHeight;
        }

        // Manejador del formulario OTA con XMLHttpRequest para reportar progreso
        document.getElementById("otaForm").addEventListener("submit", function(e) {
            e.preventDefault();
            const fileInput = document.getElementById("firmwareFile");
            if (!fileInput.files.length) {
                alert("Por favor selecciona un archivo .bin primero.");
                return;
            }

            const file = fileInput.files[0];
            const formData = new FormData();
            formData.append("update", file);

            const xhr = new XMLHttpRequest();
            xhr.open("POST", "/update", true);

            document.getElementById("otaProgressBox").style.display = "block";
            const bar = document.getElementById("otaBar");
            const status = document.getElementById("otaStatus");

            xhr.upload.onprogress = function(event) {
                if (event.lengthComputable) {
                    const percent = Math.round((event.loaded / event.total) * 100);
                    bar.style.width = percent + "%";
                    status.innerText = `Subiendo firmware: ${percent}%`;
                }
            };

            xhr.onload = function() {
                if (xhr.status === 200) {
                    bar.style.width = "100%";
                    status.innerText = "¡Firmware subido con éxito! Reiniciando ESP32 en 5 segundos...";
                    addLog("OTA exitoso. Reiniciando controlador...");
                    setTimeout(() => { window.location.reload(); }, 6000);
                } else {
                    status.innerText = "Error al actualizar firmware: " + xhr.responseText;
                    alert("Error en actualización OTA: " + xhr.responseText);
                }
            };

            xhr.onerror = function() {
                status.innerText = "Fallo de conexión durante OTA.";
            };

            xhr.send(formData);
        });

        // Iniciar WebSocket al cargar
        window.addEventListener("load", connectWebSocket);
    </script>
</body>
</html>
)rawliteral";

#endif // WEB_DASHBOARD_H
