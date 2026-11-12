import json

with open('visor_satelital_tijuana.html', 'r', encoding='utf-8') as f:
    html = f.read()

# 1. Update marker group assignment
html = html.replace(
    "else if (p.tipo_evidencia === 'MONITOREO_PROPIO') {",
    "else if (p.tipo_evidencia === 'MONITOREO_PROPIO' || p.tipo_evidencia === 'PROTOTIPO_AGUA_DE_GIA') {"
)

# 2. Add badge for PROTOTIPO_AGUA_DE_GIA
html = html.replace(
    "'MONITOREO_PROPIO': '<span style=\"background:#10b981;color:#fff;padding:2px 6px;border-radius:4px;font-size:0.62rem;font-weight:700;\">🟢 MONITOREO PROPIO CAMPO</span>',",
    "'PROTOTIPO_AGUA_DE_GIA': '<span style=\"background:#10b981;color:#fff;padding:2px 6px;border-radius:4px;font-size:0.62rem;font-weight:700;\">🟢 PLANTA PILOTO AGUA DE GÍA</span>',\n                'MONITOREO_PROPIO': '<span style=\"background:#10b981;color:#fff;padding:2px 6px;border-radius:4px;font-size:0.62rem;font-weight:700;\">🟢 PROTOTIPO EXPERIMENTAL</span>',"
)

# 3. Popup header for Agua de Gía
html = html.replace(
    "<span style=\"font-size:0.72rem; font-weight:800; color:${markerColor};\">NAF: ${h.profundidad_naf_m !== null ? h.profundidad_naf_m + ' m' : 'No reportado'}</span>",
    "<span style=\"font-size:0.72rem; font-weight:800; color:${markerColor};\">${p.tipo_evidencia === 'PROTOTIPO_AGUA_DE_GIA' ? 'TECNOLOGÍA POU' : (h.profundidad_naf_m !== null ? 'NAF: ' + h.profundidad_naf_m + ' m' : 'NAF: No reportado')}</span>"
)

# 4. Conditionally hide brocal/PNE for Agua de Gía
bloque_brocal_antiguo = """                    <div style="background:#f8fafc; border:1px solid #e2e8f0; border-radius:5px; padding:5px 7px; margin:4px 0; font-size:0.72rem; line-height:1.35;">
                        <div><strong>Clave Acuífero:</strong> ${p.clave_acuifero} (${p.nombre_acuifero || 'TIJUANA'})</div>
                        <div><strong>Cota de Brocal:</strong> ${h.elevacion_brocal_msnm !== null ? h.elevacion_brocal_msnm + ' msnm (' + h.origen_cota_brocal + ')' : '<span style="color:#64748b;font-style:italic;">Pendiente nivelación geodésica</span>'}</div>
                        ${h.pne_2009 !== undefined && h.pne_2009 !== null ? `<div><strong>PNE Campaña 2009:</strong> <span style="color:#0284c7;font-weight:700;">${h.pne_2009} m</span> ${h.elevacion_ne_2009_msnm ? '(Cota NE: ' + h.elevacion_ne_2009_msnm + ' msnm)' : ''}</div>` : ''}
                        ${h.pne_2013 !== undefined && h.pne_2013 !== null ? `<div><strong>PNE Campaña 2013:</strong> <span style="color:#a855f7;font-weight:700;">${h.pne_2013} m</span> ${h.elevacion_ne_2013_msnm ? '(Cota NE: ' + h.elevacion_ne_2013_msnm + ' msnm)' : ''}</div>` : ''}
                    </div>"""

bloque_brocal_nuevo = """                    ${p.tipo_evidencia !== 'PROTOTIPO_AGUA_DE_GIA' ? `
                    <div style="background:#f8fafc; border:1px solid #e2e8f0; border-radius:5px; padding:5px 7px; margin:4px 0; font-size:0.72rem; line-height:1.35;">
                        <div><strong>Clave Acuífero:</strong> ${p.clave_acuifero} (${p.nombre_acuifero || 'TIJUANA'})</div>
                        <div><strong>Cota de Brocal:</strong> ${h.elevacion_brocal_msnm !== null ? h.elevacion_brocal_msnm + ' msnm (' + h.origen_cota_brocal + ')' : '<span style="color:#64748b;font-style:italic;">Pendiente nivelación geodésica</span>'}</div>
                        ${h.pne_2009 !== undefined && h.pne_2009 !== null ? `<div><strong>PNE Campaña 2009:</strong> <span style="color:#0284c7;font-weight:700;">${h.pne_2009} m</span> ${h.elevacion_ne_2009_msnm ? '(Cota NE: ' + h.elevacion_ne_2009_msnm + ' msnm)' : ''}</div>` : ''}
                        ${h.pne_2013 !== undefined && h.pne_2013 !== null ? `<div><strong>PNE Campaña 2013:</strong> <span style="color:#a855f7;font-weight:700;">${h.pne_2013} m</span> ${h.elevacion_ne_2013_msnm ? '(Cota NE: ' + h.elevacion_ne_2013_msnm + ' msnm)' : ''}</div>` : ''}
                    </div>` : ''}"""

if bloque_brocal_antiguo in html:
    html = html.replace(bloque_brocal_antiguo, bloque_brocal_nuevo)
    print("Bloque de brocal adaptado condicionalmente.")
else:
    print("ALERTA: no se encontro bloque_brocal_antiguo")

with open('visor_satelital_tijuana.html', 'w', encoding='utf-8') as f:
    f.write(html)

print("visor_satelital_tijuana.html actualizado con exito.")
