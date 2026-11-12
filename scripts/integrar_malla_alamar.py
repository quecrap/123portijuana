import json

with open('visor_satelital_tijuana.html', 'r', encoding='utf-8') as f:
    html = f.read()

with open('data/malla_drastic_alamar.json', 'r', encoding='utf-8') as f:
    celdas = json.load(f)

# 1. Insertar entrada en la simbología del sidebar
entrada_leyenda_antigua = """            <div class="legend-item" style="align-items: flex-start; margin-bottom: 6px;">
                <div style="width: 10px; height: 10px; border-radius: 50%; background: #eab308; margin-top: 3px; margin-right: 8px; flex-shrink: 0; border: 1.5px solid #fff;"></div>
                <div style="font-size: 0.72rem; line-height: 1.35;">
                    <strong style="color: #fde047;">Referencias Indirectas y Cárcamos:</strong> Afloramientos naturales, cárcamos de obra civil y taludes monitoreados.
                </div>
            </div>"""

entrada_leyenda_nueva = """            <div class="legend-item" style="align-items: flex-start; margin-bottom: 6px;">
                <div style="width: 10px; height: 10px; border-radius: 50%; background: #eab308; margin-top: 3px; margin-right: 8px; flex-shrink: 0; border: 1.5px solid #fff;"></div>
                <div style="font-size: 0.72rem; line-height: 1.35;">
                    <strong style="color: #fde047;">Referencias Indirectas y Cárcamos:</strong> Afloramientos naturales, cárcamos de obra civil y taludes monitoreados.
                </div>
            </div>
            <div class="legend-item" style="align-items: flex-start; margin-bottom: 6px;">
                <div style="width: 10px; height: 10px; border-radius: 2px; background: #00E5FF; margin-top: 3px; margin-right: 8px; flex-shrink: 0; border: 1px solid #38bdf8;"></div>
                <div style="font-size: 0.72rem; line-height: 1.35;">
                    <strong style="color: #38bdf8;">Malla Freática Alamar (DRASTIC 500m):</strong> Vulnerabilidad y profundidad freática (2 a 10 m). Estudios COLEF / CICESE / UABC.
                </div>
            </div>"""

if entrada_leyenda_antigua in html:
    html = html.replace(entrada_leyenda_antigua, entrada_leyenda_nueva, 1)
    print("Entrada de leyenda agregada.")
else:
    print("ALERTA: no se encontró entrada_leyenda_antigua.")

# 2. Declarar capaMallaAlamar
declaracion_capas_antigua = """        const sismosGroup = L.layerGroup().addTo(map);
        const usgsGroup = L.layerGroup().addTo(map);"""

declaracion_capas_nueva = """        const sismosGroup = L.layerGroup().addTo(map);
        const usgsGroup = L.layerGroup().addTo(map);
        const capaMallaAlamar = L.layerGroup().addTo(map);"""

if declaracion_capas_antigua in html:
    html = html.replace(declaracion_capas_antigua, declaracion_capas_nueva, 1)
    print("capaMallaAlamar declarada.")
else:
    print("ALERTA: no se encontró declaracion_capas_antigua.")

# 3. Agregar a overlayMaps
overlay_maps_antiguo = """        const overlayMaps = {
            "🟣 Piezometría Oficial CONAGUA (Acuífero 0201 - 48 Pozos)": capaPiezometriaOficial,
            "🟣 Piezometría CONAGUA (Acuífero 0245 Rosarito - 3 Pozos)": capaPiezometriaRosarito,
            "🟢 Planta Demostrativa Agua de Gía (Reactor 1 BBL)": capaMonitoreoPropio,
            "🔵 Concesiones REPDA (20 Títulos)": capaRepda,
            "🟡 Referencias Urbanas y Cárcamos": capaReferenciasIndirectas,
            "💧 Proximidad Visual 1 km - 2009 (48 Pozos)": capaProximidad2009,
            "💧 Proximidad Visual 1 km - 2013 (27 Pozos)": capaProximidad2013,
            "🔴 Deformación InSAR (Sentinel-1)": zonasInSARGroup,
            "🔥 Índice Heurístico de Humedad": heatLayerGroup,
            "⚡ Sismicidad Reciente (USGS)": sismosGroup,
            "📡 Testigo Telemetría USGS Nestor": usgsGroup
        };"""

overlay_maps_nuevo = """        const overlayMaps = {
            "🟦 Malla Freática Alamar (Celdas DRASTIC 500m)": capaMallaAlamar,
            "🟣 Piezometría Oficial CONAGUA (Acuífero 0201 - 48 Pozos)": capaPiezometriaOficial,
            "🟣 Piezometría CONAGUA (Acuífero 0245 Rosarito - 3 Pozos)": capaPiezometriaRosarito,
            "🟢 Planta Demostrativa Agua de Gía (Reactor 1 BBL)": capaMonitoreoPropio,
            "🔵 Concesiones REPDA (20 Títulos)": capaRepda,
            "🟡 Referencias Urbanas y Cárcamos": capaReferenciasIndirectas,
            "💧 Proximidad Visual 1 km - 2009 (48 Pozos)": capaProximidad2009,
            "💧 Proximidad Visual 1 km - 2013 (27 Pozos)": capaProximidad2013,
            "🔴 Deformación InSAR (Sentinel-1)": zonasInSARGroup,
            "🔥 Índice Heurístico de Humedad": heatLayerGroup,
            "⚡ Sismicidad Reciente (USGS)": sismosGroup,
            "📡 Testigo Telemetría USGS Nestor": usgsGroup
        };"""

if overlay_maps_antiguo in html:
    html = html.replace(overlay_maps_antiguo, overlay_maps_nuevo, 1)
    print("overlayMaps actualizado con Malla Freática Alamar.")
else:
    print("ALERTA: no se encontró overlay_maps_antiguo.")

# 4. Insertar la lógica de renderizado e interactividad de la Malla DRASTIC Alamar
codigo_render_malla = f"""
        // 7. Renderizado e Interactividad de la Malla Hidrogeológica DRASTIC Alamar (500m x 500m)
        // Fuente: Tesis Gutiérrez Anima (COLEF/CICESE) y Renovato (UABC 2014)
        const celdasAlamar = {json.dumps(celdas, ensure_ascii=False)};

        celdasAlamar.forEach(c => {{
            const rect = L.rectangle(c.bounds, {{
                color: '#38bdf8',
                weight: 1,
                fillColor: c.color,
                fillOpacity: 0.45
            }}).addTo(capaMallaAlamar);

            // Interactividad Hover: Resalte de borde blanco y opacidad aumentada
            rect.on('mouseover', function (e) {{
                this.setStyle({{
                    weight: 2.5,
                    color: '#ffffff',
                    fillOpacity: 0.70
                }});
                this.bringToFront();
            }});

            rect.on('mouseout', function (e) {{
                this.setStyle({{
                    weight: 1,
                    color: '#38bdf8',
                    fillOpacity: 0.45
                }});
            }});

            // Ficha técnica interactiva Glassmorphism al hacer clic
            const popupCard = `
                <div style="color: #0f172a; font-family: 'Inter', sans-serif; width: 280px; box-sizing: border-box;">
                    <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 4px;">
                        <span style="font-size: 0.65rem; font-weight: 800; background: ${{c.color}}22; color: ${{c.color}}; border: 1px solid ${{c.color}}; padding: 2px 6px; border-radius: 4px;">
                            MALLA DRASTIC 500m
                        </span>
                        <span style="font-size: 0.72rem; font-weight: 800; color: #0284c7;">${{c.id_celda}}</span>
                    </div>
                    <div style="font-size: 0.95rem; font-weight: 800; margin: 3px 0 2px; color: #0f172a; line-height: 1.25;">
                        Malla Hidrogeológica DRASTIC — Celda ${{c.id_celda}}
                    </div>
                    <div style="font-size: 0.68rem; color: #64748b; margin-bottom: 6px;">
                        Cuenca Arroyo Alamar (Tesis Gutiérrez Anima, COLEF / CICESE)
                    </div>

                    <div style="background: #f8fafc; border: 1px solid #e2e8f0; border-radius: 6px; padding: 6px 8px; margin-bottom: 6px; font-size: 0.73rem; line-height: 1.4;">
                        <div style="display: flex; justify-content: space-between; margin-bottom: 2px;">
                            <strong>Nivel Estático Reportado:</strong>
                            <span style="color: ${{c.color}}; font-weight: 800; font-size: 0.85rem;">${{c.depth_to_water_m.toFixed(1)}} m</span>
                        </div>
                        <div><strong>Rango:</strong> ${{c.rango_clasificacion}}</div>
                        <div><strong>Tipo de Acuífero:</strong> Aluvión Cuaternario Libre (Arroyo Alamar)</div>
                    </div>

                    <div style="background: #f0fdf4; border: 1px solid #bbf7d0; border-radius: 6px; padding: 6px 8px; margin-bottom: 6px; font-size: 0.69rem; color: #166534; line-height: 1.35;">
                        <strong>🛡️ Vulnerabilidad Intrínseca a Contaminación:</strong><br>
                        ${{c.vulnerabilidad}}
                    </div>

                    <div style="background: #fffbeb; border: 1px solid #fef3c7; border-radius: 6px; padding: 6px 8px; font-size: 0.69rem; color: #92400e; line-height: 1.35;">
                        <strong>⚠️ Impacto Antropogénico (Canalización):</strong><br>
                        ${{c.factor_abatimiento_canal}}
                    </div>
                </div>
            `;

            rect.bindPopup(popupCard, {{ maxWidth: 300, autoPan: true, autoPanPadding: [20, 80] }});
        }});
"""

punto_insercion_codigo = """        // Interacción del divisor arrastrable y botón de ocultar panel"""

if punto_insercion_codigo in html:
    html = html.replace(punto_insercion_codigo, codigo_render_malla + '\n        ' + punto_insercion_codigo, 1)
    print("Código de renderizado DRASTIC insertado.")
else:
    print("ALERTA: no se encontró punto_insercion_codigo.")

with open('visor_satelital_tijuana.html', 'w', encoding='utf-8') as f:
    f.write(html)

print("visor_satelital_tijuana.html actualizado con la capa DRASTIC Alamar.")
