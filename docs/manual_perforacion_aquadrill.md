# Manual de Perforación Hidráulica Frugal
## AquaDrill — Protocolo de campo

**Proyecto:** AquaResiliencia Tijuana
**Versión:** 0.3 — Revisado y completado con investigación técnica de respaldo (curado, tolerancia de desviación, granulometría de grava, protocolo de acceso, disponibilidad real de bombas)
**Fecha:** 25 de agosto de 2026
**Autor:** Pablo Campis

> **ESTADO DEL DOCUMENTO:** Borrador. Basado en el Caso 001 (Playas de Tijuana, 2022) y en la cotización de agosto 2026. Requiere validación en campo antes de considerarse operativo.

---

## Nota preliminar — Alcance y límites

Este manual describe un procedimiento técnico. **No constituye autorización para perforar.** El acuífero Tijuana 0201 está bajo veda desde 1965 y presenta disponibilidad oficial negativa (−1.664 hm³/año). Ninguna etapa de este manual debe ejecutarse sin haber completado la Etapa 0.

El método aquí descrito es **perforación por lavado (jetting) con percusión asistida**: el agua a presión hace el corte y arrastra el material; la percusión se usa únicamente para superar obstáculos puntuales.

---

# ETAPA 0 — Gate regulatorio

**Regla de paro: si esta etapa no está cerrada, no se arma el tripié.**

| Paso | Acción | Evidencia que debe existir |
|---|---|---|
| 0.1 | Verificar situación jurídica del predio y consentimiento escrito del propietario | Documento firmado |
| 0.2 | Consultar padrón REPDA para el polígono del sitio | Captura o respuesta oficial |
| 0.3 | Presentar preconsulta escrita ante CONAGUA describiendo la naturaleza y escala del punto | Acuse de recibo |
| 0.4 | Obtener determinación escrita o, en su defecto, acreditar la omisión con fecha | Oficio o acuse con plazo vencido |
| 0.5 | Documentar el carácter reversible y de bajo impacto de la obra | Ficha técnica del punto |

**Nota crítica:** "sondeo" no es una categoría que exima. Si se alcanza agua, se instala ademe y se bombea, la autoridad puede tratarlo como obra regulada. El expediente debe construirse asumiendo eso.

## 0.6 Actualización legal (agosto 2026) — trámite simplificado por bajo volumen

> Las solicitudes de concesión para uso doméstico en zonas rurales, y en general para cualquier uso cuyo volumen anual no sea mayor de **150 metros cúbicos por solicitante**, están exentas de presentar Memoria técnica y Documentación técnica de soporte — Artículo 32 del Reglamento de la Ley de Aguas Nacionales.

**Esto NO elimina la obligación de tramitar la concesión** (trámite CNA-01-003/004 ante CONAGUA sigue siendo obligatorio) — lo que elimina es la parte más cara y lenta del expediente: la memoria técnica y su documentación de soporte, que normalmente requiere contratar un ingeniero certificado.

**Cómo esto afecta el diseño del pozo:**

| Volumen objetivo | Régimen | Papeleo |
|---|---|---|
| ≤ 150 m³/año (≈ 411 L/día) | Simplificado | Sin memoria técnica |
| > 150 m³/año | Régimen normal | Memoria técnica completa |

El Caso 001 extrajo hasta 600 L/día (≈ 219 m³/año) — **por arriba del umbral**. Si el diseño del piloto se ajusta a un objetivo de ~400 L/día en vez de 600 L/día, el pozo entero califica para el trámite simplificado, reduciendo significativamente costo y tiempo de regularización legal. Vale la pena decidir esto de forma explícita al diseñar cada pozo piloto: **¿priorizar volumen (600 L/día, memoria técnica completa) o priorizar velocidad de regularización (≤400 L/día, trámite simplificado)?**

> **Contexto adicional (diciembre 2025):** la nueva Ley General de Aguas (DOF 11-dic-2025) exceptúa de sanción penal a quienes extraen agua para uso personal o doméstico (Art. 123 Bis 4), y crea un Programa de Regularización de Zonas de Libre Alumbramiento para que pozos sin título se incorporen al sistema de forma regularizable, no criminalizada. El marco legal se ha vuelto más favorable para este tipo de proyecto en los últimos meses.

`[ESPACIO PARA FOTO — captura del oficio o acuse de recibo de CONAGUA]`

---

# ETAPA 1 — Reconocimiento de sitio

Objetivo: **predecir** profundidad, material y calidad antes de perforar. Esta es la etapa que convierte al proyecto en método.

## 1.1 Lectura de farallones (exclusivo de zona costera)

En Playas de Tijuana los acantilados exponen la estratigrafía completa. Es un corte geológico gratuito.

**Procedimiento:**
1. Recorrer el frente de acantilado en el tramo de interés
2. Identificar escurrimientos ("lloraderos") en la cara del talud
3. Medir con flexómetro la altura del escurrimiento respecto a la superficie superior
4. Fotografiar y georreferenciar cada punto
5. Identificar la capa impermeable subyacente (arcilla/limolita) — es la base del lente
6. Registrar espesor aparente del estrato saturado
7. Trazar la continuidad lateral: ¿el escurrimiento aparece a lo largo de cientos de metros o es puntual?

**Salida esperada:** profundidad estimada del lente, espesor, y extensión lateral. Estos tres datos alimentan AquaMap.

**Referencia Caso 001:** escurrimientos observados entre 5 y 7 m.

`[ESPACIO PARA FOTO — panorámica del farallón con el escurrimiento marcado y el flexómetro visible]`

## 1.2 Censo de aprovechamientos vecinos

1. Identificar norias, pozos o captaciones existentes en un radio de 500 m
2. Solicitar acceso para medición
3. Registrar: profundidad total, nivel estático, uso, antigüedad
4. Medir conductividad eléctrica / TDS con equipo de mano
5. Registrar el testimonio del propietario sobre comportamiento estacional

**Protocolo de acceso (para no depender de la memoria en la puerta de cada casa):**

1. Llevar una carta breve impresa que explique el proyecto en un párrafo — la gente no abre la puerta a una explicación verbal larga
2. Pedir solo 10 minutos: medir y anotar, no interrogar
3. Ofrecer compartir el resultado de su propio pozo con el propietario — es el incentivo más simple y honesto
4. Nunca insistir si hay negativa; anotar la casa como "sin acceso" y seguir
5. Registrar la fecha de la visita — el TDS y el nivel estático cambian de temporada, así que un dato de censo tiene fecha de caducidad

## 1.3 Tamizado de calidad (screening)

**Equipo mínimo:** conductivímetro/TDS de mano.

| Lectura TDS | Interpretación preliminar | Decisión |
|---|---|---|
| < 1,000 mg/L | Dentro o cerca de norma | Continuar |
| 1,000 – 2,000 mg/L | Salobre bajo | Continuar con reserva; potable requiere membranas |
| > 2,000 mg/L | Salobre | Reclasificar sitio a usos no potables (80%) |

**Advertencia:** el TDS no detecta nitratos, amonio, metales ni contaminación bacteriológica. Es un tamiz, no una caracterización.

## 1.4 Discriminación de origen — ¿lente perchado o cuña salina?

Crítico en sitios costeros. Un lente perchado es recurso aprovechable; la cuña salina no lo es.

**Prueba de correlación mareal** (si existe pozo o punto de observación):
1. Medir nivel del agua cada hora durante 12–24 h continuas
2. Obtener tabla de mareas del mismo periodo
3. Graficar ambas series

| Resultado | Interpretación |
|---|---|
| El nivel oscila en fase con la marea | Conectado al mar — descartar para uso potable |
| El nivel permanece plano | Cuerpo aislado / perchado — prospecto válido |

**Indicadores secundarios de lente perchado:**
- Se abate rápido al bombear fuerte y recupera lentamente → almacenamiento finito, no conectado a reservorio infinito
- Presencia de capa impermeable visible en farallón
- Nivel estable interanual con extracción moderada

## 1.5 Criterios de descarte de sitio

Descartar si se cumple cualquiera:
- Correlación mareal positiva
- Ubicación aguas abajo de corredor industrial en el mismo gradiente de flujo
- Presencia de fosas sépticas activas a menos de 30 m
- TDS > 3,000 mg/L en pozos vecinos
- Sitio dentro de cauce activo o zona de inundación

---

# ETAPA 2 — Preparación

## 2.1 Verificación de inventario

Contrastar contra el documento de cotización. Confirmar que se cuenta con:
- Sarta completa: tramos suficientes para profundidad objetivo + 3 m de torre
- Guía de ABS 4" dentada
- Cabecera (cruz + adaptadores + tuerca unión)
- Motobomba, mangueras, tambos interconectados
- Barra de percusión, puntas, llaves Stillson
- Bentonita
- Adhesivos, limpiador, cinta teflón, grasa

`[ESPACIO PARA FOTO — inventario completo tendido antes de salir a campo]`

## 2.2 Preparación de la sarta

1. Cortar tubos de PVC de 6 m en tramos de 1.5 m
2. Limpiar cada extremo con limpiador multiuso
3. Cementar adaptador macho en un extremo y hembra en el otro
4. **Dejar curar el tiempo completo indicado por el fabricante** — este es el punto de falla más común
5. Numerar cada tramo con marcador indeleble
6. Verificar longitud real de cada tramo ensamblado y anotarla

> **Por qué numerar:** la suma de tramos ensamblados es tu única medida de profundidad. Un error aquí corrompe todo el registro litológico.

**Tiempos reales (Weld-On®, fabricante de referencia — tubería 1½"–2", que es la que usa este manual):**

| Etapa | 16°–38°C (clima templado/cálido) | 5°–16°C (clima fresco) |
|---|---|---|
| **Manejo cuidadoso** (se puede mover/acoplar el siguiente tramo) | 5 minutos | 10 minutos |
| **Curado antes de presión baja** (hasta 11 bar / 160 psi — el caso de la inyección de agua en jetting) | 30 minutos | 45 minutos |
| **Curado antes de presión alta** (11–22 bar) | 12 horas | 24 horas |

> **Corrección importante respecto a la versión anterior de este manual:** los tiempos de horas que se habían puesto aquí estaban sobreestimados. El cemento PVC para este diámetro fija en minutos, no en horas. Sí aplica la regla de **+50% de tiempo en clima húmedo o neblina costera** (frecuente en Playas de Tijuana), y el criterio de la etapa 3.2 — inyectar agua a presión por la sarta — cae en la categoría de "presión baja" del fabricante, así que basta con los 30–45 minutos de espera antes de someter la junta al flujo de la motobomba, no días.

> Fuente: tabla oficial de tiempos de curado de Weld-On Adhesives Inc. (weldon.com). Ajustar si se usa una marca distinta de cemento — cada fabricante publica su propia tabla y pueden variar.

`[ESPACIO PARA FOTO — tramos numerados y curados, listos para transportar]`

## 2.3 Preparación de puntas

**Punta dentada (uso general):**
1. Tomar niple galvanizado 2" × 6"
2. Cortar el extremo libre con esmeril formando dientes de ~10 mm
3. Alternar el ángulo de los dientes para mejorar el corte
4. Desbarbar el interior

**Punta helicoidal:** verificar acople NPT a 2" antes de salir a campo.

**Barra de percusión:** verificar que el diámetro no exceda 1.25" y que caiga libre por un tramo de prueba.

`[ESPACIO PARA FOTO — punta dentada terminada, primer plano]`

## 2.4 Preparación del tripié

1. Perforar los cuatro barrotes 2"×4"×12' a 20 cm de la punta
2. Pasar el eje/flecha
3. Montar polea en el eje
4. Verificar que el conjunto abre y cierra libremente
5. **Prueba de carga en el suelo antes de levantar:** colgar al menos 100 kg
6. Levantar y estabilizar; anclar las patas

`[ESPACIO PARA FOTO — tripié armado y anclado, tomada desde la base]`

## 2.5 Sistema de agua

1. Colocar tres tambos de 200 L en línea
2. Interconectar por la parte superior con niples y empaques (cascada de sedimentación)
3. Motobomba succiona del **tercer** tambo (el más limpio)
4. Retorno del pozo entra al **primer** tambo
5. Llenar con agua limpia
6. Confirmar suministro de reposición (toma o pipa)

> **Consumo esperado:** los 600 L de reserva pueden agotarse en menos de una hora en terreno permeable. Tener reposición confirmada antes de arrancar.

## 2.6 Preparación de lodo bentonítico

1. Preparar mezcla en el tambo de succión según dosificación del fabricante
2. Dejar hidratar antes de bombear
3. Función: estabilizar paredes, transportar recortes, sellar pérdidas de circulación

**Dato del Caso 001 (empírico, no calibrado con instrumento):** aproximadamente 20 kg de bentonita para unos 450 L de agua total usados en la perforación. Eso da una concentración aproximada de **44 g/L (~4.4% p/v)**.

> **Contraste con la práctica estándar:** el lodo bentonítico de perforación típico se dosifica entre 30 y 60 g/L (3–6% p/v). El dato del Caso 001 cae **dentro de ese rango**, más o menos a la mitad — señal de que, aunque fue una dosis a ojo, no estuvo lejos de lo que hubiera recomendado un manual técnico. Buen indicio de que el criterio de campo funcionó, aunque no haya sido calculado.
>
> **Para este manual, mientras no se calibre con más precisión:** partir de 40–50 g/L (que es justo donde cae el dato del Caso 001) y ajustar hacia arriba solo si el retorno no logra suspender los recortes o si hay pérdida de circulación en terreno muy permeable — el mismo criterio que ya usaste en 2022, ahora con un punto de partida documentado en vez de a ojo.

---

# ETAPA 3 — Perforación

## 3.1 Instalación de la guía

**Método validado en el Caso 001 — secuencia real de campo:**

1. **Excavación manual con pico y pala**, tan profundo como sea posible a mano antes de meter el tubo
2. Insertar el tubo ABS dentado y avanzarlo con **golpes ligeros controlados** hasta que ya no ceda con golpe de mano
3. **Rellenar el espacio anular** alrededor del tubo en superficie para estabilizarlo y evitar que se mueva lateralmente
4. **Conectar el sistema de inyección de agua a presión directamente al tubo guía** — este es el paso que aceleró todo lo demás: el jetting termina de hundir la guía mucho más rápido que la percusión sola

**Resultado documentado (Caso 001):** con esta secuencia, la guía alcanzó **~1.5 m (5 pies)** de profundidad antes de que el jetting tomara el control total del avance.

> **Por qué importa esta secuencia:** ahorra tiempo de percusión pura (la parte más lenta y desgastante) al usar la excavación manual para la capa superficial suelta, y usa el jetting — que ya vas a tener armado — en cuanto hay suficiente tubo enterrado para que el retorno se canalice bien. No hace falta hincar toda la guía a golpes antes de prender la motobomba.

**Checklist de verificación en cada paso:**
1. Posicionar el tubo en el punto exacto antes de excavar
2. Verificar verticalidad con plomada en dos ejes perpendiculares tras cada avance
3. Reverificar verticalidad después del relleno del espacio anular
4. Instalar ventanita, T y codo para canalizar retorno hacia el primer tambo antes de conectar la inyección

## 3.2 Ciclo de perforación

> **Qué es la sarta:** es la columna completa de tramos de PVC acoplados uno tras otro (Etapa 2.2) que va bajando hacia el pozo conforme se perfora. No es un material ni una pieza — es el nombre técnico de "la tubería completa que va descendiendo".

Ciclo repetitivo por cada tramo:

1. Acoplar tramo numerado a la sarta (rosca **solo con grasa**, sin cinta teflón — así se hizo en el Caso 001 y funcionó)

> **Nota técnica:** la sarta de perforación es temporal — se arma y desarma en cada pozo, no es el ademe definitivo. La grasa sola lubrica para armar/desarmar y tolera una fuga menor de retorno sin comprometer el avance; el teflón ayuda al sellado hermético pero no es indispensable aquí como sí lo sería en una tubería de presión permanente. Tenerlo a la mano como respaldo (ver inventario 2.1) por si una rosca en particular gotea más de lo normal.
2. Abrir inyección de agua
3. Aplicar peso y **rotación alternada** manual sobre la sarta
4. Descender conforme el terreno cede
5. Observar el retorno continuamente
6. Al agotar el tramo, cerrar inyección
7. **Registrar** (ver Etapa 4) antes de acoplar el siguiente

> ## ⚠️ ADVERTENCIA CRÍTICA — la sarta se puede quedar atrapada si no se mueve
>
> **Si se detiene el avance y la sarta queda quieta dentro del pozo por demasiado tiempo — especialmente si se corta la circulación de agua — la tierra la aprieta y puede volverse imposible sacarla.** El lodo/agua en movimiento es lo que mantiene el espacio anular abierto alrededor del tubo; en cuanto deja de circular, el terreno (sobre todo en formaciones sueltas o con arcilla) cierra ese espacio y la fricción sobre toda la longitud de la sarta puede superar por mucho lo que se puede jalar a mano o con el tripié.
>
> **Regla de campo:** si por cualquier motivo hay que detener la perforación antes de terminar el pozo (falla de equipo, fin de jornada, decisión de abortar el sitio), **sacar la sarta de inmediato, no dejarla parada**. Si no se puede sacar de inmediato, mantener circulación de agua aunque no se esté avanzando — es preferible gastar agua y tiempo de bomba a perder toda la sarta enterrada.
>
> Esto también aplica durante la Etapa 5 (percusión): al cerrar la inyección para percutir, no dejar pasar demasiado tiempo sin restablecer circulación entre tandas.

`[ESPACIO PARA FOTO/VIDEO — ciclo completo de perforación, útil también para el material de pitch]`

## 3.3 Control de verticalidad

Verificar con plomada **cada tres tramos** (cada 4.5 m). Una desviación temprana es corregible; una tardía obliga a reperforar.

> **Por qué importa tanto:** una bomba de 45–50 mm en ademe de 2" deja 5–6 mm de holgura. Un pozo desviado no admite la bomba.

**Tolerancia de desviación admisible (regla de trabajo, no norma oficial):**

| Profundidad acumulada | Desviación lateral máxima tolerable |
|---|---|
| Hasta 4.5 m (3 tramos) | 5 cm |
| 4.5 – 9 m (6 tramos) | 10 cm |
| 9 – 13.5 m (9 tramos) | 15 cm |

> Regla práctica: si la desviación lateral acumulada supera **~1% de la profundidad**, la holgura de 5–6 mm de la bomba ya está comprometida. Ahí aplica la Regla de paro N.° 3 del Anexo B — es más barato reubicar que forzar.

## 3.4 Manejo de pérdida de circulación

Si el retorno disminuye o desaparece:
1. Detener avance
2. Aumentar densidad de bentonita
3. Recircular sin avanzar hasta restablecer retorno
4. Si no se restablece, evaluar si se atravesó una zona muy permeable — **puede ser indicio de acuífero**

---

# ETAPA 4 — Registro litológico

**Esta etapa no es opcional. Es el producto principal del pozo, junto con el agua.**

## 4.1 Toma de muestra por tramo

**Problema real (no cubierto en la versión anterior):** el retorno que llega al primer tambo ya viene mezclado con agua, bentonita y sedimento de todo lo que se ha perforado hasta ese momento — tomar la muestra directamente del tambo da una muestra contaminada, no representativa de ese tramo específico.

**Método corregido — muestreo en el punto de salida, no en el tambo:**

1. Colocar un **tamiz o coladera fina** (malla de mosquitero o similar) justo donde el retorno sale de la guía/ademe, **antes** de que caiga al tambo — ahí es donde se atrapa el sedimento más fresco de ese tramo
2. Recolectar el material retenido en el tamiz apenas se note cambio de textura o cada vez que se agote un tramo
3. **Enjuagar la muestra con agua limpia** (de la reserva, no del tambo de retorno) para quitar la película de bentonita adherida — esto es indispensable, sin enjuague la bentonita cubre y disfraza el material real
4. Después del enjuague, evaluar textura y tamaño de grano — esto se conserva bien
5. Etiquetar con profundidad exacta y guardar en bolsa o frasco

> **Limitación honesta que hay que aceptar:** el **color** del retorno como indicador (columna "Color del retorno" en la ficha 4.2) es poco confiable con lodo bentonítico — la bentonita es gris y tiñe casi todo. Priorizar **textura, tamaño de clasto y velocidad de avance** como indicadores principales; usar el color solo como dato secundario, y solo después de enjuagar la muestra, nunca a partir del color del tambo.
>
> **Para el muestreo de calidad de agua (TDS, etc. — Etapa 6 y 11):** ese muestreo es distinto y **no debe tomarse del retorno en ningún caso** — ya se hace con bailer directo al estrato de interés (ver 6.2), que es agua no mezclada con el sistema de lodo. Esa parte del protocolo ya estaba bien; el problema era solo el muestreo litológico.

`[ESPACIO PARA FOTO — muestra etiquetada junto a su ficha de registro]`

## 4.2 Ficha de registro por tramo

| Campo | Anotar |
|---|---|
| Tramo n° | |
| Profundidad acumulada | m |
| Hora inicio / fin | |
| Material observado en retorno | arena / arcilla / grava / conglomerado / roca |
| Color del retorno | |
| Tamaño máximo de clasto | mm |
| Velocidad de avance | min/m |
| Presión / comportamiento de la bomba | |
| Retorno: normal / reducido / perdido | |
| Obstáculos | |
| Indicios de agua | |
| Observaciones | |

## 4.3 Interpretación en campo

> Usar principalmente textura, tamaño de clasto y velocidad de avance — el color es secundario y solo confiable **después de enjuagar la muestra tamizada** (ver 4.1). El color del tambo de retorno no sirve para esto.

| Observación | Interpretación probable |
|---|---|
| Avance se acelera + retorno se aclara | Posible estrato permeable / acuífero |
| Retorno se pierde súbitamente | Zona muy permeable o fractura |
| Avance se detiene, retorno con arena gruesa | Canto rodado |
| Muestra enjuagada gris/azulosa y plástica al tacto | Arcilla — posible base impermeable del lente |
| Cambio abrupto de textura o tamaño de clasto | Cambio de estrato — anotar profundidad exacta |

---

# ETAPA 5 — Manejo de obstáculos (percusión)

Aplicar únicamente cuando el avance se detiene y se descarta falla de equipo.

## 5.1 Procedimiento

1. **Cerrar la inyección de agua y apagar la motobomba**
2. Desconectar la manguera de la cabecera
3. Retirar la tapa superior de la cruz
4. Verificar que la barra cae libre por el interior de la sarta
5. Elevar la barra con la cuerda por la polea
6. Soltar en caída libre — **la barra golpea el obstáculo, nunca la tubería**
7. Repetir en tandas de 10–15 golpes
8. Restablecer circulación y evaluar avance
9. Alternar percusión / lavado hasta superar el obstáculo

## 5.2 Parámetros

| Parámetro | Valor de trabajo |
|---|---|
| Altura de caída recomendada | 1.0 – 1.5 m |
| Diámetro máximo de barra | 1.25" |
| Golpes por tanda | 10 – 15 |

> **Advertencia:** dentro de un tubo con agua y lodo, el fluido amortigua la caída. Alturas mayores no rinden energía proporcional. **Pendiente de calibrar en campo.**

## 5.3 "Piedra bailarina" — canto rodado

Un canto rodado no confinado gira o se desplaza en lugar de fracturarse.

**Ventaja del método:** la sarta de PVC confina la piedra y evita que escape lateralmente.

**Si tras 3–4 tandas no hay avance:**
- Rotar la sarta 90° y reintentar (cambiar punto de impacto)
- Evaluar bajar el ademe de 4" para confinar mejor
- Si la piedra excede el diámetro y desvía el pozo: **evaluar reubicar el punto**

**Criterio de paro:** si el pozo se desvía por encima de tolerancia, es más barato reubicar que insistir.

---

# ETAPA 6 — Detección de agua y protocolo de parada

**Regla central: DETENERSE ANTE EL PRIMER INDICIO DE AGUA.**

> **Lección del Caso 001:** se perforó hasta 8–9 m cuando probablemente existía agua aprovechable a 5–7 m. Perforar de corrido atraviesa el mejor estrato y asienta el pozo por debajo de él.

## 6.1 Indicios de agua

- Aceleración súbita del avance
- Aclaramiento del retorno
- Pérdida parcial de circulación
- Cambio en el sonido de la sarta
- Retorno con arena limpia y bien clasificada
- Coincidencia con la profundidad predicha por farallones

## 6.2 Protocolo al detectar indicio

1. **Detener avance inmediatamente**
2. Registrar profundidad exacta
3. Apagar la motobomba y cerrar inyección
4. Esperar 30 minutos sin operar
5. Medir nivel del agua dentro de la sarta (sonda o cámara)
6. Esperar 30 minutos más y volver a medir
7. **¿El nivel sube y se estabiliza por encima de la punta?** → hay aporte
8. Introducir cámara sumergible y documentar
9. Tomar muestra con bailer y medir TDS en el acto

`[ESPACIO PARA FOTO/VIDEO — imagen de cámara sumergible al primer indicio de agua]`

## 6.3 Decisión: ¿continuar o terminar aquí?

| Situación | Decisión |
|---|---|
| Nivel se estabiliza, buena recuperación, TDS aceptable | **Terminar aquí.** Es el estrato objetivo |
| Aporte marginal | Registrar y continuar con precaución; se puede volver |
| Nivel no sube | Falso indicio; continuar |
| TDS elevado | Registrar el estrato y continuar buscando uno más profundo o superficial |

> **Nunca atravesar un estrato productivo sin haberlo evaluado.** Una vez pasado, recuperarlo exige sellar el fondo.

## 6.4 Profundidad objetivo

| Referencia | Valor |
|---|---|
| Rango CONAGUA en aluvión Tijuana/Alamar (2013) | 4 – 15 m |
| Caso 001 (Playas) | 8 – 9 m |
| Farallones Playas — escurrimientos observados | 5 – 7 m |
| **Objetivo de diseño** | **detenerse al primer aporte, típicamente 5–10 m** |
| Profundidad máxima antes de replantear | 15 m |

---

# ETAPA 7 — Prueba de recuperación

Convierte la observación en dato de ingeniería.

## 7.1 Prueba básica (mínimo obligatorio)

1. Medir nivel estático inicial y anotar hora
2. Bombear a caudal constante conocido durante 30 min
3. Medir abatimiento cada 2 min los primeros 10 min, luego cada 5 min
4. Detener bombeo y anotar hora exacta
5. Medir recuperación con la misma cadencia durante 60 min
6. Graficar abatimiento y recuperación

**Salidas:** caudal sostenible aproximado, velocidad de recuperación, indicio de transmisividad.

## 7.2 Prueba escalonada (opcional)

Bombear a tres caudales crecientes, midiendo abatimiento estabilizado en cada escalón.

> **Advertencia:** llevar el pozo al límite lo estresa y puede arrastrar finos al ademe. Ejecutar solo si se requiere demostrar capacidad máxima.

## 7.3 Filosofía operativa

**Nunca operar al máximo. Operar siempre dentro de la zona demostrada.**

El Caso 001 sostuvo ~600 L/día durante tres meses sin abatimiento anual **porque no fue forzado**. Ese es un piso comprobado, no un techo — y para el caso de uso doméstico (350–500 L/día) es suficiente con margen.

---

# ETAPA 8 — Terminación del pozo

## 8.1 Diseño del ranurado

**Regla: el ranurado debe ser tan largo como el estrato productivo, no más.**

| Espesor del estrato | Longitud de ranurado |
|---|---|
| < 50 cm | 30 – 50 cm |
| 50 cm – 1 m | igual al espesor |
| > 1 m | 1 – 1.5 m |

> **Por qué:** un ranurado que excede el estrato capta también los estratos vecinos, que pueden ser de peor calidad. En un lente delgado, un ranurado de 1.5 m arruina el pozo.

**Ejecución:**
1. Marcar la zona de ranurado en el tramo de fondo
2. Cortar ranuras horizontales con esmeril y disco delgado
3. Ancho de ranura menor al tamaño del empaque de grava
4. Desbarbar interior y exterior
5. Cerrar el extremo inferior con tapón

`[ESPACIO PARA FOTO — tramo ranurado terminado, antes de bajarlo]`

## 8.2 Instalación del ademe definitivo

1. Bajar la sarta con el tramo ranurado ubicado **exactamente** en el estrato
2. Verificar profundidad por conteo de tramos
3. Confirmar verticalidad

## 8.3 Empaque de grava

1. Verter grava calibrada por el espacio anular
2. Cubrir el ranurado y ~50 cm por encima
3. Verificar nivel del empaque

**Función:** aumenta el radio efectivo del pozo y filtra finos. Es la solución barata al cuello de botella de transmisividad.

**Regla de dimensionamiento (criterio estándar de diseño de pozos):**

| Elemento | Regla |
|---|---|
| Tamaño de grava del empaque | 4 – 6 veces el D50 (tamaño medio de grano) del acuífero |
| Ancho de ranura en el ademe | Debe retener el 90% del empaque de grava — típicamente **1 – 2 mm** para grava fina/arena gruesa |
| Espesor mínimo del empaque anular | 5 cm alrededor de todo el ademe |

> Sin muestra de grano del acuífero (Etapa 4) no se puede calcular el D50 real. Con el registro litológico ya en marcha, esto deja de ser una suposición y se vuelve cálculo — otra razón por la que la Etapa 4 no es opcional. Mientras no haya dato de campo, usar grava comercial de 3–6 mm y ranura de 1.5 mm como punto de partida conservador.

## 8.4 Sello sanitario

**Obligatorio.** Evita que contaminación superficial descienda por el espacio anular.

1. Colocar sello de bentonita por encima del empaque
2. Rematar con brocal de concreto en superficie
3. Sobresalir el ademe al menos 30 cm sobre el nivel del terreno
4. Tapa hermética con respiradero

> Sin sello sanitario, un lente perchado se contamina con lo que se infiltre en la superficie inmediata.

`[ESPACIO PARA FOTO — brocal terminado con tapa hermética]`

## 8.5 Retiro de la guía

Recuperar el tubo ABS 4" para reutilización en el siguiente pozo.

---

# ETAPA 9 — Desarrollo del pozo

Objetivo: retirar finos y lodo bentonítico del entorno del ranurado.

1. Bombear y detener en ciclos cortos (pistoneo)
2. Repetir hasta que el agua salga clara
3. Registrar tiempo y volumen hasta clarificación
4. **No pasar a operación normal hasta lograr agua clara**

`[ESPACIO PARA FOTO — comparación de turbidez: primer bombeo vs. agua clarificada]`

---

# ETAPA 10 — Instalación de bomba

1. Verificar diámetro exterior real de la carcasa (≤ 45–50 mm)
2. Verificar verticalidad del pozo antes de bajar
3. Instalar cable de seguridad independiente de la manguera
4. Bajar la bomba sin forzar; **si encuentra resistencia, detener**
5. Posicionarla por encima del fondo, nunca apoyada
6. Conectar cable de uso rudo y fuente de 12 V CD
7. Sellar pasos de cable en la tapa
8. Prueba de arranque y medición de caudal real

## 10.1 El cuello de botella real: no hay bomba de consumo doméstico que quepa en 2"

Investigación de mercado (agosto 2026): las bombas sumergibles 12V CC de venta común (Amazon, tiendas de energía solar) están hechas para pozos de **3.5" a 4" o más** — su carcasa no entra en un ademe de 2". Las únicas bombas de 12V que sí caben en diámetros angostos (p. ej. Solinst 12V modelo 415, hasta 36.5 m de profundidad, hasta 13.5 L/min) son equipo de **muestreo ambiental/monitoreo**, no bombas de uso doméstico continuo — existen, pero su costo y disponibilidad en México no están confirmados.

**Esto deja dos rutas reales, no una:**

| Ruta | Qué implica |
|---|---|
| **A. Ademe de 3" en vez de 2"** | Cambia diámetro de guía, de sarta base y de cotización. Abre acceso a bombas domésticas 12V estándar (3.2 GPM / 12 LPM, ~$2,000–$4,000 MXN según proveedor) |
| **B. Bomba de monitoreo angosta (tipo Solinst) en ademe de 2"** | Mantiene el diseño actual, pero el costo por unidad probablemente supera varias veces el de una bomba doméstica — hay que cotizar directamente con el fabricante/distribuidor |

> **Pendiente de decisión:** esto no se puede cerrar sin cotizar la Ruta B en México o sin decidir si el ademe de 3" es aceptable para el resto de la cadena de perforación (guía, tripié, sarta). Es el hallazgo más importante de esta revisión — cambia el diseño del pozo, no solo la compra de un componente.

## 10.2 Régimen de operación

**Bombeo intermitente**, no continuo.

Para 600 L/día con bomba de bajo caudal: ciclos cortos distribuidos a lo largo del día, permitiendo recuperación entre ciclos. Descarga a cisterna desacoplada.

---

# ETAPA 11 — Muestreo y calidad

**El pozo es también un nodo centinela. Muestrear no es opcional.**

## 11.1 Muestreo inicial

Tomar después del desarrollo, no antes.

| Parámetro | Método | Prioridad |
|---|---|---|
| TDS / conductividad | Campo | Inmediata |
| pH | Campo | Inmediata |
| Nitratos | Campo o laboratorio | Alta |
| Amonio | Laboratorio | Alta |
| Coliformes | Laboratorio | Alta |
| Hierro, manganeso | Laboratorio | Media |
| Sulfatos, cloruros | Laboratorio | Media |
| Metales pesados | Laboratorio | Según entorno |

> **Perfil de riesgo por tipo de sitio:**
> - Lente perchado costero → nitratos y bacteriológico (recarga local, fosas sépticas)
> - Aluvión de cauce urbano/industrial → sales, amonio, metales, solventes

## 11.2 Clasificación de uso

Con los resultados, asignar el sitio a la arquitectura 80/20:
- Fracción potable (con tren de tratamiento definido por la química real)
- Fracción de usos secundarios y regeneración

---

# ETAPA 12 — Monitoreo continuo

| Variable | Frecuencia |
|---|---|
| Nivel estático | Semanal |
| Volumen extraído | Diario |
| TDS | Mensual |
| Bacteriológico | Trimestral |
| Caracterización completa | Semestral |

**Medición en dos temporadas** (seca y de lluvia) antes de declarar un sitio como viable de forma definitiva.

---

# ETAPA 13 — Seguridad

- Lentes y guantes obligatorios en corte, esmerilado y percusión
- **Nadie bajo la carga suspendida** durante izaje o percusión
- Verificar anclaje del tripié antes de cada jornada
- Motobomba en área ventilada; nunca en espacio confinado
- Combustible almacenado lejos del área de trabajo
- Conexiones eléctricas de 12 V protegidas de humedad
- Pozo **siempre tapado** cuando no se opera — riesgo de caída de objetos y de personas

---

# ETAPA 14 — Cierre y abandono

Si el sitio se descarta:
1. Retirar bomba y equipo recuperable
2. Rellenar con material inerte
3. Sellar con bentonita y concreto en superficie
4. Documentar el cierre con fotografía y coordenadas

> Un pozo abandonado sin sellar es una vía directa de contaminación al acuífero. El cierre es parte del protocolo de bajo impacto.

---

# ANEXO A — Pendientes de este boceto

**Resueltos en esta revisión (agosto 2026):**

- [x] Granulometría del empaque de grava vs. ancho de ranura — ver 8.3
- [x] Tiempos de curado por tipo de cemento y temperatura ambiente — ver 2.2
- [x] Tabla de tolerancia de desviación vertical admisible — ver 3.3
- [x] Protocolo de acceso y medición en pozos de terceros — ver 1.2
- [x] Investigación de modelo de bomba — **no se resolvió con una respuesta simple**: ver 10.1. El hallazgo real es que no hay bomba doméstica de carcasa ≤50 mm en el mercado de venta común; hay que decidir entre agrandar el ademe a 3" o cotizar bombas de monitoreo angostas tipo Solinst.

**Pendientes que solo se resuelven en campo (nadie más que tú tiene el dato):**

- [x] Dosificación de bentonita usada en el Caso 001 — ~44 g/L (4.4% p/v), dato empírico dentro del rango estándar. Ver 2.6
- [ ] Calibración empírica: tamaño máximo de canto rodado rompible por percusión
- [ ] Resultado de la prueba de correlación mareal en el Caso 001
- [ ] Fotografías de campo en cada uno de los espacios marcados en el documento

**Pendiente de decisión de diseño (nueva, surge de esta revisión):**

- [ ] Ademe de 2" (bomba especial, más cara) vs. 3" (bomba estándar, cambia cotización) — ver 10.1

---

# ANEXO B — Resumen de reglas de paro

| N° | Regla |
|---|---|
| 1 | Sin Etapa 0 cerrada, no se perfora |
| 2 | Detenerse ante el primer indicio de agua y evaluar |
| 3 | Si la desviación vertical excede tolerancia, reubicar |
| 4 | Si el TDS descalifica el uso previsto, reclasificar el sitio |
| 5 | No operar el pozo por encima del caudal demostrado |
| 6 | Si la bomba encuentra resistencia al bajar, detener |
| 7 | No pasar a operación sin desarrollo completo |
| 8 | Correlación mareal positiva descarta el sitio para uso potable |
| 9 | Si se detiene el avance, sacar la sarta de inmediato o mantener circulación — nunca dejarla quieta sin agua |

---

*Documento de trabajo del proyecto AquaResiliencia Tijuana. Sujeto a revisión tras la primera aplicación en campo.*
