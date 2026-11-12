# 📑 REPORTE DE AUDITORÍA Y CONCILIACIÓN PIEZOMÉTRICA CONAGUA (ACUÍFERO 0201 TIJUANA)

**Fecha de Auditoría:** 2026-11-11 22:25:19
**Fuente Primaria:** Base Nacional de la Red Piezométrica CONAGUA (`red_piezometrica_2025.dbf`), Geovisor SIGAGIS v5.8 y Archivo de Campaña GAS1 2009 Acuífero 0201.
**Entidad Auditada:** Archivo `visor_satelital_tijuana.html` de AquaResiliencia Tijuana.

---

## 1. CONCLUSIONES EJECUTIVAS DE LA AUDITORÍA

1. **Existencia Real de `Z04: Pozo XD`:** Se comprobó de forma concluyente que `Pozo XD` **NO es un placeholder sintético**. Está catalogado textualmente como `Pozo XD` (Registro Oficial #162) en la base de datos de la Gerencia de Aguas Subterráneas (CONAGUA GAS1) con coordenadas `32.52535, -117.01527`, brocal nivelado a `25.834 msnm` y mediciones de nivel estático en 2009 (`4.36 m`) y 2013 (`5.70 m`).
2. **Identidad de `Z01–Z07`:** 6 de los 7 puntos (`Z01`, `Z03`, `Z04`, `Z05`, `Z06`, `Z07`) coinciden exactamente a menos de 6 metros de distancia Haversine con pozos oficiales de la red piezométrica federal. El punto `Z02 (CNA_33)` corresponde al registro #8 de la campaña GAS1 de Tijuana (`32.52259, -116.91410`, brocal `59.38 m`), localizado a 430 m del pozo #151 `CNA_88`.
3. **Corte Temporal de Mediciones Oficiales (2009 y 2013):** En la base oficial de CONAGUA para el Acuífero 0201 Tijuana, **las últimas mediciones de campo registradas corresponden a los años 2009 y 2013**. No existen mediciones de campo gubernamentales para 2018–2026 en este acuífero. **Las series de 10 valores (2018–2027) presentes en el visor son proyecciones sintéticas**, por lo que deben reclasificarse a `serie_sintetica_legacy` y dejar `historial_verificado: []` con los años reales.
4. **Precisión Topográfica de Brocales:** Todas las cotas de brocal de CONAGUA en el Acuífero 0201 cuentan con nivelación topográfica instrumental con precisión decimal (`28.17 m`, `20.01 m`, `25.83 m`, etc.), lo que valida su uso para el cálculo de superficies piezométricas absolutas (`ELEVNE = ELEVBROC - PNE`).

---

## 2. TABLA COMPARATIVA DE CONCILIACIÓN: `Z01–Z07`

| ID Visor | Nombre en Visor | Pozo Oficial CONAGUA | Lat Oficial | Lon Oficial | Distancia (m) | Brocal (msnm) | PNE Real 2009 (m) | PNE Real 2013 (m) | Estado Auditoría |
| :--- | :--- | :--- | :---: | :---: | :---: | :---: | :---: | :---: | :--- |
| **Z01** | Pozo Oficial CONAGUA CNA_99 (Agua Caliente) | #153 CNA_99 | 32.51782 | -117.00187 | 3.8 m | 28.17 | 3.83 | 3.32 | ✅ MATCH EXACTO (<6m) |
| **Z02** | Pozo Oficial CONAGUA CNA_33 (Arroyo Alamar) | #151 CNA_88 | 32.52218 | -116.91866 | 430.33 m | 58.24 | 7.41 | N/D | 🟡 MATCH GAS1 2009 |
| **Z03** | Pozo Oficial CONAGUA Pozo 36 (Zona Río Padre Kino) | #157 Pozo 36 | 32.53587 | -117.02677 | 4.78 m | 20.01 | 4.34 | N/D | ✅ MATCH EXACTO (<6m) |
| **Z04** | Pozo Oficial CONAGUA Pozo XD (Plaza Río / Héroes) | #162 Pozo XD | 32.52535 | -117.01527 | 5.74 m | 25.83 | 4.36 | 5.7 | ✅ MATCH EXACTO (<6m) |
| **Z05** | Pozo Oficial CONAGUA Pozo 14 (La Mesa - Benítez) | #154 Pozo 14 | 32.51269 | -116.96822 | 2.52 m | 35.96 | 4.48 | 4.82 | ✅ MATCH EXACTO (<6m) |
| **Z06** | Pozo Oficial CONAGUA Pozo 73 (Vía Rápida Oriente) | #160 Pozo 73 | 32.53043 | -117.01379 | 3.98 m | 25.74 | 5.12 | 6.79 | ✅ MATCH EXACTO (<6m) |
| **Z07** | Pozo Oficial CONAGUA Pozo 70 (Cañón del Rubí / Fundadores) | #159 Pozo 70 | 32.50689 | -117.01379 | 1.51 m | 49.07 | 5.47 | 5.57 | ✅ MATCH EXACTO (<6m) |

---

## 3. POZOS OFICIALES ADICIONALES DE CONAGUA EN EL ACUÍFERO 0201 (45 POZOS)

Estos pozos pertenecen a la red geohidrológica oficial de CONAGUA para el Acuífero 0201 / cuenca de Tijuana y no estaban individualizados en los registros `Z` del visor actual:

| Num Pozo | Nombre Oficial | Latitud | Longitud | Brocal (msnm) | Años Medidos | Último PNE (m) | Condición Freática |
| :---: | :--- | :---: | :---: | :---: | :---: | :---: | :--- |
| 132 | CNA_11 | 32.53295 | -117.04542 | 27.62 | [2009, 2013] | 7.49m (2013) | 🟢 SOMERO (<10m) |
| 133 | CNA_13 | 32.53372 | -117.03135 | 21.68 | [2009] | 5.45m (2009) | 🟢 SOMERO (<10m) |
| 134 | CNA_14 | 32.51529 | -117.07481 | 101.77 | [2009] | 104.75m (2009) | 🔴 PROFUNDO (>25m) |
| 135 | CNA_15 | 32.50657 | -117.06339 | 135.43 | [2009, 2013] | 152.82m (2013) | 🔴 PROFUNDO (>25m) |
| 136 | CNA_17 | 32.48835 | -117.01112 | 125.47 | [2009, 2013] | 10.60m (2013) | 🟡 MEDIO (10-25m) |
| 137 | CNA_20 | 32.50662 | -117.02156 | 68.13 | [2009] | 22.31m (2009) | 🟡 MEDIO (10-25m) |
| 138 | CNA_37 | 32.51849 | -116.93390 | 56.04 | [2009, 2013] | 10.47m (2013) | 🟡 MEDIO (10-25m) |
| 139 | CNA_41 | 32.45024 | -116.87306 | 113.68 | [2009] | 29.31m (2009) | 🔴 PROFUNDO (>25m) |
| 140 | CNA_42 | 32.43193 | -116.88485 | 133.49 | [2009, 2013] | 37.05m (2013) | 🔴 PROFUNDO (>25m) |
| 141 | CNA_43 | 32.45708 | -116.91900 | 74.89 | [2009, 2013] | 38.95m (2013) | 🔴 PROFUNDO (>25m) |
| 142 | CNA_45 | 32.45076 | -116.93802 | 110.70 | [2009] | 24.44m (2009) | 🟡 MEDIO (10-25m) |
| 143 | CNA_46 | 32.46113 | -116.90904 | 84.96 | [2009] | 74.69m (2009) | 🔴 PROFUNDO (>25m) |
| 144 | CNA_47 | 32.50389 | -116.96918 | 52.27 | [2009] | 18.01m (2009) | 🟡 MEDIO (10-25m) |
| 145 | CNA_54 | 32.51432 | -116.99790 | 31.07 | [2009] | 6.21m (2009) | 🟢 SOMERO (<10m) |
| 146 | CNA_55 | 32.49340 | -117.01611 | 107.03 | [2009, 2013] | 13.81m (2013) | 🟡 MEDIO (10-25m) |
| 147 | CNA_61 | 32.53758 | -117.05015 | 17.35 | [2009] | 7.79m (2009) | 🟢 SOMERO (<10m) |
| 148 | CNA_64 | 32.53665 | -117.04686 | 19.00 | [2009, 2013] | 5.46m (2013) | 🟢 SOMERO (<10m) |
| 149 | CNA_65 | 32.52415 | -116.92359 | 60.52 | [2009, 2013] | 13.81m (2013) | 🟡 MEDIO (10-25m) |
| 150 | CNA_82 | 32.48931 | -117.03445 | 223.56 | [2009] | 36.19m (2009) | 🔴 PROFUNDO (>25m) |
| 151 | CNA_88 | 32.52218 | -116.91866 | 58.24 | [2009] | 7.41m (2009) | 🟢 SOMERO (<10m) |
| 152 | CNA_91 | 32.50997 | -116.94640 | 46.92 | [2009, 2013] | 8.03m (2013) | 🟢 SOMERO (<10m) |
| 155 | Pozo 3 | 32.51187 | -116.96101 | 41.33 | [2009] | 8.33m (2009) | 🟢 SOMERO (<10m) |
| 156 | Pozo 32 | 32.54189 | -117.03399 | 19.92 | [2009, 2013] | 7.43m (2013) | 🟢 SOMERO (<10m) |
| 158 | Pozo 44 | 32.51963 | -116.99259 | 33.35 | [2009, 2013] | 8.94m (2013) | 🟢 SOMERO (<10m) |
| 161 | Pozo Corette | 32.51473 | -116.98648 | 33.89 | [2009] | 7.36m (2009) | 🟢 SOMERO (<10m) |
| 163 | PB_1 | 32.49095 | -116.93756 | 51.56 | [2009] | 8.84m (2009) | 🟢 SOMERO (<10m) |
| 164 | PB_2 | 32.52612 | -117.02477 | 25.97 | [2009, 2013] | 6.71m (2013) | 🟢 SOMERO (<10m) |
| 165 | PB_3 | 32.51185 | -116.96098 | 41.95 | [2009] | 7.96m (2009) | 🟢 SOMERO (<10m) |
| 166 | PO_1 | 32.49094 | -116.93753 | 51.55 | [2009, 2013] | 8.66m (2013) | 🟢 SOMERO (<10m) |
| 167 | PO_2 | 32.52316 | -117.03269 | 49.76 | [2009] | 22.39m (2009) | 🟡 MEDIO (10-25m) |
| 168 | Pozo 13 | 32.51298 | -116.97935 | 35.67 | [2009, 2013] | 8.14m (2013) | 🟢 SOMERO (<10m) |
| 169 | CNA_10 | 32.51668 | -116.93735 | 54.43 | [2009] | 10.00m (2009) | 🟢 SOMERO (<10m) |
| 170 | CNA_18 | 32.50336 | -117.02150 | 76.26 | [2009] | 30.70m (2009) | 🔴 PROFUNDO (>25m) |
| 171 | CNA_19 | 32.51165 | -117.02184 | 58.24 | [2009] | 56.20m (2009) | 🔴 PROFUNDO (>25m) |
| 172 | CNA_44 | 32.45658 | -116.92913 | 86.94 | [2009] | 57.58m (2009) | 🔴 PROFUNDO (>25m) |
| 173 | CNA_59 | 32.54644 | -116.97743 | 146.19 | [2009] | 131.96m (2009) | 🔴 PROFUNDO (>25m) |
| 174 | CNA_62 | 32.53966 | -117.05158 | 17.36 | [2009] | 14.91m (2009) | 🟡 MEDIO (10-25m) |
| 175 | CNA_63 | 32.53968 | -117.04339 | 19.09 | [2009, 2013] | 5.60m (2013) | 🟢 SOMERO (<10m) |
| 176 | CNA_66 | 32.46063 | -116.90951 | 83.33 | [2009] | 74.99m (2009) | 🔴 PROFUNDO (>25m) |
| 177 | CNA-67 | 32.44486 | -116.89149 | 111.68 | [2009, 2013] | 22.00m (2013) | 🟡 MEDIO (10-25m) |
| 178 | CNA_81 | 32.48987 | -117.03569 | 218.29 | [2009, 2013] | 32.79m (2013) | 🔴 PROFUNDO (>25m) |
| 179 | CNA_97 | 32.49823 | -116.94051 | 45.55 | [2009, 2013] | 6.10m (2013) | 🟢 SOMERO (<10m) |
| 817 | CNA 45 22 | 32.42058 | -117.03747 | 135.00 | [2013] | 36.28m (2013) | 🔴 PROFUNDO (>25m) |
| 818 | CNA 45 23 | 32.40617 | -117.04964 | 57.00 | [2013] | 36.70m (2013) | 🔴 PROFUNDO (>25m) |
| 823 | CNA 45 28 | 32.43353 | -117.01264 | 257.00 | [2013] | 45.84m (2013) | 🔴 PROFUNDO (>25m) |
