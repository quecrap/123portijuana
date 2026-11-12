with open('visor_satelital_tijuana.html', 'r', encoding='utf-8') as f:
    html = f.read()

# 1. Quitar el bloque erróneo dentro de zonasInSAR
bloque_erroneo_insar = """                const chartId = 'chart_' + z.id.replace('-', '_');

                // Renderizado condicional de series temporales
            let seccionGrafica = '';
            if (p.tipo_evidencia !== 'PROTOTIPO_AGUA_DE_GIA') {
                if (tieneHistorial) {
                    seccionGrafica = `
                        <div style="margin-top:8px; font-size:0.72rem; font-weight:700; color:#334155;">
                            📈 Serie Temporal Instrumental Verificada (${p.historial_verificado.map(x=>x.ano).join(', ')}):
                        </div>
                        <div style="font-size:0.62rem; color:#64748b; margin-bottom:2px;">Mediciones registradas en campo (CONAGUA / Bitácora Aforo)</div>
                        <div style="position:relative; height:110px; width:100%; margin-top:2px;">
                            <canvas id="${chartPointId}"></canvas>
                        </div>
                    `;
                } else {
                    seccionGrafica = `
                        <div style="margin-top:8px; background:#fffbeb; border:1px solid #fef3c7; border-radius:5px; padding:5px 7px; font-size:0.68rem; color:#92400e; line-height:1.35;">
                            <strong>⚠️ Registro puntual o administrativo</strong><br>
                            Sin serie temporal instrumental verificada. Se descarta proyección sintética en vista técnica.
                        </div>
                    `;
                }
            }

            const popupHtml = `"""

bloque_correcto_insar = """                const chartId = 'chart_' + z.id.replace('-', '_');

                const popupHtml = `"""

if bloque_erroneo_insar in html:
    html = html.replace(bloque_erroneo_insar, bloque_correcto_insar, 1)
    print("Bloque erróneo en zonasInSAR eliminado.")
else:
    print("ALERTA: no se encontró bloque_erroneo_insar exacto.")

# 2. Insertar seccionGrafica dentro de puntosAgua.forEach
target_puntos = """            // Ficha verificada para Agua de Gía (Planta Demostrativa de Regeneración)"""

# Buscamos en puntosAgua antes de const popupHtml
marcador_popup_puntos = """                    </div>
                `;
            }

            const popupHtml = `"""

marcador_popup_puntos_reemplazo = """                    </div>
                `;
            }

            // Renderizado condicional de series temporales
            let seccionGrafica = '';
            if (p.tipo_evidencia !== 'PROTOTIPO_AGUA_DE_GIA') {
                if (tieneHistorial) {
                    seccionGrafica = `
                        <div style="margin-top:8px; font-size:0.72rem; font-weight:700; color:#334155;">
                            📈 Serie Temporal Instrumental Verificada (${p.historial_verificado.map(x=>x.ano).join(', ')}):
                        </div>
                        <div style="font-size:0.62rem; color:#64748b; margin-bottom:2px;">Mediciones registradas en campo (CONAGUA / Bitácora Aforo)</div>
                        <div style="position:relative; height:110px; width:100%; margin-top:2px;">
                            <canvas id="${chartPointId}"></canvas>
                        </div>
                    `;
                } else {
                    seccionGrafica = `
                        <div style="margin-top:8px; background:#fffbeb; border:1px solid #fef3c7; border-radius:5px; padding:5px 7px; font-size:0.68rem; color:#92400e; line-height:1.35;">
                            <strong>⚠️ Registro puntual o administrativo</strong><br>
                            Sin serie temporal instrumental verificada. Se descarta proyección sintética en vista técnica.
                        </div>
                    `;
                }
            }

            const popupHtml = `"""

if marcador_popup_puntos in html:
    html = html.replace(marcador_popup_puntos, marcador_popup_puntos_reemplazo, 1)
    print("seccionGrafica insertada correctamente en puntosAgua.forEach.")
else:
    print("ALERTA: no se encontró marcador_popup_puntos exacto.")

with open('visor_satelital_tijuana.html', 'w', encoding='utf-8') as f:
    f.write(html)

print("visor_satelital_tijuana.html procesado.")
