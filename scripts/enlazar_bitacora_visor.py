with open('visor_satelital_tijuana.html', 'r', encoding='utf-8') as f:
    html = f.read()

# 1. Agregar botón de acceso rápido en el header del sidebar
bloque_sub_header = """            <div class="sub-header">
                Fusión satelital de monitoreo piezométrico (CONAGUA / REPDA), deformación InSAR (Sentinel-1) y telemetría binacional en tiempo real.
            </div>"""

bloque_sub_header_nuevo = """            <div class="sub-header">
                Fusión satelital de monitoreo piezométrico (CONAGUA / REPDA), deformación InSAR (Sentinel-1) y telemetría binacional en tiempo real.
            </div>
            <!-- Barra de Herramientas de Ingeniería Privada -->
            <div style="display: flex; gap: 8px; margin-top: 10px;">
                <a href="bitacora_sensores_gia.html" title="Abrir Bitácora de Sensores y Muestreo OCR" style="flex: 1; background: rgba(0, 229, 255, 0.12); border: 1px solid var(--accent-cyan); color: var(--accent-cyan); padding: 6px 10px; border-radius: 8px; font-size: 0.72rem; font-weight: 700; display: flex; align-items: center; justify-content: center; gap: 6px; text-decoration: none; transition: all 0.2s ease;">
                    <span>🧪 Bitácora Sensores (OCR)</span>
                </a>
                <a href="portal/index.html" target="_blank" title="Abrir Portal Público Agua de Gía" style="background: rgba(255,255,255,0.06); border: 1px solid rgba(255,255,255,0.15); color: #cbd5e1; padding: 6px 10px; border-radius: 8px; font-size: 0.72rem; font-weight: 600; display: flex; align-items: center; justify-content: center; gap: 5px; text-decoration: none;">
                    <span>🌐 Portal Público</span>
                </a>
            </div>"""

if bloque_sub_header in html:
    html = html.replace(bloque_sub_header, bloque_sub_header_nuevo, 1)
    print("Barra de navegación privada insertada en header.")
else:
    print("ALERTA: no se encontró bloque_sub_header.")

# 2. Agregar botón de Bitácora en el popup de Agua de Gía
popup_gia_antiguo = """                        <div style="margin-top:5px; font-size:0.65rem; color:#854d0e; background:#fef9c3; border:1px solid #fef08a; padding:4px 6px; border-radius:4px;">
                            ℹ️ <em>Aclaración Técnica:</em> No es un pozo de extracción subterránea ni una estación piezométrica de CONAGUA. Representa el tren de purificación y regeneración descentralizada de Agua de Gía en Playas de Tijuana.
                        </div>"""

popup_gia_nuevo = """                        <div style="margin-top:5px; font-size:0.65rem; color:#854d0e; background:#fef9c3; border:1px solid #fef08a; padding:4px 6px; border-radius:4px;">
                            ℹ️ <em>Aclaración Técnica:</em> No es un pozo de extracción subterránea ni una estación piezométrica de CONAGUA. Representa el tren de purificación y regeneración descentralizada de Agua de Gía en Playas de Tijuana.
                        </div>
                        <a href="bitacora_sensores_gia.html" style="display:block; text-align:center; background:#10b981; color:#ffffff; font-weight:700; text-decoration:none; padding:5px 8px; border-radius:5px; font-size:0.68rem; margin-top:6px; box-shadow:0 2px 6px rgba(16,185,129,0.3);">
                            🧪 Registrar / Consultar Lecturas en Bitácora
                        </a>"""

if popup_gia_antiguo in html:
    html = html.replace(popup_gia_antiguo, popup_gia_nuevo, 1)
    print("Botón de bitácora añadido al popup de Agua de Gía.")
else:
    print("ALERTA: no se encontró popup_gia_antiguo.")

with open('visor_satelital_tijuana.html', 'w', encoding='utf-8') as f:
    f.write(html)

print("visor_satelital_tijuana.html actualizado con enlaces a Bitácora.")
