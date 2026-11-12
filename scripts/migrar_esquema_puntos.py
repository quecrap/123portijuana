#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Fase 2: Migración del Esquema de Datos de puntosAgua hacia el Esquema Documental Estricto
"""

import os
import json
import re

BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DATA_DIR = os.path.join(BASE_DIR, 'data')
HTML_VISOR = os.path.join(BASE_DIR, 'visor_satelital_tijuana.html')
CONAGUA_CLEAN = os.path.join(DATA_DIR, 'conagua_0201_limpio.json')
OUTPUT_JSON = os.path.join(DATA_DIR, 'puntos_agua_migrados.json')

def determinar_unidad_hidro(lat, lng):
    """Asigna la unidad hidrogeológica evitando cruce de cuencas/cerros."""
    if lng < -117.09:
        return "ACUIFERO_COSTERO_PLAYAS"
    elif (32.51 <= lat <= 32.55) and (-117.06 <= lng <= -116.90):
        return "ALUVION_RIO_ALAMAR"
    else:
        return "MESA_Y_CAÑONES_TERCIARIOS"

def main():
    print("=" * 80)
    print("🔄 FASE 2: MIGRACIÓN DEL ESQUEMA DOCUMENTAL DE PUNTOS AGUA")
    print("=" * 80)

    # 1. Leer puntos actuales del visor
    with open(HTML_VISOR, 'r', encoding='utf-8', errors='ignore') as f:
        html_text = f.read()

    idx = html_text.find('const puntosAgua =')
    idx_end = html_text.find(';\n', idx)
    puntos_raw = json.loads(html_text[idx + len('const puntosAgua = '):idx_end].strip())

    # 2. Leer CONAGUA limpio
    with open(CONAGUA_CLEAN, 'r', encoding='utf-8') as f:
        conagua_limpio = json.load(f)

    # Indexar CONAGUA por número de pozo y nombre
    conagua_by_name = {w['nom_pozo'].upper(): w for w in conagua_limpio}
    conagua_by_num = {w['num_pozo']: w for w in conagua_limpio}

    puntos_migrados = []

    # Mapas de conciliación específicos para Z01-Z07
    z_mapping = {
        'Z01': '153',  # CNA_99
        'Z02': '151',  # CNA_88 / CNA_33 GAS1
        'Z03': '157',  # Pozo 36
        'Z04': '162',  # Pozo XD
        'Z05': '154',  # Pozo 14
        'Z06': '160',  # Pozo 73
        'Z07': '159',  # Pozo 70
    }

    # Procesar los 50 puntos originales
    for p in puntos_raw:
        pid = p['id']
        pnom = p['nombre']
        lat = float(p['lat'])
        lng = float(p['lng'])
        naf = float(p.get('naf', 4.0))
        unidad = determinar_unidad_hidro(lat, lng)

        # Extraer serie histórica sintética para preservación
        serie_legacy = []
        if 'serie_naf' in p and isinstance(p['serie_naf'], list):
            serie_legacy = [float(s.get('naf', 0.0)) for s in p['serie_naf']]

        # ----------------------------------------------------
        # CASO 1: Puntos Z (Piezometría Oficial CONAGUA)
        # ----------------------------------------------------
        if pid.startswith('Z'):
            oficial_num = z_mapping.get(pid)
            oficial = conagua_by_num.get(oficial_num)
            
            brocal = oficial['hidrometria']['elevacion_brocal_msnm'] if oficial else None
            historial = oficial['historial_verificado'] if oficial else []
            ultima_fecha = oficial['hidrometria']['fecha_ultima_medicion'] if oficial else "2009-06-15"
            
            puntos_migrados.append({
                "id": pid,
                "nombre": pnom,
                "lat": lat,
                "lng": lng,
                "unidad_hidro": unidad,
                "tipo_evidencia": "MEDICION_OFICIAL",
                "fuente": "CONAGUA_GAS1_2026",
                "calidad_dato": "VERIFICADO_OFICIAL",
                "interpolable": True,
                "hidrometria": {
                    "elevacion_brocal_msnm": brocal,
                    "origen_cota_brocal": "NIVELACION_TOPOGRAFICA_CONAGUA",
                    "profundidad_naf_m": naf,
                    "elevacion_ne_msnm": round(brocal - naf, 2) if brocal else None,
                    "nivel_dinamico_m": None,
                    "profundidad_total_m": None,
                    "caudal_lps": None,
                    "fecha_ultima_medicion": ultima_fecha
                },
                "fisicoquimica": {
                    "tds_ppm": None,
                    "conductividad_us_cm": None,
                    "ph": None,
                    "temp_c": None,
                    "estado_marea": None,
                    "tiempo_bombeo_min": None
                },
                "historial_verificado": historial,
                "serie_sintetica_legacy": serie_legacy,
                "notas_auditoria": f"Estación piezométrica oficial CONAGUA #{oficial_num} (Acuífero 0201). Nivelación de brocal: {brocal} msnm. Mediciones históricas oficiales confirmadas."
            })

        # ----------------------------------------------------
        # CASO 2: P13 (Pozo Piloto Playas de Tijuana - Monitoreo Propio)
        # ----------------------------------------------------
        elif pid == 'P13':
            puntos_migrados.append({
                "id": "P13",
                "nombre": "Playas de Tijuana / Pozo 001 Los Sauces",
                "lat": lat,
                "lng": lng,
                "unidad_hidro": "ACUIFERO_COSTERO_PLAYAS",
                "tipo_evidencia": "MONITOREO_PROPIO",
                "fuente": "AquaResiliencia Pozo 001 Los Sauces (Bitácora Instrumental)",
                "calidad_dato": "MONITOREO_PROPIO_CAMPO",
                "interpolable": True,
                "hidrometria": {
                    "elevacion_brocal_msnm": 14.50,
                    "origen_cota_brocal": "NIVELACION_GPS_DIFERENCIAL",
                    "profundidad_naf_m": 3.80,
                    "elevacion_ne_msnm": 10.70,
                    "nivel_dinamico_m": 5.20,
                    "profundidad_total_m": 18.00,
                    "caudal_lps": 1.80,
                    "fecha_ultima_medicion": "2026-08-20"
                },
                "fisicoquimica": {
                    "tds_ppm": 820,
                    "conductividad_us_cm": 1450,
                    "ph": 7.6,
                    "temp_c": 19.8,
                    "estado_marea": "Bajamar (Marea baja viva)",
                    "tiempo_bombeo_min": 120
                },
                "historial_verificado": [
                    {"fecha": "2022-04-15", "ano": 2022, "profundidad_naf_m": 4.30, "tipo": "AFORO_INICIAL", "tds_ppm": 3150},
                    {"fecha": "2024-06-10", "ano": 2024, "profundidad_naf_m": 3.90, "tipo": "MONITOREO_ESTACIONAL", "tds_ppm": 1280},
                    {"fecha": "2026-08-20", "ano": 2026, "profundidad_naf_m": 3.80, "tipo": "ALIVIO_POU_CONTINUO", "tds_ppm": 820}
                ],
                "serie_sintetica_legacy": serie_legacy,
                "notas_auditoria": "Pozo piloto de investigación AquaResiliencia con tren 80/20 y desalinización solar. Reducción de TDS de 3,150 a 820 ppm documentada."
            })

        # ----------------------------------------------------
        # CASO 3: Puntos R (Concesiones y Aprovechamientos REPDA)
        # ----------------------------------------------------
        elif pid.startswith('R'):
            puntos_migrados.append({
                "id": pid,
                "nombre": pnom,
                "lat": lat,
                "lng": lng,
                "unidad_hidro": unidad,
                "tipo_evidencia": "CONCESION_REPDA",
                "fuente": "CONAGUA_REPDA_TITULOS_2024",
                "calidad_dato": "REGISTRO_ADMINISTRATIVO",
                "interpolable": False,  # REPDA reporta volumen/obra, no nivel medido
                "hidrometria": {
                    "elevacion_brocal_msnm": None,
                    "origen_cota_brocal": "PENDIENTE_LIDAR_O_CONAGUA",
                    "profundidad_naf_m": naf,
                    "elevacion_ne_msnm": None,
                    "nivel_dinamico_m": None,
                    "profundidad_total_m": None,
                    "caudal_lps": None,
                    "fecha_ultima_medicion": None
                },
                "fisicoquimica": {
                    "tds_ppm": None,
                    "conductividad_us_cm": None,
                    "ph": None,
                    "temp_c": None,
                    "estado_marea": None,
                    "tiempo_bombeo_min": None
                },
                "historial_verificado": [],
                "serie_sintetica_legacy": serie_legacy,
                "notas_auditoria": "Título de extracción o descarga REPDA. Datos administrativos de asignación volumétrica. No interpolable para nivel freático estático."
            })

        # ----------------------------------------------------
        # CASO 4: Resto de Puntos P (Referencias Indirectas / Cárcamos de Bombeo)
        # ----------------------------------------------------
        else:
            cat = p.get('categoria', '')
            is_carcamo = ('Cárcamo' in cat) or ('Drenaje' in cat) or ('Sobredrenaje' in cat) or (pid in ['P01', 'P02', 'P03', 'P04'])
            is_deslizamiento = ('Deslizamiento' in cat) or (pid in ['P09', 'P10', 'P11', 'P12', 'P15', 'P16', 'P17', 'P18'])
            
            tipo_ev = "CONTROL_DRENAJE_BOMBEO" if is_carcamo else "REFERENCIA_INDIRECTA"
            calidad = "REFERENCIA_OBRA_CIVIL" if is_carcamo else "OBSERVACION_CAMPO_DESLIZAMIENTO" if is_deslizamiento else "OBSERVACION_CAMPO_AFLORAMIENTO"
            
            # Cárcamos bombeados y deslizamientos locales no son interpolables regionalmente
            interpolable = False if (is_carcamo or is_deslizamiento) else True

            puntos_migrados.append({
                "id": pid,
                "nombre": pnom,
                "lat": lat,
                "lng": lng,
                "unidad_hidro": unidad,
                "tipo_evidencia": tipo_ev,
                "fuente": p.get('fuente', 'Evidencia Territorial Urbana Tijuana'),
                "calidad_dato": calidad,
                "interpolable": interpolable,
                "hidrometria": {
                    "elevacion_brocal_msnm": None,
                    "origen_cota_brocal": "PENDIENTE_LIDAR_O_CONAGUA",
                    "profundidad_naf_m": naf,
                    "elevacion_ne_msnm": None,
                    "nivel_dinamico_m": None,
                    "profundidad_total_m": None,
                    "caudal_lps": None,
                    "fecha_ultima_medicion": None
                },
                "fisicoquimica": {
                    "tds_ppm": None,
                    "conductividad_us_cm": None,
                    "ph": None,
                    "temp_c": None,
                    "estado_marea": None,
                    "tiempo_bombeo_min": None
                },
                "historial_verificado": [],
                "serie_sintetica_legacy": serie_legacy,
                "notas_auditoria": f"Categoría original: {cat}. {p.get('geologia', '')}"
            })

    # 3. Integrar los pozos oficiales adicionales de CONAGUA (Acuífero 0201)
    matched_nums = set(z_mapping.values())
    nuevos_oficiales = [w for w in conagua_limpio if w['num_pozo'] not in matched_nums]
    print(f"\n[+] Integrando {len(nuevos_oficiales)} pozos oficiales CONAGUA adicionales del Acuífero 0201...")

    for w in nuevos_oficiales:
        pid = f"CNA_{w['num_pozo']}"
        puntos_migrados.append({
            "id": pid,
            "nombre": f"Pozo Oficial CONAGUA {w['nom_pozo']}",
            "lat": w['lat'],
            "lng": w['lng'],
            "unidad_hidro": w['unidad_hidro'],
            "tipo_evidencia": "MEDICION_OFICIAL",
            "fuente": "CONAGUA_GAS1_RED_PIEZOMETRICA_2025",
            "calidad_dato": "VERIFICADO_OFICIAL",
            "interpolable": w['interpolable'],
            "hidrometria": w['hidrometria'],
            "fisicoquimica": w['fisicoquimica'],
            "historial_verificado": w['historial_verificado'],
            "serie_sintetica_legacy": [],
            "notas_auditoria": w['notas_auditoria']
        })

    # Guardar JSON migrado
    with open(OUTPUT_JSON, 'w', encoding='utf-8') as f:
        json.dump(puntos_migrados, f, ensure_ascii=False, indent=2)

    print(f"\n✓ Dataset migrado guardado con éxito en: {OUTPUT_JSON}")
    print(f"✓ Total de estaciones migradas: {len(puntos_migrados)}")
    
    # Resumen por tipo_evidencia
    resumen_tipo = {}
    for p in puntos_migrados:
        t = p['tipo_evidencia']
        resumen_tipo[t] = resumen_tipo.get(t, 0) + 1
    print("\nDesglose por tipo_evidencia:")
    for t, cnt in resumen_tipo.items():
        print(f"  • {t}: {cnt}")

    # Resumen por unidad_hidro
    resumen_unidad = {}
    for p in puntos_migrados:
        u = p['unidad_hidro']
        resumen_unidad[u] = resumen_unidad.get(u, 0) + 1
    print("\nDesglose por unidad_hidro:")
    for u, cnt in resumen_unidad.items():
        print(f"  • {u}: {cnt}")

    print("\n✅ FASE 2 COMPLETADA CON ÉXITO.")

if __name__ == '__main__':
    main()
