# Resumen ejecutivo — Investigación de NAF y subsuelo en Tijuana (fuentes públicas)

**Fecha:** 14 de septiembre de 2026
**Alcance:** Minería de repositorios públicos abiertos (UMAI, IMPLAN, SEMARNAT/SINAT, DOIUM, CONAGUA/REPDA, SMIG, FLACSO, UABC, CICESE, GEOS/UGM) — sin solicitudes formales de transparencia.
**Regla aplicada en toda la investigación:** ningún dato se reporta sin haber sido leído directamente en el documento fuente. Donde no se encontró información real, se documenta la brecha en vez de inventarla.
**Archivos detallados que respaldan este resumen:**
- `REGISTROS_NAF_PLAYAS_TIJUANA_FUENTES_PUBLICAS.json`
- `REGISTROS_NAF_TIJUANA_TODAS_DELEGACIONES_FUENTES_PUBLICAS.json`
- `REGISTROS_NAF_TIJUANA_COLEGIOS_REPDA_PERMISOS.json`
- `REGISTROS_NAF_TIJUANA_FUENTES_ALTERNAS_SIN_SOLICITUDES.json`
- `REGISTROS_NAF_TIJUANA_FLACSO_TRANSFRONTERIZO.json`
- `BORRADORES_SOLICITUDES_TRANSPARENCIA_NAF_TIJUANA.md` (borradores no presentados, por decisión del usuario)

---

## 1. El hallazgo más accionable: Playas de Tijuana tiene suelos marinos colapsables

Fuente: PDUCPT 2002-2025, "Diagnóstico. Medio Físico Natural" (documento oficial, Tabla 6).

> "Suelos marinos colapsables — Meseta marina de Playas: [...] susceptibles al fenómeno de tubificación y colapsamiento. En condición seca son fuertes y estables, pero al sustraerse agua se encogen y sufren contracciones."

**Implicación:** para cimentación en Playas de Tijuana, el riesgo geotécnico dominante no es solo "¿a qué profundidad está el NAF?", sino que **cualquier cambio de humedad del suelo (bombeo, fuga, abatimiento de obra) puede inducir colapso/asentamiento diferencial**, independientemente de la cifra exacta de NAF. El mismo documento ubica ahí la Formación Limolita-arenisca (origen marino, poco consolidada, "estratos débiles").

---

## 2. Datos cuantitativos reales de nivel freático (NAF)

| Fuente | Dato | Verificado |
|---|---|---|
| CONAGUA, *Disponibilidad Acuífero Tijuana 0201* (2013, campo 2009-2010) | Nivel estático regional: 2-22 m según topografía. Cerca de la ciudad ~18 m; cauce Río Tijuana ~17 m; Arroyo Alamar ~11 m | ✅ Leído completo |
| Mismo documento | Evolución 2009-2010: recuperación hasta 1.5 m (centro), abatimiento hasta 0.3 m (noreste) | ✅ |
| Delgado Argote et al., GEOS 2017 (artículo arbitrado) | Fraccionamiento Valle del Sur: NAF varía de **3 a 18 m en menos de 200 m** de distancia (geoeléctrica); en un punto el agua aflora en superficie | ✅ Leído completo |
| Atlas de Riesgos IMPLAN 2024 | NAF regional 4-15 m (2013, cita de CONAGUA) — mismo dato que el punto 1, de segunda mano | ✅ |
| Sánchez et al. 2016, *Journal of Hydrology* (citando CONAGUA 2008a) | Afirma descenso de 5 m/año durante 20 años "al sur de Tijuana" | ⚠️ **No confiable** — contradice la edición 2013 del mismo estudio CONAGUA (que reporta cambios de 0.3-1.5 m) y la declaración oficial de "sin déficit". No se pudo verificar contra el documento 2008 original. No usar sin confirmación adicional. |

**Dato transversal importante:** en NINGÚN documento oficial revisado existe una cifra de NAF específica para Playas de Tijuana — el polígono legal del Acuífero Tijuana sí llega hasta la costa de Playas ("línea de bajamar"), pero la red real de monitoreo piezométrico (31 km²) se concentra en los cauces de los ríos Tijuana y Alamar. El propio Atlas 2024 reconoce esta carencia de datos fuera de esa zona.

---

## 3. Pozos y usuarios reales de agua subterránea

- **385 títulos de pozos** registrados en el municipio de Tijuana según la API pública de CONAGUA (REPDA), con folio, coordenadas y volumen — ninguno literalmente en la franja costera de Playas (el más cercano está a 3.7 km).
- **180 pozos** según censo citado en tesis FLACSO (2020) basada en datos CESPT/REPDA 2020, concentrados en el subálveo del Río Tijuana.
- **8 usuarios industriales/comerciales identificados por nombre** con volúmenes reales (Coca-Cola/Fuerte, Bonafont, Hipódromo de Agua Caliente, Pasteurizadora Jersey, inmobiliarias del Grupo ICA, etc.)
- **Limitación confirmada:** ninguna base de datos pública de CONAGUA incluye nivel estático/dinámico por pozo de forma consultable masivamente — ese dato, si existe, está solo en el expediente escaneado individual de cada título.

---

## 4. Expedientes y licencias individuales verificados

- **PT2022A45** — única licencia de edificación en Playas de Tijuana localizada y verificada (2 sótanos, DRO real con cédula, Baja Malibú Sección Lomas).
- **02BC2025TD039** — MIA completa (350 páginas) del Malecón de Playas de Tijuana, **rechazada por SEMARNAT** (21-abr-2026), con consultora real (Geomar Consultores S.C.) y sondeos con capas de arenisca/limolita ~3 m de espesor promedio.
- **02BC2023TD020** — expediente del desarrollo "Civantia" (Sección Rivera) confirmado, pero sin acceso a su Capítulo IV.
- Evidencia periodística (no oficial) de que la cimentación del malecón usa pilotes de 6-8.5 m, adjudicados de forma directa (no licitación pública).

---

## 5. Alertas de datos fabricados detectados y corregidos

Durante la investigación se identificaron y descartaron **múltiples intentos de alucinación**, incluyendo en el propio proyecto:

1. El documento previo del proyecto `FUENTES_OFICIALES_CARTOGRAFIA_Y_CAPAS_ALTERNATIVAS.md` combina datos reales con una carta SGM inexistente, una estación USGS tergiversada (es de aforo superficial, no piezométrica, inactiva desde 1982), y porcentajes/fórmulas sin fuente rastreable. **Sigue sin corregirse** — quedó pendiente tu decisión.
2. Un folio de licitación DOIUM usado como ejemplo en la solicitud original resultó ser inexistente.
3. Varios resúmenes automáticos de buscador asociaron incorrectamente documentos de Ensenada/Mexicali a Tijuana, y un acta de cabildo a un tema que no contenía.
4. El dato de "5 m/año" (sección 2) se trata con la misma cautela por no poder verificarse contra la fuente primaria.

---

## 6. Lo que se buscó y NO se encontró (honesto)

- El estudio completo pozo-por-pozo de Moro Ingeniería (2009-2010) — solo existe en catálogo físico de CONAGUA, sin versión digital.
- Nivel estático individual de los 385 títulos REPDA — no está en ninguna base consultable; el formulario de consulta individual tuvo fallas técnicas en esta sesión.
- Tres papers de la SMIG sobre Tijuana (García Fons 1988, Santoyo/Montañez, Rocha) — confirmados como reales pero bloqueados sin membresía; no existen en ningún repositorio abierto ni tienen DOI.
- Ninguna MIA de desarrollos verticales fuera de Playas (Zona Río, Otay) con Capítulo IV accesible.
- Ningún permiso público de descarga de agua de bombeo/achique de obra (ni CONAGUA ni CESPT publican los otorgados).

---

## 7. Recomendación

Para el dictamen técnico del proyecto, los datos más sólidos y defendibles son: (a) la clasificación oficial de suelos colapsables en Playas (sección 1), (b) la piezometría regional de CONAGUA 2013 (sección 2, primeras 3 filas), y (c) el dato empírico de variabilidad extrema de Valle del Sur (GEOS 2017). El resto de los hallazgos (pozos/usuarios, expedientes) sirve como contexto institucional pero no sustituye una medición directa en el sitio específico de interés.
