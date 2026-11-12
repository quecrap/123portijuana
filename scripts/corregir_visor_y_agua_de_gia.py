import json
import re

# 1. ACTUALIZAR data/puntos_agua_migrados.json
with open('data/puntos_agua_migrados.json', 'r', encoding='utf-8') as f:
    puntos = json.load(f)

for p in puntos:
    if p.get('id') == 'P13':
        p['id_interno'] = 'AGUA_DE_GIA_PROTOTIPO_PILOTO'
        p['nom_pozo_oficial'] = 'Planta Demostrativa Agua de Gía'
        p['nombre'] = 'Planta Demostrativa Agua de Gía (Playas de Tijuana)'
        p['tipo_evidencia'] = 'PROTOTIPO_AGUA_DE_GIA'
        p['fuente'] = 'Agua de Gía — Especificación Técnica y Resultados de Banco Experimental 1 BBL'
        p['calidad_dato'] = 'VERIFICADO_BANCO_EXPERIMENTAL'
        p['interpolable'] = False
        
        # Eliminar cualquier dato ficticio de pozo
        p['hidrometria'] = {
            'elevacion_brocal_msnm': None,
            'origen_cota_brocal': 'NO_APLICA_PLANTA_POU',
            'profundidad_naf_m': None,
            'elevacion_ne_msnm': None,
            'pne_2009': None,
            'pne_2013': None,
            'nivel_dinamico_m': None,
            'profundidad_total_m': None,
            'caudal_lps': None,
            'fecha_ultima_medicion': None
        }
        
        # Mantener únicamente los datos verificados del sistema de tratamiento de Agua de Gía
        p['tecnologia_agua_de_gia'] = {
            'sistema': 'Reactor de Electrofloculación Faradaica 1 BBL (ESP32)',
            'ubicacion': 'Playas de Tijuana, B.C.',
            'tds_salobre_inicial_ppm': 3150,
            'tds_pou_salida_ppm': 820,
            'conductividad_inicial_us_cm': 4800,
            'conductividad_salida_us_cm': 1450,
            'temperatura_batch_c': 22.8,
            'floculantes_quimicos': 'CERO (100% físico-electroquímico, Ley de Faraday)',
            'fuente_datos_experimental': 'Bitácora Técnica y Especificación de Diseño: Sistema de Electrofloculación 1 BBL ESP32'
        }
        
        # Limpiar campos de pozo
        if 'fisicoquimica_experimental' in p:
            del p['fisicoquimica_experimental']
        if 'parametros_diseno_pendientes_aforo' in p:
            del p['parametros_diseno_pendientes_aforo']
            
        p['historial_verificado'] = []
        p['serie_sintetica_legacy'] = []
        p['notas_auditoria'] = (
            "CORRECCIÓN PERICIAL DOCUMENTAL: No existe pozo de extracción denominado 'Pozo 001 Los Sauces'. "
            "Este punto corresponde exclusivamente a la ubicación del Prototipo y Planta Demostrativa Agua de Gía. "
            "Se descartaron los supuestos datos de NAF y perforación de 18 m por carecer de verificación física. "
            "Los únicos parámetros certificados son los de operación del reactor de electrofloculación (TDS 3150 -> 820 ppm, 0 químicos)."
        )

with open('data/puntos_agua_migrados.json', 'w', encoding='utf-8') as f:
    json.dump(puntos, f, indent=2, ensure_ascii=False)

print("data/puntos_agua_migrados.json actualizado con éxito.")

# 2. ACTUALIZAR visor_satelital_tijuana.html
with open('visor_satelital_tijuana.html', 'r', encoding='utf-8') as f:
    html = f.read()

# A. Limpiar las líneas de arrays muertos 561 y 562
# Buscamos la asignación de puntosAgua y lo que sigue hasta const zonasInSAR
patron_puntos = r'const puntosAgua = \[[\s\S]*?\];\s*(?:\[\{[\s\S]*?\}\];?\s*)*\s*const zonasInSAR'
nuevo_bloque_puntos = f'const puntosAgua = {json.dumps(puntos, ensure_ascii=False)};\n        const zonasInSAR'

if re.search(patron_puntos, html):
    html = re.sub(patron_puntos, nuevo_bloque_puntos, html, count=1)
    print("Arrays muertos eliminados y puntosAgua reinyectado limpiamente.")
else:
    print("ALERTA: No se encontró el patrón exacto de puntosAgua.")

# B. Corregir capas base para evitar 403 Forbidden de OpenStreetMap
capas_base_antiguas = """        // Capas base HD sin marca de agua
        const sateliteHD = L.tileLayer('https://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile/{z}/{y}/{x}', {
            attribution: '&copy; Esri &copy; Maxar, Earthstar Geographics, CNES/Airbus DS, USGS',
            maxZoom: 19
        });

        const modoOscuro = L.tileLayer('https://services.arcgisonline.com/arcgis/rest/services/Canvas/World_Dark_Gray_Base/MapServer/tile/{z}/{y}/{x}', {
            attribution: '&copy; Esri, HERE, Garmin, &copy; OpenStreetMap',
            maxZoom: 16
        });

        const callesOSM = L.tileLayer('https://tile.openstreetmap.org/{z}/{x}/{y}.png', {
            attribution: '&copy; OpenStreetMap contributors',
            maxZoom: 19
        });

        sateliteHD.addTo(map);

        const baseMaps = {
            "🛰️ Satélite HD (Esri World Imagery)": sateliteHD,
            "🌑 Modo Oscuro Profesional": modoOscuro,
            "🗺️ Calles y Topografía (OSM)": callesOSM
        };"""

capas_base_nuevas = """        // Capas base HD sin bloqueo 403 (compatibles con file:/// y hosting local)
        const sateliteHD = L.tileLayer('https://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile/{z}/{y}/{x}', {
            attribution: '&copy; Esri, Maxar, Earthstar Geographics, CNES/Airbus DS, USGS',
            maxZoom: 19
        });

        const callesTopo = L.tileLayer('https://server.arcgisonline.com/ArcGIS/rest/services/World_Topo_Map/MapServer/tile/{z}/{y}/{x}', {
            attribution: '&copy; Esri, HERE, Garmin, Intermap, USGS, METI/NASA',
            maxZoom: 19
        });

        const callesVoyager = L.tileLayer('https://basemaps.cartocdn.com/rastertiles/voyager/{z}/{x}/{y}{r}.png', {
            attribution: '&copy; CARTO, &copy; OpenStreetMap contributors',
            maxZoom: 19
        });

        const modoOscuro = L.tileLayer('https://services.arcgisonline.com/arcgis/rest/services/Canvas/World_Dark_Gray_Base/MapServer/tile/{z}/{y}/{x}', {
            attribution: '&copy; Esri, HERE, Garmin',
            maxZoom: 16
        });

        sateliteHD.addTo(map);

        const baseMaps = {
            "🛰️ Satélite HD (Esri World Imagery)": sateliteHD,
            "🗺️ Calles y Topografía (Esri Topo)": callesTopo,
            "🏙️ Vialidades Limpias (CartoDB Voyager)": callesVoyager,
            "🌑 Modo Oscuro Profesional": modoOscuro
        };"""

if capas_base_antiguas in html:
    html = html.replace(capas_base_antiguas, capas_base_nuevas)
    print("Capas base reemplazadas (OpenStreetMap 403 eliminado).")
else:
    print("ALERTA: No se encontró el bloque exacto de capas base.")

# C. Corregir z-index de .timeline-container para que los popups no queden tapados
if '.timeline-container {' in html:
    html = re.sub(
        r'(\.timeline-container\s*\{[\s\S]*?z-index:\s*)1000;',
        r'\g<1>500;',
        html,
        count=1
    )
    print("z-index de .timeline-container corregido a 500.")

# D. Corregir texto del sidebar para las 48 estaciones y Agua de Gía
html = html.replace(
    '<strong style="color: #c084fc;">Red Piezométrica Oficial CONAGUA (51 Pozos):</strong> Mediciones de nivel estático oficial en Acuífero 0201. Cota topográfica de brocal milimétrica.',
    '<strong style="color: #c084fc;">Red Piezométrica CONAGUA (48 Pozos 0201 + 3 Pozos 0245):</strong> Mediciones oficiales en Acuífero Tijuana (48) y Rosarito (3). Brocales nivelados.'
)

html = html.replace(
    '<strong style="color: #34d399;">Red Monitoreo Propio (AquaResiliencia):</strong> Pozo 001 Los Sauces Playas con aforo instrumental continuo, TDS, pH y marea.',
    '<strong style="color: #34d399;">Planta Demostrativa Agua de Gía:</strong> Prototipo POU de regeneración y electrofloculación Faradaica (Reactor 1 BBL). No es pozo de extracción.'
)

# E. Actualizar el renderizado del marcador y popup para Agua de Gía
# En el forEach de puntosAgua:
html = html.replace(
    '"🟢 Red de Monitoreo Propio (AquaResiliencia Pozo 001 Los Sauces)": capaMonitoreoPropio,',
    '"🟢 Planta Demostrativa Agua de Gía (Reactor POU 1 BBL)": capaMonitoreoPropio,'
)

# Actualizar el bloque de renderizado de P13
patron_render_p13 = r'// Ficha expandida para P13 \(Separación documental rigurosa\)[\s\S]*?const popupHtml ='
nuevo_render_gia = """// Ficha verificada para Agua de Gía (Planta Demostrativa de Regeneración)
            let seccionP13 = '';
            if (p.id === 'P13' || p.tipo_evidencia === 'PROTOTIPO_AGUA_DE_GIA') {
                const tg = p.tecnologia_agua_de_gia || {};
                seccionP13 = `
                    <div style="margin-top:8px; background:#f0fdf4; border:1px solid #86efac; border-radius:6px; padding:8px; font-size:0.72rem;">
                        <div style="font-weight:800; color:#166534; margin-bottom:4px; display:flex; justify-content:space-between; align-items:center;">
                            <span>💧 PLANTA DEMOSTRATIVA AGUA DE GÍA</span>
                            <span style="background:#15803d; color:#fff; padding:1px 5px; border-radius:3px; font-size:0.60rem;">Reactor 1 BBL</span>
                        </div>
                        <div style="font-size:0.67rem; color:#14532d; margin-bottom:5px;">
                            <strong>Tecnología:</strong> ${tg.sistema || 'Electrofloculación Faradaica ESP32'}
                        </div>
                        <div style="display:grid; grid-template-columns:1fr 1fr; gap:3px 6px; color:#1e293b; font-size:0.69rem; background:#ffffff; padding:6px; border-radius:4px; border:1px solid #bbf7d0;">
                            <div>TDS Salobre Inicial: <strong style="color:#b45309;">${tg.tds_salobre_inicial_ppm || 3150} ppm</strong></div>
                            <div>TDS Agua de Gía: <strong style="color:#0284c7;">${tg.tds_pou_salida_ppm || 820} ppm</strong></div>
                            <div>Conductividad In: <strong>${tg.conductividad_inicial_us_cm || 4800} µS/cm</strong></div>
                            <div>Conductividad Out: <strong>${tg.conductividad_salida_us_cm || 1450} µS/cm</strong></div>
                            <div>Temp Batch: <strong>${tg.temperatura_batch_c || 22.8} °C</strong></div>
                            <div>Floculantes: <strong style="color:#166534;">CERO QUÍMICOS</strong></div>
                        </div>
                        <div style="margin-top:5px; font-size:0.65rem; color:#854d0e; background:#fef9c3; border:1px solid #fef08a; padding:4px 6px; border-radius:4px;">
                            ℹ️ <em>Aclaración Técnica:</em> No es un pozo de extracción subterránea ni una estación piezométrica de CONAGUA. Representa el tren de purificación y regeneración descentralizada de Agua de Gía en Playas de Tijuana.
                        </div>
                    </div>
                `;
            }

            const popupHtml ="""

if re.search(patron_render_p13, html):
    html = re.sub(patron_render_p13, nuevo_render_gia, html, count=1)
    print("Renderizado de ficha de Agua de Gía actualizado.")
else:
    print("ALERTA: No se encontró el bloque exacto de renderizado de P13.")

# F. Asegurar autoPan en bindPopup
html = html.replace(
    "marker.bindPopup(popupHtml, { maxWidth: 300 });",
    "marker.bindPopup(popupHtml, { maxWidth: 300, autoPan: true, autoPanPadding: [20, 80] });"
)

with open('visor_satelital_tijuana.html', 'w', encoding='utf-8') as f:
    f.write(html)

print("visor_satelital_tijuana.html sobrescrito y actualizado con éxito.")
