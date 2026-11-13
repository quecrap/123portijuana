#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Generador del Raster Estático Georreferenciado: mapa_semaforo_tijuana.png
Extrae la matriz hidrogeológica de visor_satelital_tijuana.html (SIN MODIFICARLO)
y produce un PNG estático con fondo transparente y cuadrícula estilo CICESE/COLEF
con exactamente 3 colores sólidos:
  - VERDE   (#10B981): Zonas Factibles Directas
  - NARANJA (#F59E0B): Zonas Factibles con Precaución (probable piedra/boleos/salino)
  - ROJO    (#EF4444): Zonas No Viables / Rocosas / Falla InSAR Crítica
"""

import json
import re
import math
from PIL import Image, ImageDraw

# Bounding Box oficial de Tijuana
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

# Focos críticos de colapso de ladera donde es estrictamente NO VIABLE
FALLAS_CRITICAS = [
    ("Lomas del Rubí", 32.4975, -117.0385, 0.65),
    ("Camino Verde (Cañón de las Carretas)", 32.482, -116.998, 0.65),
    ("Sánchez Taboada (Calle Casiopea)", 32.476, -116.9855, 0.75),
    ("Cañón del Matadero / Desarenador", 32.529, -117.0985, 0.50),
    ("Fracc. Valle del Sur / Talud", 32.4882, -117.0421, 0.45),
    ("Cañón Johnson / Col. Hidalgo", 32.5125, -117.046, 0.50),
    ("Cumbres del Rubí / Tejamen", 32.492, -117.034, 0.50),
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
    z_match = re.search(r'const zonasInSAR = (\[[\s\S]*?\]);', text)

    puntosAgua = json.loads(p_match.group(1))
    celdasAlamar = json.loads(c_match.group(1))
    zonasInSAR = json.loads(z_match.group(1))

    # Pozos de control válidos con NAF medido
    pozosControl = [
        p for p in puntosAgua 
        if p.get('hidrometria') and p['hidrometria'].get('profundidad_naf_m') is not None
    ]
    return puntosAgua, celdasAlamar, zonasInSAR, pozosControl


def clasificar_coordenada(lat, lng, celdasAlamar, pozosControl):
    """Clasifica una coordenada exclusivamente en VERDE, NARANJA o ROJO."""
    # Fuera de la mancha metropolitana de Tijuana
    if not (32.36 <= lat <= 32.555 and -117.14 <= lng <= -116.78):
        return None

    # 1. Focos críticos de falla geológica / colapso
    for nombre, flat, flng, r_km in FALLAS_CRITICAS:
        if haversine(lat, lng, flat, flng) <= r_km:
            return COLOR_ROJO

    # 2. Celdas calibradas DRASTIC Arroyo Alamar (COLEF / CICESE)
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

    # 3. Cerro Colorado (Cumbre y laderas basálticas Fm. Alisitos)
    if haversine(lat, lng, 32.505, -116.915) < 1.35:
        return COLOR_ROJO

    # Cimas orientales rocosas sin recarga
    if lng > -116.84 and lat < 32.44:
        return COLOR_ROJO

    # 4. Interpolación IDW NAF sobre pozos de control oficiales
    pozos_dist = sorted(
        [{'p': p, 'd': haversine(lat, lng, p['lat'], p['lng'])} for p in pozosControl],
        key=lambda x: x['d']
    )
    top3 = pozos_dist[:3]
    w_sum = sum(1.0 / (max(0.08, item['d'])**2) for item in top3)
    naf = sum((item['p']['hidrometria']['profundidad_naf_m'] / (max(0.08, item['d'])**2)) for item in top3) / w_sum

    # Distancia al pozo más cercano para recortar cerros inhabitados
    if top3[0]['d'] > 6.0:
        return None

    # 5. Región Costera Playas de Tijuana
    if lng < -117.095:
        if naf <= 7.0:
            # Agua somera pero en contacto costero / terrazas con boleos
            return COLOR_NARANJA
        else:
            return COLOR_ROJO

    # 6. Valles Aluviales Principales vs Terrazas con Dificultad Litológica
    # Zona Río y Alamar lecho central
    if naf <= 6.0:
        # Si está en el valle fluvial de fondo plano
        if lat >= 32.50 and -117.04 <= lng <= -116.90:
            return COLOR_VERDE
        elif naf <= 4.0:
            return COLOR_VERDE
        else:
            # Cañadas y terrazas intermedias con posible piedra bola/arcilla
            return COLOR_NARANJA
    elif naf <= 12.0:
        return COLOR_NARANJA
    else:
        return COLOR_ROJO


def generar_raster_semaforo():
    puntosAgua, celdasAlamar, zonasInSAR, pozosControl = load_master_data()
    print(f"Cargados {len(pozosControl)} pozos con NAF medido para IDW")

    im = Image.new("RGBA", (IMG_W, IMG_H), COLOR_TRANSPARENTE)
    draw = ImageDraw.Draw(im)

    step_lat = 0.0035  # ~380m resolución
    step_lng = 0.0042  # ~390m resolución

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

            # Rellenar celda sólida sin outline para pureza de píxel 100%
            draw.rectangle([x1, y1, x2, y2], fill=color)
            celdas_pintadas += 1

    # Superponer celdas Alamar con color puro
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
    print(f"[OK] Raster final guardado en: {out_png}")
    print(f"Dimensiones: {IMG_W}x{IMG_H} px | Celdas renderizadas: {celdas_pintadas}")


if __name__ == "__main__":
    generar_raster_semaforo()
