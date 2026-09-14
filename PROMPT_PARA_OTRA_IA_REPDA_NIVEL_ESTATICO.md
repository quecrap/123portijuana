Eres un investigador de datos públicos hidrogeológicos. Objetivo: obtener el NIVEL ESTÁTICO y NIVEL DINÁMICO (profundidad del agua subterránea en metros) de pozos específicos registrados en Tijuana, Baja California, México, usando el REPDA (Registro Público de Derechos de Agua) de CONAGUA.

## Contexto ya investigado (para que no repitas trabajo)

1. La API REST pública de CONAGUA (`https://sigagis.conagua.gob.mx/ArcGIS/rest/services/REPDA/MapServer/1/query`) SÍ funciona bien vía HTTP GET/curl y da datos reales (folio, titular, uso, coordenadas, volumen concesionado) de los pozos del municipio de Tijuana — pero su esquema de campos NO incluye nivel estático, nivel dinámico ni profundidad del pozo. Ya se consultó y se confirmó esta limitación.

2. El formulario web de consulta individual por título (`https://app.conagua.gob.mx/consultarepda.aspx`) es una página ASP.NET con postback que, en teoría, permite buscar por número de título específico (sección "Búsqueda básica", campo "Título"). En un intento previo con un navegador automatizado, el formulario mostró errores de JavaScript (`Sys is not defined`, un recurso con 403) que impidieron completar la búsqueda — no se pudo confirmar si el registro individual de un título revela o no el nivel estático. Esto quedó sin resolver.

3. Ya se identificaron los siguientes folios reales de pozos de uso "SERVICIOS" y "PÚBLICO URBANO" en el acuífero Tijuana (clave 0201), ordenados por cercanía a Playas de Tijuana (coordenadas de referencia: 32.531, -117.118), que son los candidatos prioritarios a revisar:

   - `01BCA107779/01EMGR99` — Desarrollo Industrial La Jolla, S.A. de C.V. (3.7 km de Playas)
   - `01BCA100166/01EMDA14` — Unión de Distribuidores de Agua en Tanque C.R.O.M. (5.9 km)
   - `01BCA100721/01EMGR05` — Sindicato de Trabajadores de Transportes Similares y Conexos Morelos de Tijuana (7.0 km)
   - `01BCA100301/01HSOC12` — Comisión Estatal de Servicios Públicos de Tijuana, CESPT (7.9 km — este título ampara MÚLTIPLES aprovechamientos/pozos municipales, vale la pena revisar todos sus anexos)
   - `A1BCA100200/01EMGR94` — Unión de Distribuidores de Agua de Tijuana FROC-CROC (11.6 km)

## Tu tarea

1. Intenta consultar cada uno de estos títulos en `https://app.conagua.gob.mx/consultarepda.aspx` (sección "Títulos y permisos de aguas nacionales", búsqueda básica por campo "Título" — probablemente debas ingresar solo la parte antes del "/", ej. `01BCA100301`, ya que no se confirmó el formato exacto que acepta el campo). Si tienes acceso a un navegador con JavaScript funcional, esto podría resolver el problema técnico que bloqueó el intento anterior.

2. Si el registro individual muestra un enlace a un PDF del título o su anexo técnico, descárgalo y busca dentro del documento: nivel estático, nivel dinámico, profundidad del pozo, y fecha de la prueba de bombeo.

3. Si el formulario sigue sin funcionar, intenta rutas alternativas: (a) buscar si existe un endpoint REST adicional de CONAGUA/SIGAGIS con más capas de datos (adicionales a `REPDA/MapServer/1`) que sí incluyan estos campos técnicos; (b) buscar si el "Banco Nacional de Datos de Aguas Subterráneas" (BNDAS) de CONAGUA tiene una versión pública descargable con estos datos por pozo.

4. Si logras acceder a cualquier dato real, repórtalo con el folio exacto y la fuente/URL. Si no, documenta honestamente qué intentaste y por qué no funcionó — NO inventes cifras de nivel estático ni URLs. Este es un requisito estricto: cualquier cifra reportada debe venir de un documento que hayas leído directamente, nunca de un resumen de buscador sin verificar contra la fuente primaria (esto ya causó al menos 3 alucinaciones detectadas en investigaciones previas sobre este mismo tema).

Reporta en formato JSON: folio, nivel_estatico_m, nivel_dinamico_m, profundidad_pozo_m, fecha, url_fuente, verificado (true/false), y un resumen del esfuerzo si no se encontró nada.
