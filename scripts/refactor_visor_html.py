#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Fase 3: Refactorización de Capas y UI en visor_satelital_tijuana.html
"""

import os
import json
import re

BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DATA_DIR = os.path.join(BASE_DIR, 'data')
HTML_VISOR = os.path.join(BASE_DIR, 'visor_satelital_tijuana.html')
MIGRATED_JSON = os.path.join(DATA_DIR, 'puntos_agua_migrados.json')

def main():
    print("=" * 80)
    print("🛠️ FASE 3: REFACTORIZACIÓN DE CAPAS Y UI EN VISOR_SATELITAL_TIJUANA.HTML")
    print("=" * 80)

    # 1. Cargar dataset migrado
    with open(MIGRATED_JSON, 'r', encoding='utf-8') as f:
        puntos_migrados = json.load(f)
    print(f"Cargados {len(puntos_migrados)} puntos migrados de {MIGRATED_JSON}")

    # 2. Leer visor HTML actual
    with open(HTML_VISOR, 'r', encoding='utf-8') as f:
        html = f.read()

    # A. Actualizar Sidebar UI: Simbología y Notas
    sidebar_old_simb = """        <!-- Simbología de Capas Hidrogeológicas -->
        <div class="section-title">Simbología del Visor</div>
        <div class="legend-box" style="margin-bottom: 8px;">
            <div class="legend-item" style="align-items: flex-start; margin-bottom: 6px;">
                <div style="display: flex; gap: 3px; margin-top: 4px; margin-right: 8px; flex-shrink: 0;">
                    <div style="width: 8px; height: 8px; border-radius: 50%; background: #ef4444;"></div>
                    <div style="width: 8px; height: 8px; border-radius: 50%; background: var(--accent-cyan);"></div>
                    <div style="width: 8px; height: 8px; border-radius: 50%; background: #10b981;"></div>
                </div>
                <div style="font-size: 0.72rem; line-height: 1.35;">
                    <strong style="color: #fff;">Red Piezométrica (50 Puntos Verificados):</strong> Monitoreo NAF (<span style="color:#ef4444;">● ≤2m</span>, <span style="color:var(--accent-cyan);">● ≤4m</span>, <span style="color:#10b981;">● &gt;4m</span>). Con gráficas históricas al clic.
                </div>
            </div>
            <div class="legend-item" style="align-items: flex-start; margin-bottom: 6px;">
                <div style="width: 14px; height: 14px; border-radius: 50%; border: 2px solid #f59e0b; background: rgba(245,158,11,0.25); margin-top: 2px; margin-right: 8px; flex-shrink: 0;"></div>
                <div style="font-size: 0.72rem; line-height: 1.35;">
                    <strong style="color: #f59e0b;">Focos InSAR (Deformación de Terreno):</strong> Radares Sentinel-1 con medición milimétrica de subsidencia y deslizamiento.
                </div>
            </div>
            <div class="legend-item" style="align-items: flex-start;">
                <div style="width: 14px; height: 14px; border-radius: 4px; background: linear-gradient(135deg, #00E5FF, #ef4444); margin-top: 2px; margin-right: 8px; flex-shrink: 0;"></div>
                <div style="font-size: 0.72rem; line-height: 1.35;">
                    <strong style="color: #00E5FF;">Mapa de Calor de Saturación:</strong> Gradiente continuo calculado de la presión freática somera.
                </div>
            </div>
        </div>"""

    sidebar_new_simb = """        <!-- Simbología de Capas Hidrogeológicas Auditoría 2026 -->
        <div class="section-title">Simbología Hidrogeológica (Auditoría Oficial 2026)</div>
        <div class="legend-box" style="margin-bottom: 8px;">
            <div class="legend-item" style="align-items: flex-start; margin-bottom: 6px;">
                <div style="width: 10px; height: 10px; border-radius: 50%; background: #a855f7; margin-top: 3px; margin-right: 8px; flex-shrink: 0; border: 1.5px solid #fff;"></div>
                <div style="font-size: 0.72rem; line-height: 1.35;">
                    <strong style="color: #c084fc;">Red Piezométrica Oficial CONAGUA (51 Pozos):</strong> Mediciones de nivel estático oficial en Acuífero 0201. Cota topográfica de brocal milimétrica.
                </div>
            </div>
            <div class="legend-item" style="align-items: flex-start; margin-bottom: 6px;">
                <div style="width: 10px; height: 10px; border-radius: 50%; background: #10b981; margin-top: 3px; margin-right: 8px; flex-shrink: 0; border: 1.5px solid #fff;"></div>
                <div style="font-size: 0.72rem; line-height: 1.35;">
                    <strong style="color: #34d399;">Red Monitoreo Propio (AquaResiliencia):</strong> Pozo 001 Los Sauces Playas con aforo instrumental continuo, TDS, pH y marea.
                </div>
            </div>
            <div class="legend-item" style="align-items: flex-start; margin-bottom: 6px;">
                <div style="width: 10px; height: 10px; border-radius: 50%; background: #0284c7; margin-top: 3px; margin-right: 8px; flex-shrink: 0; border: 1.5px solid #fff;"></div>
                <div style="font-size: 0.72rem; line-height: 1.35;">
                    <strong style="color: #38bdf8;">Aprovechamientos REPDA (20 Concesiones):</strong> Títulos de extracción pública, industrial y agrícola. No interpolables para estático.
                </div>
            </div>
            <div class="legend-item" style="align-items: flex-start; margin-bottom: 6px;">
                <div style="width: 10px; height: 10px; border-radius: 50%; background: #eab308; margin-top: 3px; margin-right: 8px; flex-shrink: 0; border: 1.5px solid #fff;"></div>
                <div style="font-size: 0.72rem; line-height: 1.35;">
                    <strong style="color: #fde047;">Referencias Indirectas y Cárcamos:</strong> Afloramientos naturales, cárcamos de obra civil y taludes monitoreados.
                </div>
            </div>
            <div class="legend-item" style="align-items: flex-start; margin-top: 4px; padding-top: 4px; border-top: 1px dashed rgba(255,255,255,0.15);">
                <div style="width: 12px; height: 12px; border-radius: 3px; background: linear-gradient(135deg, #00E5FF, #ef4444); margin-top: 2px; margin-right: 8px; flex-shrink: 0;"></div>
                <div style="font-size: 0.70rem; line-height: 1.35; color: var(--text-muted);">
                    <strong style="color: #38bdf8;">Índice Exploratorio Heurístico:</strong> Capa de calor con pesos de referencia territorial (0.35 a 1.0). No representa presión piezométrica absoluta.
                </div>
            </div>
        </div>"""

    if sidebar_old_simb in html:
        html = html.replace(sidebar_old_simb, sidebar_new_simb)
        print("✓ Simbología en Sidebar actualizada.")
    else:
        print("⚠️ No se encontró el bloque exacto de simbología en Sidebar.")

    # B. Reemplazar la definición de capas y el control del mapa
    # Buscamos desde "const heatLayerGroup = L.layerGroup().addTo(map);" hasta "const puntosAgua = "
    idx_layers_start = html.find('const heatLayerGroup = L.layerGroup()')
    idx_puntos_start = html.find('const puntosAgua =')

    if idx_layers_start == -1 or idx_puntos_start == -1:
        print("Error: No se encontró la definición de capas de Leaflet.")
        return

    # Código JavaScript de capas refactorizado
    puntos_json_str = json.dumps(puntos_migrados, ensure_ascii=False)

    js_layers_code = f"""// Capas de datos refactorizadas - Auditoría 2026
        const heatLayerGroup = L.layerGroup();
        const zonasInSARGroup = L.layerGroup().addTo(map);
        const capaPiezometriaOficial = L.layerGroup().addTo(map);
        const capaMonitoreoPropio = L.layerGroup().addTo(map);
        const capaRepda = L.layerGroup().addTo(map);
        const capaReferenciasIndirectas = L.layerGroup().addTo(map);
        const capaNafObservado = L.layerGroup().addTo(map);
        const capaSuperficiePiezometrica = L.layerGroup();
        const sismosGroup = L.layerGroup().addTo(map);
        const usgsGroup = L.layerGroup().addTo(map);

        const overlayMaps = {{
            "🟣 Red Piezométrica Oficial (CONAGUA 0201)": capaPiezometriaOficial,
            "🟢 Red de Monitoreo Propio (AquaResiliencia)": capaMonitoreoPropio,
            "🔵 Aprovechamientos y Concesiones (REPDA)": capaRepda,
            "🟡 Referencias Indirectas, Norias y Cárcamos": capaReferenciasIndirectas,
            "💧 Nivel Freático Observado (Profundidad NAF en m)": capaNafObservado,
            "📐 Superficie Piezométrica (Elevación NE en msnm)": capaSuperficiePiezometrica,
            "🔴 Focos InSAR (Deformación y Pronóstico)": zonasInSARGroup,
            "🔥 Índice Exploratorio de Humedad y Referencias (Heurístico)": heatLayerGroup,
            "⚡ Sismicidad Reciente (USGS)": sismosGroup,
            "📡 Estación Telemetría USGS": usgsGroup
        }};

        L.control.layers(baseMaps, overlayMaps, {{ position: 'topright', collapsed: false }}).addTo(map);

        // Dataset documental estricto (94 estaciones auditadas)
        const puntosAgua = {puntos_json_str};
"""

    html = html[:idx_layers_start] + js_layers_code + html[idx_puntos_start + len('const puntosAgua = '):]

    # Ahora buscamos el bloque que genera los popups y los marcadores
    # Empieza en: "// 4. Gráficas Interactivas en TODOS los 50 Puntos de Agua Somera (Puntos Azules)"
    # y termina antes de "// 5. Sismicidad Reciente"
    idx_puntos_loop_start = html.find('// 4. Gráficas Interactivas')
    idx_sismos_start = html.find('// 5. Sismicidad Reciente')

    if idx_puntos_loop_start == -1 or idx_sismos_start == -1:
        print("Error: No se encontró el bucle de renderizado de puntos de agua.")
        return

    js_render_puntos = """// 4. Renderizado Multicapa y Fichas Técnicas con Control Documental Estricto
        puntosAgua.forEach(p => {
            const h = p.hidrometria || {};
            const f = p.fisicoquimica || {};
            const naf = h.profundidad_naf_m !== null ? h.profundidad_naf_m : 4.0;
            const tieneHistorial = p.historial_verificado && p.historial_verificado.length > 0;
            const chartPointId = 'chart_pt_' + p.id.replace(/[^a-zA-Z0-9_]/g, '_');

            // Asignación de color y estilo por tipo de evidencia
            let markerColor = '#00E5FF';
            let targetGroup = capaReferenciasIndirectas;
            let radius = 6;

            if (p.tipo_evidencia === 'MEDICION_OFICIAL') {
                markerColor = '#a855f7';
                targetGroup = capaPiezometriaOficial;
                radius = 7;
            } else if (p.tipo_evidencia === 'MONITOREO_PROPIO') {
                markerColor = '#10b981';
                targetGroup = capaMonitoreoPropio;
                radius = 8;
            } else if (p.tipo_evidencia === 'CONCESION_REPDA') {
                markerColor = '#0284c7';
                targetGroup = capaRepda;
                radius = 5.5;
            } else if (p.tipo_evidencia === 'CONTROL_DRENAJE_BOMBEO') {
                markerColor = '#f97316';
                targetGroup = capaReferenciasIndirectas;
                radius = 6;
            } else {
                markerColor = '#eab308';
                targetGroup = capaReferenciasIndirectas;
                radius = 6;
            }

            // Marcador principal en su capa respectiva
            const marker = L.circleMarker([p.lat, p.lng], {
                radius: radius,
                fillColor: markerColor,
                color: '#ffffff',
                weight: 1.5,
                opacity: 1,
                fillOpacity: 0.9
            }).addTo(targetGroup);

            // Capa 1: Nivel freático observado (Exclusivamente interpolables dentro de su unidad hidrogeológica)
            if (p.interpolable && h.profundidad_naf_m !== null) {
                const nafColor = naf <= 2.5 ? '#ef4444' : naf <= 5.0 ? '#00E5FF' : '#10b981';
                
                // Zona de influencia hidrogeológica máxima de 1,000m
                L.circle([p.lat, p.lng], {
                    radius: 1000,
                    color: nafColor,
                    weight: 1,
                    dashArray: '4, 4',
                    fillColor: nafColor,
                    fillOpacity: 0.08,
                    interactive: false
                }).addTo(capaNafObservado);

                // Marcador con etiqueta NAF
                L.circleMarker([p.lat, p.lng], {
                    radius: 4.5,
                    fillColor: nafColor,
                    color: '#ffffff',
                    weight: 1.5,
                    fillOpacity: 1
                }).addTo(capaNafObservado).bindTooltip(`NAF: ${naf.toFixed(1)}m [${p.unidad_hidro}]`, {
                    permanent: false,
                    direction: 'top',
                    className: 'leaflet-tooltip-dark'
                });
            }

            // Capa 2: Superficie piezométrica (Elevación NE en msnm con cota de brocal verificada)
            if (p.interpolable && h.elevacion_ne_msnm !== null) {
                L.circle([p.lat, p.lng], {
                    radius: 1000,
                    color: '#c084fc',
                    weight: 1,
                    dashArray: '3, 5',
                    fillColor: '#c084fc',
                    fillOpacity: 0.06,
                    interactive: false
                }).addTo(capaSuperficiePiezometrica);

                L.circleMarker([p.lat, p.lng], {
                    radius: 5,
                    fillColor: '#a855f7',
                    color: '#ffffff',
                    weight: 1.5,
                    fillOpacity: 0.95
                }).addTo(capaSuperficiePiezometrica).bindTooltip(`NE: ${h.elevacion_ne_msnm.toFixed(1)} msnm (Brocal ${h.elevacion_brocal_msnm}m)`, {
                    permanent: false,
                    direction: 'top'
                });
            }

            // Badges de Metadatos
            const badgeEvidencia = {
                'MEDICION_OFICIAL': '<span style="background:#a855f7;color:#fff;padding:2px 6px;border-radius:4px;font-size:0.62rem;font-weight:700;">🟣 MEDICIÓN OFICIAL CONAGUA</span>',
                'MONITOREO_PROPIO': '<span style="background:#10b981;color:#fff;padding:2px 6px;border-radius:4px;font-size:0.62rem;font-weight:700;">🟢 MONITOREO PROPIO CAMPO</span>',
                'CONCESION_REPDA': '<span style="background:#0284c7;color:#fff;padding:2px 6px;border-radius:4px;font-size:0.62rem;font-weight:700;">🔵 CONCESIÓN REPDA</span>',
                'CONTROL_DRENAJE_BOMBEO': '<span style="background:#f97316;color:#fff;padding:2px 6px;border-radius:4px;font-size:0.62rem;font-weight:700;">🟠 CÁRCAMO DE ACHIQUE 24/7</span>',
                'REFERENCIA_INDIRECTA': '<span style="background:#ca8a04;color:#fff;padding:2px 6px;border-radius:4px;font-size:0.62rem;font-weight:700;">🟡 REFERENCIA INDIRECTA</span>'
            }[p.tipo_evidencia] || '';

            const badgeCalidad = `<span style="background:#f1f5f9;color:#334155;border:1px solid #cbd5e1;padding:2px 5px;border-radius:4px;font-size:0.60rem;font-weight:600;">${p.calidad_dato}</span>`;
            const badgeUnidad = `<span style="background:#e0f2fe;color:#0369a1;border:1px solid #bae6fd;padding:2px 5px;border-radius:4px;font-size:0.60rem;font-weight:700;">🏷️ ${p.unidad_hidro}</span>`;

            // Sección Físico-Química e Hidráulica Ampliada (Solo para Monitoreo Propio como P13)
            let seccionFisicoquimica = '';
            if (p.tipo_evidencia === 'MONITOREO_PROPIO') {
                seccionFisicoquimica = `
                    <div style="margin-top:8px; background:#f0fdf4; border:1px solid #bbf7d0; border-radius:6px; padding:7px; font-size:0.71rem;">
                        <div style="font-weight:800; color:#166534; margin-bottom:4px; display:flex; justify-content:space-between;">
                            <span>📊 AFORO INSTRUMENTAL EN VIVO</span>
                            <span>Tren 80/20 POU</span>
                        </div>
                        <div style="display:grid; grid-template-columns:1fr 1fr; gap:3px 8px; color:#1e293b;">
                            <div>Nivel Estático: <strong>${h.profundidad_naf_m} m</strong></div>
                            <div>Nivel Dinámico: <strong>${h.nivel_dinamico_m} m</strong></div>
                            <div>Caudal Aforado: <strong>${h.caudal_lps} L/s</strong></div>
                            <div>Profundidad Pozo: <strong>${h.profundidad_total_m} m</strong></div>
                            <div>TDS POU Salida: <strong style="color:#0284c7;">${f.tds_ppm} ppm</strong></div>
                            <div>Conductividad: <strong>${f.conductividad_us_cm} µS/cm</strong></div>
                            <div>pH: <strong>${f.ph}</strong> | Temp: <strong>${f.temp_c} °C</strong></div>
                            <div>Marea: <strong>${f.estado_marea}</strong></div>
                            <div style="grid-column: span 2; border-top:1px dashed #bbf7d0; padding-top:2px; margin-top:2px;">
                                Tiempo de Bombeo: <strong>${f.tiempo_bombeo_min} min continuos</strong> | Último: <strong>${h.fecha_ultima_medicion}</strong>
                            </div>
                        </div>
                    </div>
                `;
            }

            // Renderizado condicional de series temporales
            let seccionGrafica = '';
            if (tieneHistorial) {
                seccionGrafica = `
                    <div style="margin-top:8px; font-size:0.72rem; font-weight:700; color:#334155;">
                        📈 Serie Temporal Instrumental Verificada (${p.historial_verificado.map(x=>x.ano).join(', ')}):
                    </div>
                    <div style="font-size:0.63rem; color:#64748b; margin-bottom:2px;">Mediciones reales registradas en bitácora oficial / campo</div>
                    <div style="position:relative; height:120px; width:100%; margin-top:3px;">
                        <canvas id="${chartPointId}"></canvas>
                    </div>
                `;
            } else {
                seccionGrafica = `
                    <div style="margin-top:8px; background:#fffbeb; border:1px solid #fef3c7; border-radius:5px; padding:6px 8px; font-size:0.69rem; color:#92400e; line-height:1.35;">
                        <strong>⚠️ Registro puntual o administrativo</strong><br>
                        Sin serie temporal instrumental verificada. Se descarta curva proyectada para garantizar rigor hidrogeológico.
                    </div>
                `;
            }

            // Popup HTML profesional
            const popupHtml = `
                <div style="color: #0f172a; font-family: sans-serif; width: 280px; box-sizing: border-box;">
                    <div style="display:flex; justify-content:space-between; align-items:center; margin-bottom:3px;">
                        <span style="font-size:0.72rem; font-weight:800; color:#0284c7;">[${p.id}]</span>
                        <span style="font-size:0.72rem; font-weight:800; color:${markerColor};">NAF: ${h.profundidad_naf_m !== null ? h.profundidad_naf_m + ' m' : 'No reportado'}</span>
                    </div>
                    <div style="display:flex; flex-wrap:wrap; gap:3px; margin-bottom:4px;">
                        ${badgeEvidencia}
                        ${badgeCalidad}
                        ${badgeUnidad}
                    </div>
                    <div style="font-size:0.92rem; font-weight:800; margin:3px 0 2px 0; line-height:1.25;">${p.nombre}</div>
                    
                    <div style="background:#f8fafc; border:1px solid #e2e8f0; border-radius:5px; padding:5px 7px; margin:4px 0; font-size:0.73rem; line-height:1.35;">
                        <div><strong>Profundidad Freática:</strong> <span style="color:#0284c7;font-weight:700;">${h.profundidad_naf_m !== null ? h.profundidad_naf_m + ' m' : 'Sin medición'}</span> (bajo terreno)</div>
                        <div><strong>Elevación Piezométrica:</strong> <span style="color:#a855f7;font-weight:700;">${h.elevacion_ne_msnm !== null ? h.elevacion_ne_msnm + ' msnm' : 'Pendiente nivelación'}</span></div>
                        <div><strong>Cota de Brocal:</strong> ${h.elevacion_brocal_msnm !== null ? h.elevacion_brocal_msnm + ' msnm (' + h.origen_cota_brocal + ')' : 'No nivelado'}</div>
                    </div>

                    <div style="font-size:0.71rem; color:#475569; margin:2px 0;"><strong>Fuente Oficial:</strong> ${p.fuente}</div>
                    ${p.notas_auditoria ? `<div style="font-size:0.68rem; color:#64748b; font-style:italic; margin-top:2px;">"${p.notas_auditoria}"</div>` : ''}

                    ${seccionFisicoquimica}
                    ${seccionGrafica}
                </div>
            `;

            marker.bindPopup(popupHtml, { maxWidth: 300 });

            // Inicialización de la gráfica histórica real al abrir popup
            if (tieneHistorial) {
                marker.on('popupopen', () => {
                    setTimeout(() => {
                        const ctx = document.getElementById(chartPointId);
                        if (ctx && !ctx._chartDone) {
                            ctx._chartDone = true;
                            new Chart(ctx, {
                                type: 'line',
                                data: {
                                    labels: p.historial_verificado.map(x => x.fecha || x.ano),
                                    datasets: [{
                                        label: 'Profundidad NAF (m)',
                                        data: p.historial_verificado.map(x => x.profundidad_naf_m),
                                        borderColor: '#a855f7',
                                        backgroundColor: 'rgba(168, 85, 247, 0.2)',
                                        fill: true,
                                        tension: 0.2,
                                        pointRadius: 4,
                                        pointBackgroundColor: '#a855f7'
                                    }]
                                },
                                options: {
                                    responsive: true,
                                    maintainAspectRatio: false,
                                    plugins: {
                                        legend: { display: false },
                                        tooltip: {
                                            callbacks: {
                                                label: function(c) { return c.raw + ' m bajo terreno'; }
                                            }
                                        }
                                    },
                                    scales: {
                                        x: { grid: { display: false }, ticks: { font: { size: 8 } } },
                                        y: {
                                            reverse: true,
                                            grid: { color: '#e2e8f0' },
                                            ticks: {
                                                font: { size: 8 },
                                                callback: function(v) { return v + 'm'; }
                                            }
                                        }
                                    }
                                }
                            });
                        }
                    }, 80);
                });
            }
        });

        """

    html = html[:idx_puntos_loop_start] + js_render_puntos + html[idx_sismos_start:]

    # C. Guardar visor refactorizado
    with open(HTML_VISOR, 'w', encoding='utf-8') as f:
        f.write(html)

    print(f"✓ Archivo {HTML_VISOR} refactorizado con éxito ({len(html)} bytes).")
    print("✅ FASE 3 COMPLETADA CON ÉXITO.")

if __name__ == '__main__':
    main()
