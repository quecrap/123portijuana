#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Generador del Raster Estático Georreferenciado Calibrado: mapa_semaforo_tijuana.png
Calibración topográfica rigurosa:
- Exclusión total de lomas altas, crestas rocosas y mesetas (incluyendo la Loma del Soler / Mirador
  donde SEDENA perforó el túnel del Segundo Piso y el basamento es seco a >100m).
- Confinamiento estricto del color VERDE exclusivamente a los fondos de valle aluviales planos
  (Río Tijuana fondo y Arroyo Alamar lecho aluvial).
- Clasificación de zonas de transición con boleos en NARANJA.
- Clasificación de fallas InSAR, laderas de colapso y cerros rocosos en ROJO.
"""

import json
import re
import math
from PIL import Image, ImageDraw

LAT_MIN = 32.35
LAT_MAX = 32.56
LNG_MIN = -117.15
LNG_MAX = -116.75

IMG_W = 1600
IMG_H = 840

COLOR_VERDE   = (16, 185, 129, 255)   # #10B981
COLOR_NARANJA = (245, 158, 11, 255)   # #F59E0B
COLOR_ROJO    = (239, 68, 68, 255)    # #EF4444
COLOR_TRANSPARENTE = (0, 0, 0, 0)

# Focos críticos de colapso de ladera e inestabilidad estructural
FALLAS_CRITICAS = [
    ("Lomas del Rubí", 32.4975, -117.0385, 0.75),
    ("Camino Verde (Cañón de las Carretas)", 32.482, -116.998, 0.75),
    ("Sánchez Taboada (Calle Casiopea)", 32.476, -116.9855, 0.85),
    ("Cañón del Matadero / Desarenador", 32.529, -117.0985, 0.65),
    ("Fracc. Valle del Sur / Talud", 32.4882, -117.0421, 0.50),
    ("Cañón Johnson / Col. Hidalgo", 32.5125, -117.046, 0.55),
    ("Cumbres del Rubí / Tejamen", 32.492, -117.034, 0.55),
]


def haversine(lat1, lon1, lat2, lon2):
    R = 6371.0
    dlat = math.radians(lat2 - lat1)
    dlon = math.radians(lon2 - lon1)
    a = math.sin(dlat / 2)**2 + math.cos(math.radians(lat1)) * math.cos(math.radians(lat2)) * math.sin(dlon / 2)**2
    c = 2 * math.atan2(math.sqrt(a), math.sqrt(1 - a))
    return R * c


def latlng_to_pixel(lat, lng):
    x = int(((lng - LNG_MIN) / (LNG_MAX - LNG_MIN)) * IMG_W)
    y = int(((LAT_MAX - lat) / (LAT_MAX - LAT_MIN)) * IMG_H)
    return max(0, min(IMG_W - 1, x)), max(0, min(IMG_H - 1, y))


def load_master_data():
    with open(r'c:\Users\PEPITO\Downloads\123portijuana\visor_satelital_tijuana.html', 'r', encoding='utf-8') as f:
        text = f.read()

    p_match = re.search(r'const puntosAgua = (\[[\s\S]*?\]);', text)
    c_match = re.search(r'const celdasAlamar = (\[[\s\S]*?\]);', text)

    puntosAgua = json.loads(p_match.group(1))
    celdasAlamar = json.loads(c_match.group(1))

    # Pozos de control con NAF medido
    pozosControl = [
        p for p in puntosAgua 
        if p.get('hidrometria') and p['hidrometria'].get('profundidad_naf_m') is not None
    ]
    return puntosAgua, celdasAlamar, pozosControl


def clasificar_coordenada(lat, lng, celdasAlamar, pozosControl):
    """
    Clasificación calibrada contra topografía e hidrogeología real:
    - Zonas elevadas/rocosas = ROJO
    - Valles aluviales planos = VERDE
    - Zonas de transición/boleos = NARANJA
    """
    # 0. Fuera de mancha metropolitana
    if not (32.36 <= lat <= 32.555 and -117.14 <= lng <= -116.78):
        return None

    # 1. FOCOS CRÍTICOS DE FALLA GEOLÓGICA / INESTABILIDAD INSAR
    for nombre, flat, flng, r_km in FALLAS_CRITICAS:
        if haversine(lat, lng, flat, flng) <= r_km:
            return COLOR_ROJO

    # 2. CALIBRACIÓN TOPOGRÁFICA: EXCLUSIÓN DE CRESTAS Y LOMAS ALTAS
    
    # A. Loma de El Soler / El Mirador / Castillo (Zona del Túnel Subterráneo del Segundo Piso)
    # Cota 90 a 150 msnm. Estrato seco atravesado por el túnel del Viaducto Elevado.
    if 32.525 <= lat <= 32.545 and -117.090 <= lng <= -117.058:
        return COLOR_ROJO

    # B. Mesa de Otay alta (Cota 135 a 160 msnm, conglomerados cementados, NAF > 50-90m)
    if 32.528 <= lat <= 32.552 and -116.995 <= lng <= -116.920:
        return COLOR_ROJO

    # C. Cerro Colorado y estribaciones volcánicas basálticas
    if haversine(lat, lng, 32.505, -116.915) < 1.60:
        return COLOR_ROJO

    # D. Santa Fe Lomas y Cerros del Suroeste (lomas secas sin recarga)
    if 32.420 <= lat <= 32.455 and -117.060 <= lng <= -117.025:
        return COLOR_ROJO

    # E. Cimas orientales rocosas / laderas abruptas
    if lng > -116.84 and lat < 32.44:
        return COLOR_ROJO

    # F. Lomas de Sánchez Taboada / Camino Verde altas
    if 32.465 <= lat <= 32.485 and -117.005 <= lng <= -116.975:
        return COLOR_ROJO

    # 3. CELDAS OFICIALES CALIBRADAS DEL ARROYO ALAMAR (COLEF / CICESE)
    for c in celdasAlamar:
        minLat = min(c['bounds'][0][0], c['bounds'][1][0])
        maxLat = max(c['bounds'][0][0], c['bounds'][1][0])
        minLng = min(c['bounds'][0][1], c['bounds'][1][1])
        maxLng = max(c['bounds'][0][1], c['bounds'][1][1])
        if minLat <= lat <= maxLat and minLng <= lng <= maxLng:
            naf = c['depth_to_water_m']
            if naf <= 4.8:
                return COLOR_VERDE
            elif naf <= 8.5:
                return COLOR_NARANJA
            else:
                return COLOR_ROJO

    # 4. VALLES ALUVIALES PLANOS Y VERIFICADOS

    # A. Corredor del Río Tijuana (Zona Río, Chaparral, 3ra Etapa hasta Presa)
    # Fondo aluvial plano (Cota 18 a 45 msnm, freático continuo comprobado a 2.5-4.5m)
    if 32.495 <= lat <= 32.545 and -117.042 <= lng <= -116.925:
        # Franja ribereña aluvial del Río Tijuana
        dist_rio = abs(lat - (32.542 - 0.40 * (lng - (-117.040))))
        if dist_rio < 0.015:
            return COLOR_VERDE

    # B. Ribera Alamar Oriental (Vado de La Morita, Cañón del Padre, Ejido Rojo Gómez)
    # NAF somero a 3.5 - 5.5m en arenas de arroyo
    if 32.520 <= lat <= 32.540 and -116.910 <= lng <= -116.865:
        return COLOR_VERDE

    # C. Franja Costera Baja Playas de Tijuana (Malecón y terraza baja plana < 25 msnm)
    if lng < -117.100 and 32.510 <= lat <= 32.535:
        return COLOR_NARANJA  # Naranja por contacto salino / terraza marina

    # 5. ZONAS DE TRANSICIÓN ALUVIAL CON BOLEOS / PIEDRA (NARANJA)
    # Corredor Maclovio Rojas vado, Valle Redondo vado, Ojo de Agua bajo, Cañón del Sainz fondo
    if 32.450 <= lat <= 32.515 and -116.850 <= lng <= -116.780:
        return COLOR_NARANJA

    if 32.420 <= lat <= 32.445 and -116.955 <= lng <= -116.925:
        return COLOR_NARANJA

    # 6. Resto de zonas urbanas intermedias: IDW sobre pozos oficiales
    pozos_dist = sorted(
        [{'p': p, 'd': haversine(lat, lng, p['lat'], p['lng'])} for p in pozosControl],
        key=lambda x: x['d']
    )
    top3 = pozos_dist[:3]
    if top3[0]['d'] > 4.5:
        return None

    w_sum = sum(1.0 / (max(0.1, item['d'])**2) for item in top3)
    naf = sum((item['p']['hidrometria']['profundidad_naf_m'] / (max(0.1, item['d'])**2)) for item in top3) / w_sum

    if naf <= 6.0:
        return COLOR_NARANJA
    elif naf <= 15.0:
        return COLOR_NARANJA
    else:
        return COLOR_ROJO


def generar_raster_semaforo():
    puntosAgua, celdasAlamar, pozosControl = load_master_data()
    print(f"Cargados {len(pozosControl)} pozos oficiales de control")

    im = Image.new("RGBA", (IMG_W, IMG_H), COLOR_TRANSPARENTE)
    draw = ImageDraw.Draw(im)

    step_lat = 0.0035
    step_lng = 0.0042

    lat_steps = int((LAT_MAX - LAT_MIN) / step_lat) + 1
    lng_steps = int((LNG_MAX - LNG_MIN) / step_lng) + 1

    celdas_pintadas = 0

    for i in range(lat_steps):
        cell_lat1 = LAT_MIN + (i * step_lat)
        cell_lat2 = min(LAT_MAX, cell_lat1 + step_lat)
        mid_lat = (cell_lat1 + cell_lat2) / 2.0

        for j in range(lng_steps):
            cell_lng1 = LNG_MIN + (j * step_lng)
            cell_lng2 = min(LNG_MAX, cell_lng1 + step_lng)
            mid_lng = (cell_lng1 + cell_lng2) / 2.0

            color = clasificar_coordenada(mid_lat, mid_lng, celdasAlamar, pozosControl)
            if color is None:
                continue

            x1, y1 = latlng_to_pixel(cell_lat2, cell_lng1)
            x2, y2 = latlng_to_pixel(cell_lat1, cell_lng2)

            draw.rectangle([x1, y1, x2, y2], fill=color)
            celdas_pintadas += 1

    # Superponer celdas Alamar oficiales
    for c in celdasAlamar:
        minLat = min(c['bounds'][0][0], c['bounds'][1][0])
        maxLat = max(c['bounds'][0][0], c['bounds'][1][0])
        minLng = min(c['bounds'][0][1], c['bounds'][1][1])
        maxLng = max(c['bounds'][0][1], c['bounds'][1][1])

        naf = c['depth_to_water_m']
        color = COLOR_VERDE if naf <= 4.8 else (COLOR_NARANJA if naf <= 8.5 else COLOR_ROJO)

        x1, y1 = latlng_to_pixel(maxLat, minLng)
        x2, y2 = latlng_to_pixel(minLat, maxLng)
        draw.rectangle([x1, y1, x2, y2], fill=color)

    out_png = r"c:\Users\PEPITO\Downloads\123portijuana\mapa_semaforo_tijuana.png"
    im.save(out_png, "PNG", optimize=True)
    print(f"[OK] Raster recalibrado guardado en: {out_png}")
    print(f"Dimensiones: {IMG_W}x{IMG_H} px | Celdas renderizadas: {celdas_pintadas}")


if __name__ == "__main__":
    generar_raster_semaforo()
