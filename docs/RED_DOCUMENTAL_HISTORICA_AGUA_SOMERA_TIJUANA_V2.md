# Red Documental e Histórica del Agua Somera de Tijuana (v2.0)
**Dataset Consolidado de 50 Puntos Georreferenciados + Red Piezométrica CONAGUA 0201**  
**Fecha de actualización:** Septiembre de 2026  
**Colectivo 1, 2, 3 por Tijuana / Iniciativa AquaResiliencia Tijuana**

---

## 1. Introducción y Marco de Referencia

La ciudad de Tijuana ha enfrentado históricamente una paradoja hídrica: mientras depende en más de un 90% del Acueducto Río Colorado–Tijuana (con un gasto energético de ~7.5 kWh/m³ para remontar La Rumorosa), el subsuelo urbano y periurbano alberga importantes mantos freáticos someros (1 a 10 metros de profundidad). 

Históricamente, estos mantos freáticos han sido tratados como un estorbo constructivo —evacuados y achicados 24/7 hacia el drenaje o la canalización mediante cárcamos y losas de supresión— o bien han detonado deslizamientos de ladera al combinarse con fugas de agua potable sobre arcillas expansivas (Formación Otay).

Este documento compila y sistematiza la **Red Documental e Histórica v2.0**, respaldada por el dataset maestro de 50 registros georreferenciados (`data/DATASET_MAPA_CALOR_AGUA_SOMERA_TIJUANA.csv`) y la Red Piezométrica Oficial de CONAGUA (`data/RED_MONITOREO_PIEZOMETRICO_CONAGUA_TIJUANA_0201.csv`).

---

## 2. Estructura y Taxonomía del Dataset v2 (50 Registros)

El dataset consolida 50 puntos georreferenciados en el datum WGS84, categorizados según su tipología de evidencia hidrológica y geotécnica:

```
┌──────────────────────────────────────────────────────────┬───────────┬────────────────────────────────────────────────────────┐
│ Grupo de Puntos                                          │ Cantidad  │ Descripción y Tipología de Evidencia                   │
├──────────────────────────────────────────────────────────┼───────────┼────────────────────────────────────────────────────────┤
│ P01 – P23 (Puntos de Campo y Obras Urbanas)              │ 23        │ Sótanos con bombeo continuo, deslizamientos,           │
│                                                          │           │ bioindicadores freatófitos, manantiales y geofísica.  │
│ R01 – R20 (Concesiones y Títulos Oficiales REPDA)        │ 20        │ Aprovechamientos industriales, comerciales, baterías   │
│                                                          │           │ masivas CESPT (19 pozos) y comunitarias.               │
│ Z01 – Z07 (Piezometría Instrumental CONAGUA GAS1)        │ 7         │ Pozos oficiales de monitoreo piezométrico con cotas    │
│                                                          │           │ de brocal y profundidad a nivel estático (PNE).        │
└──────────────────────────────────────────────────────────┴───────────┴────────────────────────────────────────────────────────┘
```

---

## 3. Matriz de Evidencia por Categorías

### A. Sótanos con Cárcamo de Achique 24/7 y Losas de Supresión
* **P01 (Torre Cosmopolitan - Zona Río):** NAF a 3.2 m. Aluvión arenoso saturado. Bombeo permanente para abatir subpresión hidrostática.
* **P02 (New City Medical Plaza):** NAF a 2.8 m. Aluvión fluvial próximo a canalización; cimentación con pilas profundas y cárcamo activo.
* **P03 (Plaza Río Tijuana):** NAF a 3.8 m. Pozos de alivio piezométrico en estacionamientos subterráneos.
* **R04 (Club Campestre Tijuana):** Permiso REPDA `01BCA150194/01ERDA18` para achique y drenaje de 2,592 m³/día en aluvión saturado somero.

### B. Grandes Concesiones Industriales y Baterías de Extracción (REPDA)
* **R10 (Batería Principal CESPT - Río Tijuana):** 19 pozos subterráneos en el aluvión cuaternario principal con volumen concesionado de **13,213,584 m³/año** (Título `01BCA100301/01HSDA19`).
* **R01 (Embotelladora del Fuerte / Coca-Cola):** 2 pozos activos con **387,680 m³/año** (Título `BCA101749`).
* **R02 (Pasteurizadora Jersey del Noroeste):** 3 pozos con **378,432 m³/año** en Vía Rápida Poniente (Título `BCA109271`).
* **R03 (Hipódromo / Estadio Caliente):** 2 títulos sumando **240,000 m³/año** en el Valle de Agua Caliente con termalismo somero.
* **R05 (Pipas CROM/CROC):** 2 pozos con **294,600 m³/año** en La Mesa para distribución en pipas.
* **R19 (Familia Gutiérrez Valenzuela):** 10 pozos agrícolas en el Arroyo Alamar sumando más de **180,000 m³/año**.

### C. Deslizamientos Rotacionales y Riesgo Geotécnico
* **P08 (Valle del Sur):** NAF a 3.0 m. Tomografía de resistividad eléctrica dipolo-dipolo (Delgado Argote et al., *GEOS* 2017) demostrando variabilidad de 3 a 18 m en menos de 200 m y afloramiento directo.
* **P09 (Lomas del Rubí):** NAF a 1.8 m. Contacto Fm. Otay / Fm. San Diego; colapso de 2018 inducido por saturación del plano de falla.
* **P10 (Camino Verde - Cañón de las Carretas):** NAF a 2.2 m. Declaratoria de Emergencia Geológica 2022 con sondeos a 2 m.
* **P11 (Sánchez Taboada - Casiopea):** NAF a 2.0 m. Límite de contacto con paleocanales saturados.
* **P12 (Cañón del Matadero):** NAF a 1.5 m. Suelos limosos colapsables saturados que provocaron el colapso del terraplén carretero en 2023.

### D. Manantiales Históricos y Bioindicadores Freatófitos
* **P16 (Cañón Johnson / Col. Hidalgo):** NAF a 0.8 m. Afloramiento perenne de agua dulce de venero natural.
* **P17 (Aguaje de la Tuna):** NAF a 1.0 m. Manantial fundacional registrado desde la cartografía del siglo XIX (concesión SEDENA `01BCA150147`).
* **P19 (Sauceda Cañón de San Antonio):** NAF a 2.5 m. Presencia de freatófitos obligados (*Salix gooddingii*, *Populus fremontii*).
* **P20 (Arroyo Huertita - Playas Sur):** NAF a 2.8 m. Bosquete costero de sauce dependiente de nivel colgado somero.

### E. Red Piezométrica Instrumental Oficial (CONAGUA GAS1 - Acuífero 0201)
* **Z01 (Pozo CNA_99 - Agua Caliente):** Elevación 28.17 msnm, PNE reciente -3.83 m.
* **Z02 (Pozo CNA_33 - Arroyo Alamar):** Elevación 59.38 msnm, PNE reciente -3.95 m.
* **Z03 (Pozo 36 - Zona Río Padre Kino):** Elevación 20.01 msnm, PNE reciente -4.34 m.
* **Z04 (Pozo XD - Plaza Río):** Elevación 25.83 msnm, PNE reciente -4.36 m.
* **Z05 (Pozo 14 - La Mesa Benítez):** Elevación 35.96 msnm, PNE reciente -4.48 m.
* **Z06 (Pozo 73 - Vía Rápida Oriente):** Elevación 25.74 msnm, PNE reciente -5.12 m.
* **Z07 (Pozo 70 - Cañón del Rubí):** Elevación 49.07 msnm, PNE reciente -5.47 m.

---

## 4. Columnas del Dataset (`DATASET_MAPA_CALOR_AGUA_SOMERA_TIJUANA.csv`)

1. `id`: Identificador único (P01–P23, R01–R20, Z01–Z07).
2. `nombre_sitio`: Nombre común o toponimia del punto de medición.
3. `delegacion`: Delegación municipal y colonia/sector.
4. `latitud`: Coordenada decimal en grados norte (WGS84).
5. `longitud`: Coordenada decimal en grados oeste (WGS84).
6. `profundidad_naf_m`: Profundidad del Nivel de Aguas Freáticas en metros bajo superficie.
7. `intensidad_calor`: Normalización de 0.0 a 1.0 para el algoritmo de densidad KDE del mapa de calor.
8. `categoria_evidencia`: Tipología técnica del punto.
9. `tipo_suelo_geologia`: Unidad litoestratigráfica (Aluvión $Qal$, Fm. Otay, Fm. San Diego, terrazas marinas).
10. `fuente_documental`: Cita bibliográfica, oficio, título REPDA o peritaje judicial.
11. `verificacion`: Estado de consistencia y validación de gabinete/campo.

---

## 5. Conclusiones y Rutas de Aprovechamiento Resiliente

1. **Desacoplar la demanda urbana no potable:** Con más de 13 millones de m³/año extraídos en la cuenca baja y decenas de miles de litros diarios tirados al drenaje por achique de sótanos, la captación somera para riego de parques (Sistema Metropolitano de Parques) y uso de servicios es plenamente viable.
2. **Monitoreo telemétrico como mitigación de riesgo:** La instalación de piezómetros IoT en zonas de ladera (Camino Verde, Sánchez Taboada, Rubí) permitiría alertas tempranas ante sobre-saturación de arcillas expansivas.
3. **Ciencia Ciudadana y Datos Abiertos:** Este dataset representa el esfuerzo más completo de integración de datos freáticos públicos y oficiales de Tijuana a la fecha.
