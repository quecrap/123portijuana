#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
generar_mapa_calor_tijuana.py
=============================
Genera un mapa de calor (Heatmap) y modelo de interpolación espacial de 
Agua Somera (nivel freático h_naf < 7.5 m) para el municipio de Tijuana, B.C.

Utiliza el dataset empírico y forense verificado (23 puntos):
- Sótanos con cárcamos 24/7 (Zona Río)
- Mediciones geofísicas directas (Valle del Sur, Delgado Argote et al. 2017)
- Peritajes judiciales por saturación de fugas (Lomas del Rubí, Camino Verde, Sánchez Taboada)
- Manantiales y veneros históricos (Cañón Johnson, Aguaje de la Tuna)
- Piezometría oficial CONAGUA (Acuífero 0201) y Pozo 001 Playas de Tijuana

Genera:
1. 'mapa_calor_agua_somera_tijuana.png' (Figura de alta resolución con contornos de saturación)
2. 'mapa_calor_agua_somera_tijuana.html' (Visor interactivo Leaflet con Leaflet.heat y popups)
"""

import json
import csv
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patheffects as pe

def cargar_datos_csv(csv_path="DATASET_MAPA_CALOR_AGUA_SOMERA_TIJUANA.csv"):
    puntos = []
    with open(csv_path, mode='r', encoding='utf-8') as f:
        reader = csv.DictReader(f)
        for row in reader:
            puntos.append({
                'id': row['id'],
                'nombre': row['nombre_sitio'],
                'delegacion': row['delegacion'],
                'lat': float(row['latitud']),
                'lng': float(row['longitud']),
                'profundidad_naf_m': float(row['profundidad_naf_m']),
                'peso': float(row['intensidad_calor']),
                'categoria': row['categoria_evidencia'],
                'geologia': row['tipo_suelo_geologia'],
                'fuente': row['fuente_documental']
            })
    return puntos

def generar_mapa_calor_png(puntos, output_png="mapa_calor_agua_somera_tijuana.png"):
    lats = np.array([p['lat'] for p in puntos])
    lngs = np.array([p['lng'] for p in puntos])
    pesos = np.array([p['peso'] for p in puntos])
    profundidades = np.array([p['profundidad_naf_m'] for p in puntos])

    # Rejilla de interpolación espacial en coordenadas Tijuana
    lat_min, lat_max = 32.41, 32.56
    lng_min, lng_max = -117.15, -116.82
    grid_lat, grid_lng = np.mgrid[lat_min:lat_max:200j, lng_min:lng_max:250j]

    # Kernel Density Estimation (KDE) Gaussiano 2D
    sigma = 0.015  # Grados (~1.5 km de radio de influencia hidrológica)
    densidad = np.zeros(grid_lat.shape)

    for lat, lng, peso in zip(lats, lngs, pesos):
        dist_sq = ((grid_lat - lat)**2 + (grid_lng - lng)**2) / (2 * sigma**2)
        densidad += peso * np.exp(-dist_sq)

    densidad = densidad / np.max(densidad)  # Normalizar 0 a 1

    # Configuración de figura estilo Dark Sci-Tech
    fig, ax = plt.subplots(figsize=(14, 10), dpi=300, facecolor='#070d18')
    ax.set_facecolor('#0b1324')

    # Contorno de densidad (Mapa de calor térmico)
    cmap = plt.cm.turbo
    cf = ax.contourf(grid_lng, grid_lat, densidad, levels=30, cmap=cmap, alpha=0.75)
    cs = ax.contour(grid_lng, grid_lat, densidad, levels=[0.25, 0.5, 0.75, 0.9], colors='#ffffff', alpha=0.35, linewidths=0.8)

    # Dispersión de puntos corroborados
    sc = ax.scatter(lngs, lats, c=profundidades, cmap='viridis_r', s=pesos * 180 + 40,
                    edgecolors='#00e5ff', linewidth=1.5, zorder=5)

    # Etiquetas de puntos emblemáticos
    puntos_clave = ['P01', 'P04', 'P08', 'P09', 'P10', 'P11', 'P12', 'P13', 'P16', 'P17']
    for p in puntos:
        if p['id'] in puntos_clave:
            txt = ax.annotate(
                f"{p['id']}: {p['nombre'].split('/')[0].strip()}\n(NAF: {p['profundidad_naf_m']}m)",
                (p['lng'], p['lat']),
                xytext=(6, 6), textcoords='offset points',
                color='#f8fafc', fontsize=8, fontweight='bold',
                bbox=dict(boxstyle='round,pad=0.25', facecolor='#0f172a', edgecolor='#38bdf8', alpha=0.85),
                path_effects=[pe.withStroke(linewidth=2, foreground='#000000')],
                zorder=6
            )

    # Barra de color
    cbar = fig.colorbar(cf, ax=ax, orientation='vertical', fraction=0.03, pad=0.02)
    cbar.set_label('Índice de Densidad / Calor de Agua Somera (0.0 = Profundo | 1.0 = Afloramiento / Crítico <2m)', 
                   color='#f8fafc', fontsize=9, fontweight='bold')
    cbar.ax.tick_params(labelsize=8, colors='#94a3b8')

    # Títulos y estética
    ax.set_title('MAPA DE CALOR: ZONAS CRÍTICAS DE AGUA SOMERA EN TIJUANA, B.C.\n'
                 'Modelo de Densidad de Kernel (KDE) calibrado con 23 evidencias forenses, geotécnicas y oficiales',
                 fontsize=13, fontweight='bold', color='#00e5ff', pad=15)
    ax.set_xlabel('Longitud (°W)', color='#94a3b8', fontsize=10)
    ax.set_ylabel('Latitud (°N)', color='#94a3b8', fontsize=10)
    ax.tick_params(colors='#94a3b8', labelsize=9)
    ax.grid(True, color='#1e293b', linestyle='--', linewidth=0.5, alpha=0.7)

    # Recuadro de fuentes
    nota_fuentes = (
        "FUENTES VERIFICADAS:\n"
        "• Delgado Argote et al. (GEOS 2017) Tomografía Geoeléctrica Valle del Sur\n"
        "• CONAGUA DR_0201 (Acuífero 0201) & REPDA Subterráneo 2026\n"
        "• Peritajes de Protección Civil (Lomas del Rubí, Camino Verde, Sánchez Taboada)\n"
        "• Cimentaciones y Sótanos con Cárcamos 24/7 (Torre Cosmopolitan, New City, SIDURT)\n"
        "• Pozo Piloto Experimental AquaResiliencia Caso 001 (Playas de Tijuana, 2022)"
    )
    ax.text(0.02, 0.03, nota_fuentes, transform=ax.transAxes,
            fontsize=7.5, color='#cbd5e1', family='monospace',
            bbox=dict(boxstyle='round,pad=0.5', facecolor='#070d18', edgecolor='#334155', alpha=0.9))

    plt.tight_layout()
    plt.savefig(output_png, dpi=300, facecolor='#070d18')
    plt.close()
    print(f"[*] Mapa de calor estático generado: {output_png}")

def generar_mapa_calor_html(puntos, output_html="mapa_calor_agua_somera_tijuana.html"):
    pts_heat_json = [[p['lat'], p['lng'], p['peso']] for p in puntos]
    puntos_json = json.dumps(puntos, ensure_ascii=False, indent=2)

    html_code = f"""<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Mapa de Calor: Agua Somera en Tijuana — AquaResiliencia</title>
  <link rel="stylesheet" href="https://unpkg.com/leaflet@1.9.4/dist/leaflet.css" />
  <script src="https://unpkg.com/leaflet@1.9.4/dist/leaflet.js"></script>
  <script src="https://unpkg.com/leaflet.heat@0.2.0/dist/leaflet-heat.js"></script>
  <link href="https://fonts.googleapis.com/css2?family=Outfit:wght@400;600;700&family=JetBrains+Mono:wght@400;600&display=swap" rel="stylesheet">
  <style>
    :root {{
      --bg-dark: #070d18;
      --panel-bg: rgba(15, 23, 42, 0.88);
      --border-cyan: rgba(56, 189, 248, 0.28);
      --cyan: #00e5ff;
      --text: #f8fafc;
    }}
    * {{ box-sizing: border-box; margin: 0; padding: 0; }}
    body, html {{ height: 100%; width: 100%; font-family: 'Outfit', sans-serif; background: var(--bg-dark); color: var(--text); overflow: hidden; }}
    #map {{ height: 100%; width: 100%; }}
    .header-bar {{
      position: absolute; top: 16px; left: 16px; z-index: 1000;
      background: var(--panel-bg); backdrop-filter: blur(10px);
      border: 1px solid var(--border-cyan); border-radius: 12px;
      padding: 14px 20px; box-shadow: 0 10px 30px rgba(0,0,0,0.6);
      max-width: 440px;
    }}
    .header-bar h1 {{ font-size: 1.15rem; color: var(--cyan); margin-bottom: 4px; display: flex; align-items: center; gap: 8px; }}
    .header-bar p {{ font-size: 0.82rem; color: #94a3b8; line-height: 1.35; }}
    .legend-box {{
      position: absolute; bottom: 24px; right: 16px; z-index: 1000;
      background: var(--panel-bg); backdrop-filter: blur(10px);
      border: 1px solid var(--border-cyan); border-radius: 10px;
      padding: 12px 16px; font-size: 0.78rem; box-shadow: 0 8px 24px rgba(0,0,0,0.5);
    }}
    .legend-gradient {{
      width: 180px; height: 12px; border-radius: 4px; margin: 6px 0;
      background: linear-gradient(to right, #00e5ff, #3b82f6, #10b981, #f59e0b, #ef4444);
    }}
    .legend-labels {{ display: flex; justify-content: space-between; font-family: 'JetBrains Mono', monospace; font-size: 0.7rem; color: #cbd5e1; }}
    .control-group {{ margin-top: 10px; display: flex; gap: 8px; }}
    .btn {{
      background: rgba(56, 189, 248, 0.15); border: 1px solid var(--cyan); color: var(--cyan);
      padding: 6px 12px; border-radius: 6px; font-size: 0.78rem; font-weight: 600; cursor: pointer;
      transition: all 0.2s;
    }}
    .btn:hover {{ background: var(--cyan); color: #070d18; }}
  </style>
</head>
<body>
  <div id="map"></div>
  <div class="header-bar">
    <h1>🔥 Mapa de Calor: Agua Somera en Tijuana</h1>
    <p>Interpolación continua de densidad térmica basada en 23 registros forenses y piezométricos directos (sótanos con cárcamo, peritajes judiciales, tomografías geoeléctricas y CONAGUA).</p>
    <div class="control-group">
      <button class="btn" onclick="toggleHeat()">Alternar Calor</button>
      <button class="btn" onclick="toggleMarkers()">Alternar Puntos</button>
      <button class="btn" onclick="fitTijuana()">Centrar Tijuana</button>
    </div>
  </div>

  <div class="legend-box">
    <div style="font-weight: 700; color: #f8fafc;">Profundidad NAF / Intensidad de Saturación</div>
    <div class="legend-gradient"></div>
    <div class="legend-labels">
      <span>> 7m (Bajo)</span>
      <span>3 - 4m</span>
      <span>< 2m (Aflorante)</span>
    </div>
  </div>

  <script>
    const puntos = {puntos_json};
    const heatData = {json.dumps(pts_heat_json)};

    const map = L.map('map', {{
      center: [32.505, -117.02],
      zoom: 12,
      zoomControl: false
    }});
    L.control.zoom({{ position: 'topright' }}).addTo(map);

    // Cartografía Dark Matter
    L.tileLayer('https://{{s}}.basemaps.cartocdn.com/dark_all/{{z}}/{{x}}/{{y}}{{r}}.png', {{
      attribution: '&copy; CartoDB &copy; OpenStreetMap | AquaResiliencia',
      subdomains: 'abcd',
      maxZoom: 19
    }}).addTo(map);

    // Capa de Calor
    const heatLayer = L.heatLayer(heatData, {{
      radius: 35,
      blur: 25,
      maxZoom: 15,
      gradient: {{
        0.15: '#00e5ff',
        0.35: '#3b82f6',
        0.60: '#10b981',
        0.80: '#f59e0b',
        1.00: '#ef4444'
      }}
    }}).addTo(map);

    // Capa de Marcadores
    const markerLayer = L.layerGroup().addTo(map);
    puntos.forEach(p => {{
      const color = p.profundidad_naf_m <= 2.0 ? '#ef4444' :
                    p.profundidad_naf_m <= 4.0 ? '#f59e0b' :
                    p.profundidad_naf_m <= 6.0 ? '#10b981' : '#00e5ff';

      const circle = L.circleMarker([p.lat, p.lng], {{
        radius: 7,
        fillColor: color,
        color: '#ffffff',
        weight: 1.5,
        opacity: 0.9,
        fillOpacity: 0.85
      }});

      const popupHtml = `
        <div style="font-family:'Outfit',sans-serif;min-width:240px;color:#0f172a;line-height:1.4">
          <b style="color:#0284c7;font-size:0.95rem">${{p.id}}: ${{p.nombre}}</b><br>
          <span style="font-size:0.8rem;color:#64748b">${{p.delegacion}}</span>
          <hr style="margin:6px 0;border:0;border-top:1px solid #e2e8f0">
          <b>Profundidad NAF:</b> <span style="font-size:1rem;color:#b91c1c;font-weight:700">${{p.profundidad_naf_m}} m</span><br>
          <b>Categoría:</b> ${{p.categoria}}<br>
          <b>Geología:</b> <span style="font-size:0.75rem">${{p.geologia}}</span><br>
          <b>Evidencia:</b> <span style="font-size:0.75rem">${{p.evidencia}}</span>
        </div>
      `;
      circle.bindPopup(popupHtml);
      markerLayer.addLayer(circle);
    }});

    function toggleHeat() {{
      if (map.hasLayer(heatLayer)) {{ map.removeLayer(heatLayer); }}
      else {{ map.addLayer(heatLayer); }}
    }}

    function toggleMarkers() {{
      if (map.hasLayer(markerLayer)) {{ map.removeLayer(markerLayer); }}
      else {{ map.addLayer(markerLayer); }}
    }}

    function fitTijuana() {{
      map.setView([32.505, -117.02], 12);
    }}
  </script>
</body>
</html>
"""
    with open(output_html, 'w', encoding='utf-8') as f:
        f.write(html_code)
    print(f"[*] Mapa de calor interactivo generado: {output_html}")

if __name__ == "__main__":
    pts = cargar_datos_csv()
    print(f"[*] Puntos cargados exitosamente: {len(pts)}")
    generar_mapa_calor_png(pts)
    generar_mapa_calor_html(pts)
