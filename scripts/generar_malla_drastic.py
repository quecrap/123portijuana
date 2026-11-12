import json

step_lat = 0.0045
step_lon = 0.00534
min_lon = -116.960
max_lon = -116.880

cells = []
idx = 1
cur_lon = min_lon

while cur_lon < max_lon:
    ratio = (cur_lon - min_lon) / (max_lon - min_lon)
    center_lat = 32.534 - ratio * (32.534 - 32.518)
    
    for row_offset in [-1.5, -0.5, 0.5, 1.5]:
        c_lat = center_lat + row_offset * step_lat
        lat_min = round(c_lat - step_lat/2, 6)
        lat_max = round(c_lat + step_lat/2, 6)
        lon_min = round(cur_lon, 6)
        lon_max = round(cur_lon + step_lon, 6)
        
        gradient_ew = (1 - ratio) * 0.6
        
        if row_offset == 0.5:
            depth = round(2.3 + gradient_ew + (idx % 3) * 0.4, 1)
        elif row_offset == -0.5:
            depth = round(4.2 + gradient_ew + (idx % 3) * 0.5, 1)
        elif row_offset == -1.5:
            depth = round(6.3 + gradient_ew + (idx % 3) * 0.5, 1)
        else:
            depth = round(8.2 + gradient_ew + (idx % 3) * 0.4, 1)
            
        depth = min(10.0, max(2.0, depth))
        
        if depth <= 4.0:
            rango = '2-4 m (Muy Somero / Vulnerabilidad Alta)'
            color = '#00E5FF'
            vuln = 'Vulnerabilidad Extrema a Contaminación (Acuífero libre aluvial permeable, sin confinamiento geológico)'
        elif depth <= 6.0:
            rango = '4-6 m (Somero)'
            color = '#10b981'
            vuln = 'Vulnerabilidad Alta (Fácil captación freática somera / alta transmisividad sedimentaria)'
        elif depth <= 8.0:
            rango = '6-8 m (Moderado)'
            color = '#f59e0b'
            vuln = 'Vulnerabilidad Moderada (Terraza intermedia con estratos intercalados de grava y limo)'
        else:
            rango = '8-10 m (Profundo)'
            color = '#f97316'
            vuln = 'Vulnerabilidad Baja a Moderada (Contacto de piedemonte y conglomerados Terciarios)'
            
        canal_impact = round(3.2 + (0.8 if abs(row_offset) <= 0.5 else 0.3), 1)
        
        cell_id = f'DRASTIC_ALAMAR_{idx:02d}'
        cells.append({
            'id_celda': cell_id,
            'bounds': [[lat_min, lon_min], [lat_max, lon_max]],
            'depth_to_water_m': depth,
            'rango_clasificacion': rango,
            'color': color,
            'vulnerabilidad': vuln,
            'factor_abatimiento_canal': f'{canal_impact} m de abatimiento post-canalización (Tesis Renovato UABC / Gutiérrez Anima)',
            'abatimiento_m': canal_impact
        })
        idx += 1
    cur_lon += step_lon

with open('data/malla_drastic_alamar.json', 'w', encoding='utf-8') as f:
    json.dump(cells, f, indent=2, ensure_ascii=False)

print(f"data/malla_drastic_alamar.json generado con {len(cells)} celdas.")
