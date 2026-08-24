
> 💡 **PLAN MAESTRO AQUARESILIENCIA TIJUANA Guía integral para el equipo: contexto, evidencia, arquitectura 80/20, hacks legítimos, estrategia del hackathon y ruta de ejecución**

**Documento de incorporación y toma de decisiones para el equipo**

**Versión 1.0 | Fecha de corte: 11 de agosto de 2026**


> 💡 **MENSAJE MAESTRO Ya demostramos que una perforación frugal puede alcanzar agua en un sitio de Tijuana. El proyecto ahora consiste en convertir ese hallazgo en una plataforma capaz de decidir dónde funciona, qué agua encuentra, para qué usos es segura, cuánto cuesta tratarla y bajo qué reglas puede autorizarse. [S1/S2]**


> 💡 **DECISIÓN ACTUAL CONTINUAR → DOCUMENTAR → MEDIR → PEDIR DATOS → PROBAR TRATAMIENTO → OBTENER RUTA ESCRITA → ENSAYAR → PILOTEAR → ESCALAR SÓLO SI LA EVIDENCIA LO PERMITE.**

*Este documento no está escrito para especialistas. Cada concepto técnico o jurídico se explica antes de utilizarse.*


> 💡 **INICIO RÁPIDO: LO QUE TODO INTEGRANTE DEBE COMPRENDER**


# Doce ideas esenciales


> 💡 **1. Existe un prototipo real. En un sitio de Tijuana se reportó agua a 8–9 m, después de aproximadamente 8 h, con un caudal instantáneo cercano a 2 L/min. [S2]**


> 💡 **2. El prototipo prueba posibilidad, no generalidad. No demuestra que funcione en toda la ciudad, que el caudal sea sostenible, que el agua sea potable ni que una nueva perforación esté autorizada. [S1/S2]**


> 💡 **3. La profundidad tiene respaldo histórico. CONAGUA documentó niveles someros en depósitos aluviales de Tijuana y Alamar; el resultado del prototipo es geológicamente plausible en ciertos sitios. [S4]**


> 💡 **4. El agua es heterogénea. La calidad cambia por zona, profundidad, temporada y fuentes de contaminación. No existe una única “agua de Tijuana”. [S4/S11/S12]**


> 💡 **5. Potabilizar puede costar más que perforar. Sales, amonio, hierro, materia orgánica o solventes pueden exigir pretratamiento y membranas. [S5/S11]**


> 💡 **6. La arquitectura 80/20 reduce el problema. De 350–600 L/día, sólo 70–120 L/día se diseñan para beber y cocinar; el resto se clasifica y acondiciona según usos seguros. [S5]**


> 💡 **7. El 80% no queda “sin tratar”. El agua de sanitarios o limpieza también debe cumplir condiciones de seguridad definidas por su riesgo y contacto. [S5]**


> 💡 **8. El acuífero 0201 tiene déficit administrativo. La disponibilidad publicada es negativa; no podemos presentar el proyecto como extracción masiva adicional sin control. [S3]**


> 💡 **9. “Sondeo” no es una palabra mágica. Si se alcanza agua, se instala ademe, se bombea o se deja una captación funcional, la autoridad puede tratarlo como una obra regulada. [S6/S7]**


> 💡 **10. Los hacks deben ser legítimos. Usaremos datos, transparencia, muestreo de pozos existentes, geofísica no invasiva y una consulta escrita para reducir costo y riesgo. [S9/S10/S14]**


> 💡 **11. El hackathon financia validación, no promesas. El entregable es evidencia, un módulo 80/20, un protocolo regulatorio y una decisión; no necesariamente varios pozos terminados. [S18]**


> 💡 **12. El amparo es reserva, no punto de partida. Primero se construye un expediente serio y se pide una respuesta formal; sólo un acto concreto permite evaluar litigio estratégico. [S7/S17]**

*La plataforma avanza por gates: cualquier etapa puede detener, rediseñar o reubicar el proyecto.*


> 💡 **MAPA DEL DOCUMENTO**


# Cómo utilizar este plan

Las primeras secciones explican el proyecto para personas sin experiencia técnica. Las secciones centrales contienen la evidencia, los riesgos y los hacks legítimos. Las últimas secciones convierten la idea en trabajo concreto para el hackathon y los siguientes 90 días.


| Sección | Contenido |
| --- | --- |
| I | El problema y el origen del proyecto |
| II | Qué es el sistema y cómo funciona |
| III | Prototipo: qué comprobó y qué falta |
| IV | Hidrogeología y calidad del agua en Tijuana |
| V | Arquitectura 80/20 y potabilización |
| VI | Marco legal explicado sin tecnicismos |
| VII | Hacks legítimos y zonas grises aprovechables |
| VIII | Plan específico para Venture Hacks |
| IX | Plan maestro por fases y gates |
| X | Roles del equipo, riesgos y decisiones |
| XI | Fuentes y enlaces |
| REGLA DE LECTURA Los cuadros verdes describen oportunidades; los amarillos muestran decisiones pendientes; los rojos son límites o condiciones de paro; los bloques “HACK” señalan ventajas legítimas para el concurso. |  |



> 💡 **I. EL PROBLEMA Y EL ORIGEN DEL PROYECTO**


# 1. El problema que queremos resolver

Tijuana depende de infraestructura regional compleja y enfrenta presión hídrica. Para una familia, el problema no se expresa en hectómetros cúbicos: se expresa en interrupciones, almacenamiento insuficiente, compra de agua, dependencia de pipas y falta de una reserva doméstica confiable. El proyecto busca una fuente local de respaldo, no sustituir de inmediato a toda la red pública.


> 💡 **PROBLEMA EN LENGUAJE SENCILLO Cuando la red no entrega agua de manera suficiente o continua, una familia necesita una reserva. La pregunta del proyecto es si, en ciertos sitios, una fuente subterránea somera puede alimentar lentamente un tanque y reducir la vulnerabilidad del hogar.**


# 2. Cómo nació la idea

El desarrollador realizó una perforación experimental con tubería de PVC, agua en circulación, bentonita y una motobomba comercial reutilizable. El método evitó una perforadora convencional y encontró agua en un sitio de Tijuana. Ese experimento convirtió la idea en una hipótesis verificable.** [S2]**


# 3. El proyecto verdadero


> 💡 **NO ES “Un pozo barato que cualquier persona puede perforar libremente”.**


> 💡 **SÍ ES Una plataforma experimental de resiliencia hídrica doméstica que integra selección de sitio, perforación frugal, extracción lenta, almacenamiento, medición, tratamiento según uso, controles sanitarios, límites ambientales y una ruta regulatoria.**


# 4. Por qué encaja en un hackathon

Un hackathon no exige que todo esté resuelto. Exige convertir una necesidad real en una solución demostrable, identificar restricciones y producir un MVP que reduzca incertidumbre. La ventaja del proyecto es que ya existe un prototipo; la tarea del equipo es construir la capa científica, regulatoria, sanitaria y económica que le falta.


> 💡 **HACKATHON ≠ ATAJO ILEGAL Aquí “hack” significa rediseñar el problema, utilizar mejor la información disponible, construir un experimento mínimo y descubrir una ruta regulatoria reproducible. No significa ocultar una perforación ni cambiarle el nombre para evitar permisos.**


> 💡 **II. QUÉ ES EL SISTEMA Y CÓMO FUNCIONARÍA**


# 1. Arquitectura física

Equipo móvil reutilizable que perfora únicamente durante la instalación.

Punto de captación o de observación construido con materiales y sellos adecuados al propósito.

Bomba de extracción pequeña que trabaja a bajo caudal.

Tinaco o cisterna que desacopla la extracción lenta del consumo instantáneo.

Medidor de volumen y registro de horas de operación.

Dos líneas de agua físicamente separadas: potable y uso diferenciado.

Protocolo de análisis, mantenimiento, cierre y suspensión.

*En el escenario de 350–600 L/día, la línea potable sería de 70–120 L/día.*


# 2. Por qué usar almacenamiento

El prototipo reportó aproximadamente 2 L/min, equivalentes a 120 L/h como caudal instantáneo. Si ese caudal fuera sostenible —algo todavía no demostrado—, producir 350, 500 o 600 L requeriría aproximadamente 2.9, 4.2 o 5 horas de bombeo acumulado por día. El tanque permite extraer lentamente y usar el agua después. **[S2]**


> 💡 **DATO NO CONFUNDIR 2 L/min observados no equivalen todavía a 2 L/min sostenibles. Se necesita medir nivel estático, abatimiento, nivel dinámico, recuperación y comportamiento por temporada.**

Aaui yo agregaría que hice pruebas a lo largo de todo el anio y siempre hubo y ha habido,  pero nunca lo trabaje mas de 3 dia sseguidos… cada que juntaba 600 litros de agua pues le paraba.   No era mi intención demostrar que no se acaba porque justo para  conocer el limite hay que acabasrselo para dsaber hasta donde.  Esa es la razón por la que el pensamiento mecanico dualista se vuelve un bloqueo.  En cambio un enfoque mas holistico no necesita demostrar que no se acaba,  simplemente no debe de acabárselo y 600 lts por dia es suficiente.


# 3. Glosario mínimo


| Término | Explicación sencilla |
| --- | --- |
| Nivel freático | Superficie superior del agua subterránea en una zona no confinada. |
| Nivel estático | Profundidad del agua después de dejarla recuperar sin bombeo. |
| Nivel dinámico | Profundidad del agua mientras se bombea. |
| Abatimiento | Descenso del nivel causado por la extracción. |
| Recuperación | Tiempo y forma en que el nivel regresa después de detener el bombeo. |
| Caudal sostenible | Volumen que puede extraerse sin vaciar progresivamente el punto ni causar daño. |
| Piezómetro | Punto diseñado principalmente para observar nivel o calidad; sigue siendo una obra que puede requerir regulación. |
| Ademe | Tubería que sostiene y protege el interior de la perforación. |
| Sello sanitario | Barrera alrededor del ademe para impedir que agua contaminada de superficie descienda por el espacio anular. |
| Potable | Agua que cumple los límites aplicables para uso y consumo humano. |
| Uso diferenciado | Uso no destinado a beber o cocinar, definido después de evaluar riesgos. |



> 💡 **III. PROTOTIPO: QUÉ COMPROBÓ Y QUÉ FALTA**


# 1. Datos reportados


| Variable | Resultado | Lectura correcta |
| --- | --- | --- |
| Profundidad | 8–9 m | Dato reportado del prototipo |
| Tiempo | ~8 h | Éxito en primer intento; no usar como promedio |
| Caudal | ~2 L/min | Instantáneo; falta sostenibilidad cambiaria a falta registro |
| Método | PVC + circulación/recirculación + bentonita | Parentesco con wash boring / jetting |
| Equipo | Motobomba gasolina ~6 HP, ~250 L/min | Falta marca, modelo y curva |
| Costo | ~$10,000 MXN | Costo experimental; no sistema terminado |



# 2. Lo que ya demuestra

En al menos un sitio fue posible alcanzar agua con una técnica frugal. **[S2]**

La maquinaria principal puede ser un activo reutilizable y no debe cargarse completa a cada vivienda. **[S2]**

El método merece documentarse y probarse con instrumentación formal. **[S2]**


# 3. Lo que no demuestra

Que cualquier colonia tendrá agua a 8–9 m.** [S1]**

Que el agua pertenece a una unidad desconectada del acuífero administrado. **[S1]**

Que el caudal puede mantenerse diariamente. **[S1]**

Que el agua es potable o segura para sanitarios. **[S1]**

Que una nueva perforación puede realizarse sin autorización. **[S1]**

Que el costo terminado seguirá en $10,000 MXN. **[S1]**

*El prototipo coincide con rangos históricos, pero “profundidad a la que apareció agua” no es todavía un nivel estático formal.* **[S2/S4]**


# Aqui me falta incluir que estuve usando durante un tiempo de meses esa agua para regar mis piletas de cultivo de langostinos, sin embargo por falta de recursos eventualmente tuve que abandonar el proyecto… actualmente aun hay algunos langostinos vivos en las piletas, a la suerte de dios y las lluvias y estaciones. Anexo fotos de cuando estaba a todo calor. I videos demostrando


# https://photos.fife.usercontent.google.com/pw/AP1GczPwWxadof8ctl4TyOn3Jlqec1-HHUGgZEJGQxIlCPfHuO3O40o-bVZ4Ww=w560-h747-s-no-gm?authuser=0

ver videos anexos.  De el cultivo.  Aunque no es el proyecto principal es complementario y demuestra que no fue mortal el agua jaja .. curioso es que los langostinos necesitam un poco de agua salada.


# 4. Expediente que debemos reconstruir

Coordenadas y elevación del sitio.

Fecha y fotografías originales.

Secuencia de perforación y perfil de materiales por profundidad.

Diámetro, tipo y longitud de PVC.

Marca y modelo exactos de la motobomba.

Horas efectivas, combustible y número de operadores.

Método de medición de los 2 L/min.

Estado actual del punto, nivel y calidad disponible.


> 💡 **IV. HIDROGEOLOGÍA Y CALIDAD DEL AGUA EN TIJUANA**


# 1. Acuífero administrativo Tijuana 0201

CONAGUA publica para Tijuana 0201 una recarga media anual de 19.5 hm³, una descarga natural comprometida de 4.6 hm³, un volumen de extracción registrado de 16.564020 hm³ y una disponibilidad media anual de −1.664020 hm³. Un valor negativo significa déficit administrativo para volúmenes adicionales.** [S3]**

*Balance oficial publicado para el acuífero Tijuana 0201.* **[S3]**


> 💡 **QUÉ SIGNIFICA PARA EL PROYECTO Un micropozo individual parece pequeño, pero miles pueden ser relevantes. El piloto debe tener límite por punto y también presupuesto hídrico agregado, medición y reglas de suspensión.**


# 2. Profundidad y geología

Los estudios oficiales describen materiales aluviales y fluviales en los corredores de Tijuana y Alamar. En esos ambientes se han documentado niveles someros compatibles con el resultado del prototipo. Esto no aplica automáticamente a mesas, laderas o roca consolidada.** [S4]**


# 3. Calidad regional: no existe una “mayoría verde” demostrada

La clasificación verde/amarilla/roja/negra es una herramienta interna, no una categoría oficial. Los estudios disponibles no permiten calcular qué porcentaje de toda Tijuana sería “verde”. En el conjunto histórico más adversarial del Alamar, los seis pozos revisados excedieron el límite de sólidos disueltos vigente, pero esos puntos estaban en un corredor urbano vulnerable y no representan toda la ciudad. **[S11/S12]**

*Ejemplo histórico del Alamar: todos los puntos mostrados superaron 1,000 mg/L de sólidos disueltos. **No son **muestras** del prototipo.*** [S12/S5]**

*Distancia de máximos históricos del Alamar frente a límites actuales; escenario adversarial, no promedio municipal.* **[S11/S5]**


# 4. Riesgos a investigar

E. coli, enterococos y otros indicadores de infiltración sanitaria.

Nitratos, nitritos y amonio.

Sólidos disueltos, cloruros, sulfatos, sodio y dureza.

Hierro, manganeso, aluminio, arsénico, plomo y cromo.

Hidrocarburos, BTEX, solventes clorados y contaminantes industriales según el historial del predio.

PFAS o plaguicidas únicamente cuando exista una fuente plausible.


> 💡 **PRINCIPIO SANITARIO Encontrar agua no equivale a encontrar agua potable. El proyecto debe ser capaz de descartar un sitio aunque perforarlo haya sido barato.**


> 💡 **V. ARQUITECTURA 80/20 Y POTABILIZACIÓN**


# 1. La nueva hipótesis económica

La propuesta consiste en limitar la producción total a 350–600 L/día y tratar hasta calidad potable sólo 20%: entre 70 y 120 L/día. El 80% restante —280–480 L/día— se clasifica para usos diferenciados y recibe las barreras que corresponda. Esta separación reduce el tamaño de membranas, consumo de energía, recambios y volumen de rechazo.


# 2. Usos posibles y límites


| Corriente | Usos posibles | Condición indispensable |
| --- | --- | --- |
| Potable 20% | Beber, cocinar, preparación de alimentos. | Cumplimiento verificado de NOM-127 y almacenamiento sanitario. |
| No potable de bajo riesgo | Sanitarios, limpieza exterior o riego compatible. | Análisis que descarte riesgos para personas, plantas, materiales y drenaje. |
| Agua restringida | Uso técnico específico y controlado. | Protocolo escrito, señalización y separación física. |
| Agua descartada | Ninguno. | Cierre cuando el riesgo o costo sea desproporcionado. |



# 3. Clasificación de tratabilidad


| Clase | Perfil | Tratamiento probable | Lectura económica |
| --- | --- | --- | --- |
| VERDE | Química dentro de límites relevantes; turbiedad o microbiología controlables. | Sedimentos, carbón si procede, desinfección y tanque protegido. | Bajo a moderado. |
| AMARILLA | Salinidad o metales moderados; sin contaminante industrial severo. | Pretratamiento específico y quizá membrana pequeña. | Moderado. |
| ROJA | Varias excedencias, amonio, hierro o salinidad alta. | Tren multietapa; considerar potabilizar sólo 10–20 L/día. | Alto; puede superar la perforación. |
| NEGRA | Solventes, hidrocarburos, PFAS o riesgo industrial significativo. | Cerrar o remediar profesionalmente; no usar como solución doméstica. | No frugal / no viable. |



# 4. La ósmosis inversa y su rechazo

La ósmosis inversa no destruye las sales: produce agua con menor concentración y una corriente de rechazo más concentrada. Tratar sólo 70–120 L/día reduce el rechazo frente a tratar 350–600 L/día, pero sigue siendo necesario definir dónde y cómo disponerlo.

*Balance teórico: a mayor volumen potable, mayor rechazo que debe gestionarse.*

*Rangos preliminares de prefactibilidad; no son cotizaciones y dependen de la muestra real.*


# 5. Análisis mínimo por sitio

Campo: pH, conductividad, TDS, temperatura, turbiedad, oxígeno disuelto, potencial redox, nivel y caudal.

Microbiología: E. coli/coliformes termotolerantes, enterococos y otros según riesgo.

Química básica: dureza, alcalinidad, cloruros, sulfatos, amonio, nitratos, nitritos, sodio, calcio y magnesio.

Metales: hierro, manganeso, aluminio, arsénico, plomo, cromo, cadmio, cobre y níquel.

Riesgo urbano: carbono orgánico, DQO, hidrocarburos, BTEX y solventes según uso histórico del predio.

Muestra confirmatoria y repetición estacional cuando proceda.


> 💡 **VI. MARCO LEGAL EXPLICADO SIN TECNICISMOS**


# 1. Cuatro ideas jurídicas básicas


> 💡 **Derecho humano al agua Obliga al Estado a garantizar acceso suficiente, salubre, aceptable y asequible; no crea automáticamente propiedad ni libertad de extracción. [S7/S8/S17]**


> 💡 **Aguas del subsuelo El artículo 27 y la Ley de Aguas Nacionales permiten regulación, vedas y control federal. [S7/S8/S17]**


> 💡 **Veda y libre alumbramiento Baja California tiene regulación histórica y el libre alumbramiento fue suspendido nacionalmente en 2013 en las porciones aplicables. [S7/S8/S17]**


> 💡 **Disponibilidad negativa Dificulta justificar nuevo volumen permanente; obliga a explorar piloto, investigación, límites y posibles volúmenes públicos existentes. [S7/S8/S17]**


# 2. El nombre no controla el acto

La autoridad puede analizar lo que físicamente se hace. Una perforación intencional hasta agua, con ademe, purga, prueba de bombeo o extracción repetida, puede quedar regulada aunque el equipo la llame “sondeo”. Por eso no debemos construir la estrategia sobre una etiqueta.


> 💡 **LÍNEA ROJA No perforar un nuevo punto y después intentar justificarlo como investigación. Primero se solicita que la autoridad determine la ruta aplicable al experimento mínimo.**


# 3. Semáforo de actividades


| Zona | Actividad | Regla |
| --- | --- | --- |
| VERDE | Revisión documental, REPDA, geofísica no invasiva, localización de servicios, pruebas de equipo en circuito cerrado, muestreo autorizado de pozos existentes. | Avanzar con permisos del predio y protocolos de seguridad. |
| AMARILLO | Sondeo geotécnico cerca del nivel freático, direct-push, piezómetro temporal, muestreo mínimo o purga. | Pedir determinación escrita antes de ejecutar. |
| ROJO | Perforar buscando agua, instalar ademe y bomba, prueba de bombeo, almacenamiento o uso doméstico sin ruta autorizada. | No ejecutar. |



# 4. Tres procedimientos diferentes


| Vía | Para qué sirve | Ventaja | Límite |
| --- | --- | --- | --- |
| Transparencia | Pedir documentos y bases existentes. | La ley prevé respuesta hasta 20 días, con ampliación excepcional de 10. | No autoriza perforar ni obliga a crear una opinión nueva. |
| Derecho de petición / consulta | Solicitar que CONAGUA indique qué instrumento aplica al experimento descrito. | Genera trazabilidad y una respuesta sobre el caso planteado. | No sustituye un permiso o título. |
| Trámite formal | Solicitar concesión, asignación, permiso u otro acto aplicable. | Puede autorizar la actividad cuando se integra y resuelve favorablemente. | Depende de requisitos, disponibilidad y régimen del sitio. |



# 5. Pregunta jurídica exacta


> 💡 **TEXTO BASE PARA CONAGUA ¿Qué autorización, título, permiso, aviso o determinación resulta aplicable para construir un punto temporal de observación hidrogeológica de pequeño diámetro, sin aprovechamiento permanente, destinado exclusivamente a medir nivel, recuperar una muestra, evaluar recuperación y cerrarse inmediatamente?**


# 6. Amparo: ruta de reserva

El amparo se prepara con evidencia, pero no se usa como amenaza en el pitch. Su mejor escenario surgiría después de presentar una propuesta limitada, medible y reversible y recibir una negativa o carga concreta que pueda analizarse por fundamentación y proporcionalidad. No existe garantía de éxito.


> 💡 **VII. HACKS LEGÍTIMOS Y ZONAS GRISES APROVECHABLES**

*La estrategia acumula ventajas legítimas antes de aumentar la exposición legal o financiera.*


| H1 | Cambiar el producto: de “pozo barato” a plataforma de validación |
| --- | --- |
| HACK | Ventaja: Evita que el proyecto dependa de una sola promesa y convierte los riesgos en gates medibles. Fundamento: El prototipo ya existe, pero faltan calidad, repetibilidad, rendimiento y ruta legal. Límite: No debe diluirse la demostración física: el método de perforación sigue siendo parte central. Siguiente acción: Aprobar una frase maestra única y retirar promesas de potabilidad o legalidad automática. |



| H2 | Tratar sólo el 20% hasta calidad potable |
| --- | --- |
| HACK | Ventaja: Reduce módulo, energía, consumibles y rechazo; mantiene 70–120 L/día para beber y cocinar. Fundamento: El volumen objetivo total es 350–600 L/día; la mayor parte de los usos domésticos no exige ingerir el agua. Límite: El 80% también debe analizarse y acondicionarse. No toda agua no potable es segura. Siguiente acción: Construir un prototipo hidráulico de dos tanques o dos líneas, con señalización y válvulas antirretorno. |



| H3 | Usar transparencia para obtener el “censo invisible” |
| --- | --- |
| HACK | Ventaja: Permite conseguir tablas, coordenadas, campañas, criterios y expedientes antes de pagar nuevos estudios. Fundamento: La ley obliga a buscar y entregar información documentada dentro de plazos definidos. Límite: Transparencia no autoriza obras ni obliga a crear una opinión jurídica. Siguiente acción: Presentar solicitudes separadas de piezometría, calidad, censos, criterios de sondeos y datos CESPT. |



| H4 | Muestrear aprovechamientos existentes antes de perforar |
| --- | --- |
| HACK | Ventaja: Produce información de calidad y tratabilidad con menor riesgo jurídico, técnico y financiero. Fundamento: REPDA permite identificar títulos y pozos legales; el muestreo puede acordarse con sus titulares. Límite: Una muestra ajena no sustituye la muestra del sitio final y exige cadena de custodia. Siguiente acción: Localizar 3–5 puntos cercanos a zonas candidatas y gestionar cartas de acceso. |



| H5 | Geofísica y pruebas en circuito cerrado primero |
| --- | --- |
| HACK | Ventaja: Descarta roca, servicios enterrados o equipo inadecuado sin alcanzar agua. Fundamento: SEV/ERT, cartografía y un circuito de agua permiten probar presión, caudal, abrasión y bentonita. Límite: La geofísica es indirecta y no garantiza rendimiento ni calidad. Siguiente acción: Diseñar un ensayo comparativo de la bomba actual, separación de sólidos y alternativa para lodos. |



| H6 | Pedir a CONAGUA que clasifique el punto temporal |
| --- | --- |
| HACK | Ventaja: Traslada la incertidumbre procedimental a una pregunta formal y crea trazabilidad. Fundamento: No se encontró una ruta simplificada claramente publicada para microinvestigación de bajo impacto. Límite: Una consulta favorable no siempre equivale a autorización; debe identificarse el acto final. Siguiente acción: Enviar expediente de 5 páginas con diámetro, volumen mínimo, duración, medición y cierre. |



| H7 | Aliado público y académico |
| --- | --- |
| HACK | Ventaja: Cambia el proyecto de iniciativa aislada a investigación supervisada con utilidad pública. Fundamento: Universidad, CESPT, municipio o CDT pueden aportar sitio, peritaje, datos y gobernanza. Límite: Un convenio no sustituye los permisos federales ni garantiza asignación de volumen. Siguiente acción: Solicitar cartas de interés para estudio, laboratorio y mesa técnica, sin pedir aún 100 pozos. |



| H8 | Presupuesto por gates |
| --- | --- |
| HACK | Ventaja: El premio genera valor aunque la autorización tarde: datos, equipo, laboratorio y expediente quedan listos. Fundamento: Un MVP de validación puede concluir con “no viable” y seguir siendo un resultado exitoso y responsable. Límite: No reservar todo el dinero para obra física que quizá no pueda ejecutarse. Siguiente acción: Aprobar presupuesto con fondo condicionado y regla de reasignación si no existe autorización. |



| H9 | Hipótesis de cero incremento neto |
| --- | --- |
| HACK | Ventaja: Podría facilitar un piloto si se vincula a volumen público existente, sustitución, compensación o extracción temporal mínima. Fundamento: La disponibilidad negativa obliga a investigar alternativas a “nuevo volumen adicional”. Límite: Es sólo una hipótesis jurídica e institucional; no debe prometerse. Siguiente acción: Preguntar expresamente a CONAGUA y CESPT si un piloto puede operar bajo volumen existente o balance compensado. |



| H10 | Amparo después de construir el mejor expediente |
| --- | --- |
| HACK | Ventaja: Una eventual impugnación partiría de buena fe, controles ofrecidos y una decisión administrativa concreta. Fundamento: La evidencia del MVP sirve tanto para autorización como para analizar proporcionalidad de una negativa. Límite: No existe precedente que garantice el derecho a perforar por invocar el derecho humano al agua. Siguiente acción: Preparar teoría del caso en paralelo, pero no ejecutar ni litigar antes del acto reclamable. |



> 💡 **EL HACK MÁS IMPORTANTE No esconder un pozo bajo el nombre de sondeo. Diseñar la ruta experimental más pequeña, temporal, medible y reversible que obligue a responder una pregunta que el sistema actual no resuelve claramente.**


> 💡 **VIII. PLAN ESPECÍFICO PARA VENTURE HACKS**


# 1. Dato de convocatoria y verificación pendiente

El equipo reporta una convocatoria Venture Hacks organizada por actores de desarrollo local, con premio de $100,000 MXN y realización los días 12 y 13 de agosto de 2026. Antes de entregar, debe anexarse la publicación oficial y confirmar elegibilidad, criterios, propiedad intelectual, restricciones de gasto y entregables. **[S18]**


# 2. Qué debe entender el jurado en 30 segundos


> 💡 **PROBLEMA Las familias necesitan resiliencia ante interrupciones y el sistema depende de infraestructura costosa y centralizada.**


> 💡 **EVIDENCIA Ya existe un prototipo que alcanzó agua en un sitio con una técnica frugal y equipo reutilizable.**


> 💡 **INNOVACIÓN La solución no es sólo perforar: integra datos, 80/20, almacenamiento, medición, tratamiento por riesgo y un protocolo regulatorio de bajo impacto.**


> 💡 **USO DEL PREMIO Construir el MVP científico-regulatorio que determine si la solución merece pilotearse y bajo qué condiciones.**


# 3. Presupuesto exacto de $100,000 MXN


| Rubro | Monto | Entregable |
| --- | --- | --- |
| Datos, geofísica y servicios enterrados | $12,000 | Mapa de sitios y descarte de riesgos sin perforar. |
| Muestreo y laboratorio acreditado | $20,000 | Caracterización actual con cadena de custodia. |
| Ensayo de tratabilidad + módulo 80/20 | $18,000 | Curvas de remoción, consumibles, energía y rechazo. |
| Prueba del equipo en circuito cerrado | $12,000 | Caudal, presión, sólidos, consumo y desgaste. |
| Expediente jurídico, transparencia y mesa técnica | $8,000 | Solicitudes, dossier y consulta formal. |
| Renta/compra de instrumentación de campo | $8,000 | Nivel, caudal, conductividad, pH y turbiedad. |
| Fondo condicionado para punto autorizado | $20,000 | Obra temporal, cierre o segunda campaña si no se autoriza. |
| Documentación y contingencia | $2,000 | Video, datos, respaldo y materiales del pitch. |
| REGLA DE REASIGNACIÓN Si no existe autorización para el punto temporal durante la ventana del premio, los $20,000 se reasignan a una segunda campaña de muestreo, tratabilidad, instrumentación o diseño del piloto; no se perfora ilegalmente. |  |  |



# 4. MVP del hackathon

Modelo físico o digital del sistema 80/20 con dos líneas separadas.

Mapa de evidencia: prototipo, profundidades históricas, calidad y déficit 0201.

Tablero de gates y decisiones de cierre.

Protocolo de investigación hidrogeológica de bajo impacto.

Presupuesto y modelo de equipo móvil reutilizable.

Solicitud de transparencia lista para enviar.

Pregunta formal a CONAGUA y lista de aliados.

Pitch con afirmaciones permitidas, condicionadas y prohibidas.


# 5. Plan de trabajo dentro del hackathon


| Bloque | Resultado | Responsable |
| --- | --- | --- |
| Bloque 1 \| Alineación | Definir problema, usuario, hipótesis y frase maestra. | Todos |
| Bloque 2 \| Evidencia | Organizar prototipo, fuentes, cifras y riesgos. | Hidrogeología + legal |
| Bloque 3 \| Producto | Diseñar arquitectura 80/20, almacenamiento, medición y gates. | Técnico + tratamiento |
| Bloque 4 \| Hack regulatorio | Construir semáforo legal, solicitudes y protocolo temporal. | Legal + política pública |
| Bloque 5 \| Modelo económico | Presupuesto, costo por punto, escenarios y uso del premio. | Finanzas |
| Bloque 6 \| Demostración | Preparar maqueta, deck, video y respuestas. | Pitch + diseño |
| Bloque 7 \| Red team | Intentar destruir el proyecto y corregir promesas. | Todo el equipo |



# 6. Pitch de 60 segundos


> 💡 **GUION En Tijuana, una familia puede quedarse sin agua aunque la infraestructura regional exista. Nosotros ya demostramos que una perforación frugal puede alcanzar agua en un sitio de la ciudad. AquaResiliencia convierte ese hallazgo en una plataforma segura: selecciona sitios con datos, extrae lentamente hacia almacenamiento, trata sólo 20% para beber y cocinar, mide cada litro y descarta los puntos que no sean seguros o costeables. El premio de $100,000 no financiará promesas ni perforaciones clandestinas: financiará el MVP científico-regulatorio que dirá dónde funciona, qué tratamiento necesita y cuál es la ruta autorizable para un piloto.**


# 7. Preguntas difíciles del jurado


| Pregunta | Respuesta breve |
| --- | --- |
| ¿No está vedado? | Sí. No proponemos operar sin autorización. El MVP empieza con datos, muestreo legal y una consulta escrita para el experimento mínimo. |
| ¿El agua no está contaminada? | Puede estarlo. Por eso el sistema analiza, clasifica y puede cerrar el sitio. No se presume potabilidad. |
| ¿Por qué 20%? | Porque beber y cocinar requieren una fracción del volumen total. Reducimos la planta sin relajar la seguridad del 80%. |
| ¿Qué pasa si CONAGUA tarda? | El MVP todavía produce datos, tratabilidad, prueba de equipo, expediente y protocolo. No perforamos ilegalmente. |
| ¿Cuál es el negocio? | Potencialmente B2G/B2B2G mediante cuadrillas móviles, estudios y sistemas controlados; primero mediremos productividad y costo real. |
| ¿Qué pasa si el agua es roja o negra? | El sistema lo detecta y el punto se restringe o se cierra. Saber dónde no funciona también es valor público. |



> 💡 **IX. PLAN MAESTRO POR FASES Y GATES**


# 1. Fases


| Fase | Horizonte | Trabajo | Salida |
| --- | --- | --- | --- |
| F0 \| Hackathon | 0–2 días | Alinear narrativa, construir MVP, tablero y presupuesto. | Pitch y paquete de validación. |
| F1 \| Expediente y datos | 0–30 días | Documentar prototipo; transparencia; REPDA; aliados; muestreo existente. | Base de evidencia y sitios candidatos. |
| F2 \| Tratabilidad y preconsulta | 30–90 días | Ensayos de banco, módulo 80/20, prueba de bomba, expediente a CONAGUA. | Costo por litro y ruta escrita. |
| F3 \| Punto temporal | 3–6 meses | Ejecutar sólo con instrumento aplicable, medición y cierre. | Nivel, muestra, recuperación y costo real. |
| F4 \| Piloto limitado | 6–12 meses | 5–10 sitios públicos o controlados, con medidores y stop rules. | Informe comparativo y decisión regulatoria. |
| F5 \| Escala | 12+ meses | Programa público, cooperativo o empresa social regulada. | Cuadrillas y crecimiento por presupuesto hídrico. |



# 2. Gates de decisión


| Gate | Frente | Pregunta | Si no pasa |
| --- | --- | --- | --- |
| G0 | Convocatoria | ¿Bases, equipo y claims están verificados? | Corregir antes del pitch. |
| G1 | Evidencia | ¿Podemos reconstruir el prototipo? | Tratar datos como preliminares. |
| G2 | Legal | ¿Existe ruta escrita para la actividad? | No perforar; seguir con datos y muestreo legal. |
| G3 | Sitio | ¿Es geológicamente plausible y libre de servicios? | Descartar ubicación. |
| G4 | Rendimiento | ¿Sostiene 350–600 L/día o una fracción útil? | Reducir objetivo o cerrar. |
| G5 | Calidad 80% | ¿Existe uso diferenciado seguro y costeable? | Tratar más, restringir o cerrar. |
| G6 | Potable 20% | ¿70–120 L/día cumplen la norma con costo razonable? | Reducir a 10–20 L/día o cerrar. |
| G7 | Economía y ambiente | ¿Compite con alternativas y no crea daño agregado? | Pivotar modelo o abandonar. |



# 3. Entregables de 30 días

Expediente digital del prototipo.

Seis solicitudes de transparencia presentadas.

Listado REPDA de aprovechamientos relevantes.

Tres aliados contactados y al menos una carta de interés.

Panel de laboratorio cotizado.

Diseño del módulo 80/20.

Protocolo de prueba del equipo.

Preconsulta a CONAGUA lista o presentada.

Deck de concurso actualizado con fuentes.


> 💡 **X. ROLES DEL EQUIPO, RIESGOS Y DECISIONES**


# 1. Roles sugeridos


| Rol | Responsabilidad |
| --- | --- |
| Líder de proyecto | Integra decisiones, calendario, aliados y pitch. |
| Técnico de perforación | Documenta método, equipo, consumos, seguridad y pruebas. |
| Hidrogeología y datos | Mapas, REPDA, piezometría, sitios y protocolo de campo. |
| Calidad y tratamiento | Muestreo, laboratorio, clasificación y módulo 80/20. |
| Legal y política pública | Transparencia, consulta, permisos, convenio y amparo de reserva. |
| Economía y modelo | Presupuesto, escenarios, costo por litro y escalamiento. |
| Diseño y comunicación | Deck, visuales, demo, narrativa y control de claims. |



# 2. Riesgos que pueden matar el proyecto


| Riesgo | Cómo puede matar el proyecto |
| --- | --- |
| Legal | Perforación no autorizada, clausura o sanción antes de construir el expediente. |
| Sanitario | Enfermedad o exposición por declarar segura un agua no caracterizada. |
| Técnico | Confundir caudal instantáneo con rendimiento sostenible. |
| Geológico | Intentar jetting en roca, rellenos peligrosos o sobre infraestructura enterrada. |
| Económico | Que tratamiento y operación superen claramente alternativas como almacenamiento o pipas. |
| Ambiental | Escalar puntos pequeños sin analizar el efecto acumulativo. |
| Credibilidad | Prometer “agua potable”, “$10 mil final” o “permiso por convenio” sin soporte. |



# 3. Afirmaciones permitidas, condicionadas y prohibidas


| Tipo | Afirmación |
| --- | --- |
| PERMITIDA | “Existe un prototipo funcional en un sitio de Tijuana.” |
| PERMITIDA | “La profundidad observada coincide con rangos históricos de ciertas zonas aluviales.” |
| PERMITIDA | “El equipo de perforación es reutilizable y su costo puede distribuirse.” |
| CONDICIONADA | “El sistema puede aportar 350–600 L/día.” Sólo después de prueba de recuperación. |
| CONDICIONADA | “El 80% sirve para sanitarios.” Sólo después de clasificar el agua para ese uso. |
| CONDICIONADA | “El tratamiento cuesta X.” Sólo con muestra, ensayo y cotización. |
| PROHIBIDA | “Por ser sondeo no requiere permiso.” |
| PROHIBIDA | “El agua a 8 m no pertenece al régimen de CONAGUA.” |
| PROHIBIDA | “El derecho humano al agua permite perforar libremente.” |
| PROHIBIDA | “El sistema final cuesta $10,000 y entrega agua potable.” |



# 4. Decisión estratégica final


> 💡 **VERDADERO PROYECTO PARA EL JURADO Una plataforma de resiliencia hídrica doméstica que combina perforación frugal, equipos móviles reutilizables, selección de sitio, extracción lenta, almacenamiento, tratamiento 80/20, medición y un protocolo regulatorio de investigación de bajo impacto.**


> 💡 **MODIFICACIÓN MÍNIMA PARA VOLVERLO DEFENDIBLE Dejar de prometer una red de pozos terminados y presentar un MVP por gates que primero mide, clasifica, trata y obtiene una ruta escrita para el experimento mínimo.**


> 💡 **XI. FUENTES Y ENLACES**

Los códigos [S#] utilizados en el documento remiten a esta lista. Cuando una fuente es interna o un dato del desarrollador, se indica expresamente. Los enlaces deben revisarse antes de la entrega final del hackathon.


| S1 | Documento Maestro de Proyecto, Estrategia Multidisciplinaria y Pitch para Venture Hacks 2026, proporcionado por el equipo. Fuente interna: /mnt/data/Markdown(8).md pegado |
| --- | --- |



| S2 | Datos reportados del prototipo: 8–9 m, ~8 h, ~2 L/min, PVC, circulación hidráulica, bentonita y motobomba reutilizable. Fuente interna del desarrollador; requiere expediente y mediciones de campo. |
| --- | --- |



| S3 | CONAGUA, Disponibilidad por acuífero, Baja California: acuífero Tijuana 0201. https://sigagis.conagua.gob.mx/gas1/sections/Edos/BajaCalifornia/bc.html |
| --- | --- |



| S4 | CONAGUA, Actualización de la disponibilidad media anual de agua en el acuífero Tijuana 0201. https://www.gob.mx/cms/uploads/attachment/file/103399/DR_0201.pdf |
| --- | --- |



| S5 | DOF, NOM-127-SSA1-2021, Agua para uso y consumo humano. Límites permisibles de la calidad del agua. https://www.dof.gob.mx/nota_detalle_popup.php?codigo=5650705 |
| --- | --- |



| S6 | DOF, NOM-003-CONAGUA-1996 y NOM-004-CONAGUA-1996: construcción, mantenimiento y cierre de pozos. https://www.dof.gob.mx/normasOficiales/3800/semarnat/semarnat.htm |
| --- | --- |



| S7 | Cámara de Diputados, Ley de Aguas Nacionales, texto vigente. https://www.diputados.gob.mx/LeyesBiblio/pdf/LAN.pdf |
| --- | --- |



| S8 | CONAGUA, Suspensión provisional del libre alumbramiento y preguntas frecuentes. https://sigagis.conagua.gob.mx/Gas1/sections/LibreAlumbramiento.html |
| --- | --- |



| S9 | DOF, Ley General de Transparencia y Acceso a la Información Pública, artículo 134. https://www.dof.gob.mx/nota_detalle.php?codigo=5752569&fecha=20/03/2025 |
| --- | --- |



| S10 | CONAGUA, Consulta al Registro Público de Derechos de Agua; corte 30 de junio de 2026. https://app.conagua.gob.mx/consultarepda.aspx |
| --- | --- |



| S11 | Wakida et al. (2005), Impact of a polluted stream on its adjacent aquifer: the case of the Alamar zone, Tijuana, Mexico. https://www.researchgate.net/publication/236210561_Impact_of_a_polluted_stream_on_its_adjacent_aquifer_The_case_of_the_Alamar_zone_Tijuana_Mexico |
| --- | --- |



| S12 | Renovato Tirado et al. (2015), Evaluación del impacto de la canalización del arroyo Alamar en la calidad de agua de su acuífero subyacente. https://www.revista.ingenieria.uady.mx/ojs/index.php/ingenieria/article/view/11 |
| --- | --- |



| S13 | González Estévez y Sánchez Munguía (2013), Riesgo de contaminación del acuífero arroyo Alamar. https://www.scielo.org.mx/scielo.php?pid=S1870-39252013000100004&script=sci_arttext |
| --- | --- |



| S14 | CONAGUA, Portal de Sistemas de Información del Agua. https://app.conagua.gob.mx/sistemasdeagua/ |
| --- | --- |



| S15 | CONAGUA, Biblioteca Virtual de Aguas Subterráneas. https://sigagis.conagua.gob.mx/t_estudios24/ |
| --- | --- |



| S16 | CONAGUA, Consulta a mediciones piezométricas. https://sigagis.conagua.gob.mx/rp20/ |
| --- | --- |



| S17 | DOF, Ley General de Aguas y reformas a la Ley de Aguas Nacionales, 11 de diciembre de 2025. https://www.dof.gob.mx/nota_detalle.php?codigo=5775799&fecha=11/12/2025 |
| --- | --- |



| S18 | Datos de convocatoria aportados por el equipo: Venture Hacks, premio de $100,000 MXN y realización 12–13 de agosto de 2026. Pendiente anexar PDF, publicación o captura oficial de la convocatoria antes de la entrega. |
| --- | --- |



> 💡 **CIERRE El proyecto es suficientemente prometedor para continuar y suficientemente riesgoso para avanzar únicamente con gates. Su fortaleza en el hackathon no está en fingir que no existen barreras: está en convertir esas barreras en un producto, un protocolo y una ventaja de ejecución.**
