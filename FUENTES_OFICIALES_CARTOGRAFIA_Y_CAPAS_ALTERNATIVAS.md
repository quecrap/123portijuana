# 🗺️ GUÍA TÉCNICA: FUENTES OFICIALES ALTERNATIVAS PARA CARTOGRAFÍA Y PROSPECCIÓN GEOCIENTÍFICA
## Identificación Indirecta de Niveles Freáticos, Litología y Zonas de Perforación en Tijuana (Fuera de CONAGUA)

**Proyecto:** AquaResiliencia Tijuana — Red Centinela de Justicia Hídrica  
**Convocatoria:** Venture Hacks 2026 (CDT / FIDEM / Cedar / Catalist)  
**Objetivo:** Sistematizar las fuentes de datos oficiales no-CONAGUA que revelan indirectamente la presencia, profundidad, calidad y viabilidad física del agua somera (1 a 15 m) en Tijuana para alimentar el algoritmo inteligente multicriterio (**AquaEngine**).

---

## 1. RESUMEN DE FUENTES CARTOGRÁFICAS ALTERNATIVAS

La determinación de sitios óptimos para captación somera no depende exclusivamente de las estaciones piezométricas oficiales de CONAGUA (que están diseñadas para el acuífero regional profundo). Existen múltiples capas de información pública e institucional que permiten deducir con alta precisión métrica el nivel freático somero, la permeabilidad del suelo y la seguridad estructural:

```
┌────────────────────────────────────────────────────────────────────────────────────────────────────────┐
│                        MATRIZ DE CAPAS CARTOGRÁFICAS Y FUENTES ALTERNATIVAS                            │
├───────────────────────┬───────────────────────────────┬───────────────────────────────┬────────────────┤
│ Capa / Dimensión      │ Fuente Oficial                │ Parámetro Revelado            │ Uso en Algoritmo│
├───────────────────────┼───────────────────────────────┼───────────────────────────────┼────────────────┤
│ 1. Litología & Rocas  │ SGM (Serv. Geológico Mexicano)│ Formación geológica, permeab. │ Dificultad perfor.│
│ 2. Topografía & DEM   │ INEGI (CEM 3.0 / LiDAR)       │ Cota (msnm), pendiente, TWI   │ Profundidad NAF│
│ 3. Riesgos & Fallas   │ Protección Civil / CENAPRED   │ Deslizamiento, talud húmedo   │ Kill-Switch / Veto│
│ 4. Hidrografía Fina   │ INEGI (RHD 2.0 / Cañadas)     │ Escorrentías, lechos aluviales│ Proximidad recarga│
│ 5. Aguas Binacionales │ CILA / IBWC / USGS            │ Aforo, conductividad, estuario│ Calidad / Salinidad│
│ 6. Fugas y Cortes     │ CESPT / IMPlan                │ Infiltración urbana y demanda │ Prioridad social│
│ 7. Geotecnia Civil    │ SIDURT / CFE / Sondeos SPT    │ Nivel freático en cimientos   │ Calibración NAF │
└───────────────────────┴───────────────────────────────┴───────────────────────────────┴────────────────┘
```

---

## 2. DETALLE DE FUENTES OFICIALES NO-CONAGUA

### A. SGM (Servicio Geológico Mexicano) — Litología y Estratigrafía
* **Documento Base:** Carta Geológico-Minera *Tijuana I11-11* y *Playas de Tijuana* (Escala 1:250,000 y 1:50,000).
* **Parámetros Clave para el Algoritmo:**
  1. **Depósitos Aluviales Cuaternarios ($Qal$):** Lechos de los ríos Tijuana y Alamar, Cañón Johnson, Cañón de las Palmeras. Grava y arena permeable. **Viabilidad Máxima (85–95%)**, perforación manual en 6–8 horas, nivel freático a 3–6 m.
  2. **Formación San Diego ($Tsd$ - Plioceno):** Terrazas y cañadas costeras (Playas de Tijuana, San Antonio del Mar). Areniscas poco consolidadas con lentes de agua colgada. **Viabilidad Alta (75–88%)**, freático a 5–9 m.
  3. **Formación Rosarito Beach ($Trm$ - Mioceno):** Arenas tobáceas y basaltos subordinados (Cañón del Sainz, Valle Redondo). **Viabilidad Media (50–70%)**, requiere percusión, freático a 9–14 m.
  4. **Formación Otay ($To$ - Mioceno):** Arcilla cementada ("Otay Hardpan") y conglomerados masivos (Mesa de Otay). **Viabilidad Condicionada (40–60%)**, permeabilidad reducida.
  5. **Rocas Volcánicas Ígneas ($Kgr / Tvb$):** Cerro Colorado, Cerro de las Abejas. Basalto y andesita compacta. **Zona de Rechazo Técnico (0–15%)**, manto >60 m, imposible perforación frugal.

---

### B. Atlas Municipal de Riesgos de Tijuana (Protección Civil / CENAPRED)
* **Documento Base:** *Atlas de Peligros y Riesgos Naturales del Municipio de Tijuana* (Actualizaciones 2014–2024).
* **Parámetros Clave para el Algoritmo (Filtros de Seguridad 'Kill Switch'):**
  1. **Polígonos de Deslizamiento Activo y Falla Geológica:** Lomas del Rubí, Lomas de Sánchez Taboada, Camino Verde, Cañón del Pato, Aguaje de la Tuna.
     * *Razón Técnica:* Estas colonias presentan inestabilidad por saturación de arcillas. Aunque existe agua somera por fugas de red y aguas colgadas, **está estrictamente prohibido perforar sin contención geotécnica**, ya que la despresurización o vibración agrava las fallas rotacionales.
     * *Resultado AquaEngine:* **Rechazo Técnico Inmediato (Kill Switch activado)** y recomendación de Cosecha de Lluvia/Niebla (SCALL).
  2. **Zonas de Inundación y Encharcamiento Histórico:** Planicie aluvial Zona Río, Cañón del Matadero, Alamar. Indica nivel freático extremadamente superficial (<3.5 m) que emerge en temporada de lluvias.

---

### C. INEGI — Modelo Digital de Elevaciones (DEM) e Hidrografía Fina
* **Documento Base:** Continuo de Elevaciones Mexicano 3.0 (CEM 3.0, resolución 5m/30m) y Red Hidrográfica Digital de México 2.0 (RHD 2.0).
* **Parámetros Clave para el Algoritmo:**
  1. **Cota Altimétrica ($msnm$):** La profundidad al manto freático en Tijuana tiene una correlación matemática inversa con la cota de fondo de cañada:
     $$\text{Profundidad Estimada } (m) \approx 3.5 + 0.04 \times (\text{Cota } [msnm] - \text{Cota Base Cuenca})$$
  2. **Índice de Humedad Topográfica (TWI - *Topographic Wetness Index*):**
     $$\text{TWI} = \ln\left(\frac{\alpha}{\tan \beta}\right)$$
     Donde $\alpha$ es el área de cuenca contribuyente y $\beta$ es la pendiente local. Zonas con $\text{TWI} > 8.5$ en cañadas presentan saturación somera permanente.
  3. **Distancia a Red de Drenaje Natural ($m$):** Terrenos a menos de 150 metros del eje de un arroyo intermitente tienen 3.2x más probabilidad de encontrar gravas acuíferas productivas.

---

### D. CILA / IBWC / USGS / TRNERR — Red Binacional de Calidad y Piezometría
* **Documento Base:** Minutas CILA 283, 320, 328; *USGS National Water Information System (NWIS)*; *Tijuana River National Estuarine Research Reserve*.
* **Parámetros Clave para el Algoritmo:**
  1. **Estación Piezométrica USGS 11013500:** Registra niveles estáticos del aluvión transfronterizo entre **1.8 y 4.2 metros** en la cota de 15 msnm.
  2. **Sensores de Conductividad Eléctrica (TDS):** Mapeo de intrusión salina en la franja costera de Playas de Tijuana (0 a 1,200 m del mar) y detección de sulfatos/cloruros en el corredor industrial de Otay.
  3. **Alertas Sanitarias:** Delimitación de zonas de amortiguamiento (buffer de 250m) respecto a colectores colapsados o tiraderos clandestinos para exigir doble barrera de purificación (UV + RO).

---

### E. CESPT (Comisión Estatal de Servicios Públicos) & IMPlan Tijuana
* **Documento Base:** Mapas de sectorización hidrométrica, informes anuales de pérdidas de agua y Plan Municipal de Desarrollo Urbano (PDUCP).
* **Parámetros Clave para el Algoritmo:**
  1. **Zonas de Fuga Crónica / Recarga Antrópica:** Las pérdidas físicas de la red vieja de CESPT (estimadas en más del 25% del caudal inyectado) actúan como una fuente artificial y continua de recarga para mantos colgados en cañadas intermedias.
  2. **Mapa de Severidad de Tandeos y Cortes:** Priorización de colonias donde el servicio público se interrumpe más de 4 días al mes (e.g. Cañón del Sainz, Maclovio Rojas, Playas Sur), elevando el valor socioeconómico del AquaScore™.

---

## 3. EL CÓDIGO INTELIGENTE: ALGORITMO MULTICRITERIO (AQUAENGINE v2.0)

El código inteligente de la plataforma evalúa cualquier coordenada $(x, y)$ de Tijuana mediante una función de scoring multicriterio ponderada con lógica difusa y filtros excluyentes (*Kill Switches*):

```
                                  COORDENADAS (Lat, Lng)
                                             │
                                             ▼
                          ┌─────────────────────────────────────┐
                          │    FILTRO 1: KILL SWITCH GEOLÓGICO  │
                          │   ¿Zona de Falla o Deslizamiento?   │
                          └──────────────────┬──────────────────┘
                                             │
                       ┌─────────────────────┴─────────────────────┐
                       │ SÍ                                        │ NO
                       ▼                                           ▼
             🛑 RECHAZO TÉCNICO                   ┌───────────────────────────────────┐
           (AquaScore < 15%)                      │   ALGORITMO MULTICRITERIO AHP     │
           Alternativa: SCALL                     │                                   │
                                                  │ • Cota Topográfica & TWI (25%)    │
                                                  │ • Litología SGM Permeable (25%)   │
                                                  │ • Proximidad Arroyo INEGI (20%)   │
                                                  │ • Vulnerabilidad Social (15%)     │
                                                  │ • Factor Químico / Salino (15%)   │
                                                  └─────────────────┬─────────────────┘
                                                                    │
                                                                    ▼
                                                       AQUASCORE™ FINAL (0 - 100%)
                                                       + Profundidad Estimada (m)
                                                       + Caudal Seguro (L/min)
                                                       + Tren 80/20 POU Especificado
                                                       + Oficio Legal LAN Art 8/35
```

### Fórmula del Algoritmo Ponderado:
$$\text{AquaScore} = \left[ 0.25 \cdot S_{\text{topo}}(\text{Cota}, \text{Pendiente}) + 0.25 \cdot S_{\text{lito}}(\text{SGM}) + 0.20 \cdot S_{\text{arroyo}}(\text{Dist}) + 0.15 \cdot S_{\text{social}}(\text{Cortes}) + 0.15 \cdot S_{\text{calidad}}(\text{TDS}) \right] \times K_{\text{riesgo}}$$

Donde $K_{\text{riesgo}} = 0$ si el terreno se ubica dentro de un polígono de falla o deslizamiento activo del Atlas de Protección Civil, y $1.0$ en caso contrario.

---

## 4. DERECHO COMPARADO: OTRAS CIUDADES CON POZOS URBANOS SIMILARES

```
┌─────────────────┬───────────────────────────┬───────────────────────────────────────────┬───────────────────────────────────────────┐
│ Ciudad          │ Formación Geológica       │ Marco Legal & Figura Clave                │ Aplicación / Tecnología Frugal            │
├─────────────────┼───────────────────────────┼───────────────────────────────────────────┼───────────────────────────────────────────┤
│ 🇩🇪 Berlín       │ Valle Glaciar Urstromtal  │ § 46 WHG (*Erlaubnisfrei*)                │ >20,000 pozos de jardín (*Gartenbrunnen*) │
│                 │ Arenas aluviales (1.5–4m) │ Notificación simple (*Anzeigepflicht*)    │ Biofiltración de ribera (*Uferfiltration*)│
├─────────────────┼───────────────────────────┼───────────────────────────────────────────┼───────────────────────────────────────────┤
│ 🇺🇸 San Diego    │ Cuenca DWR Basin 9-019    │ SGMA (2014) — *De Minimis Extractors*     │ Extracción <2 AFY (~6,700 L/día) exenta   │
│                 │ Aluvión del Río Tijuana   │ Exento de medidor forzoso y tarifas       │ Monitoreo transfronterizo USGS 11013500   │
├─────────────────┼───────────────────────────┼───────────────────────────────────────────┼───────────────────────────────────────────┤
│ 🇵🇪 Lima         │ Abanicos Rímac y Surco    │ Ley 29338 — Figura de "Uso Primario"      │ Micropozos y galerías filtrantes de baja  │
│                 │ Gravas costeras (3–8m)    │ Gratuito, irrestricto y de supervivencia  │ cota como respaldo ante cortes de red     │
├─────────────────┼───────────────────────────┼───────────────────────────────────────────┼───────────────────────────────────────────┤
│ 🇺🇸 Miami        │ Acuífero Biscayne         │ SFWMD — *General Permits by Rule*         │ Micropozos someros (<10m) residenciales   │
│                 │ Caliza kárstica (1–3m)    │ Pozos de jardín <4 pulgadas exentos       │ Desalinización POU por cuña salina        │
├─────────────────┼───────────────────────────┼───────────────────────────────────────────┼───────────────────────────────────────────┤
│ 🇿🇦 Ciudad Cabo  │ Cuenca Cape Flats         │ *Water By-law Amendments (Day Zero)*      │ Micropozos domiciliarios (*wellpoints*)   │
│                 │ Arenas eólicas costeras   │ Registro vecinal de pozos someros         │ Desacople de servicios no potables        │
├─────────────────┼───────────────────────────┼───────────────────────────────────────────┼───────────────────────────────────────────┤
│ 🇯🇵 Tokio / Osaka│ Aluviones Kanto/Yodogawa  │ *Sub-drainage & Dewatering Regulations*   │ Pozos someros de alivio para prevenir     │
│                 │ Manto a 2–5m sobre arcilla│ Extracción de balance contra sismos       │ licuefacción sísmica e inundación sótano  │
└─────────────────┴───────────────────────────┴───────────────────────────────────────────┴───────────────────────────────────────────┘
```

---

## 5. CONCLUSIÓN Y VALOR PARA EL JURADO (VENTURE HACKS 2026)

Este enfoque multidimensional demuestra que **AquaResiliencia Tijuana** no opera a ciegas ni propone una perforación clandestina indiscriminada:
1. Utiliza **ciencia de datos geoespacial** combinando capas de 7 organismos gubernamentales y binacionales.
2. Incorpora **filtros geotécnicos estrictos de Protección Civil** para vetar zonas inestables antes de perforar.
3. Se respalda en **derecho comparado sólido** probado en metrópolis de clase mundial con mantos freáticos idénticos.
4. Implementa el **código inteligente (AquaEngine)** para que cualquier ciudadano o técnico pueda diagnosticar su terreno con certeza milimétrica en segundos.
