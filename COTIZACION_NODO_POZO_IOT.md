# Cotización de referencia — Nodo Pozo Somero + Kit de Telemetría IoT
**Proyecto:** AquaResiliencia Tijuana — Red Centinela de Justicia Hídrica
**Fecha:** 3 de septiembre de 2026
**Alcance:** un (1) nodo — perforación experimental + kit de telemetría IoT ya construido con proveedores reales.
**Advertencia de origen:** esta cotización combina dos fuentes de distinta madurez. El kit IoT viene de una orden de compra con SKUs y proveedores verificables (`ORDEN_DE_COMPRA_IOT_ESP32.md`). El costo de perforación viene del propio `PLAN_MAESTRO.md`, que lo etiqueta explícitamente como **"costo experimental, no sistema terminado"** y prohíbe presentarlo como costo final de un sistema que entrega agua potable. Esta cotización respeta esa distinción — no la borra.

---

## 1. Resumen — costo por nodo (con mano de obra real, separada por partida)

**Mano de obra decidida por el equipo (3 sept. 2026):** cuadrilla de 3 personas — Pablo (líder) a $3,000/día + 2 ayudantes capaces (no cualquier albañil; personal con la habilidad específica para este trabajo) a $2,000/día c/u = **$7,000 MXN por día**. Es una tarifa **por día, no por hora**, y **se paga hasta terminar el trabajo** — es decir, si el nodo se completa en 1 día (como el caso del prototipo, ~8 horas), el costo de mano de obra es $7,000; si toma 2 días, son $14,000; y así sucesivamente. Todos los cálculos de esta sección usan el escenario de 1 día como caso base y documentado, pero debe tratarse como el piso, no como un tope garantizado.

| Partida | Costo (MXN) | Madurez del dato |
|---|---:|---|
| **A. Materiales de perforación** (PVC + bentonita, ver sección 5.2-5.3) | $2,300 – $3,320 | Cotización real por insumo (2-3 proveedores) |
| **B. Mano de obra** (cuadrilla completa, por día trabajado hasta terminar; 1 día = caso base) | $7,000.00 por día | **Real — tarifa decidida por el equipo**, no estimado externo; escala si el trabajo toma más de 1 día |
| **C. Materiales de telemetría IoT** (ESP32 + sensores + gabinete, ver sección 2) | $1,123.20 | Cotización real — SKUs y proveedores verificados |
| **Total por nodo (A + B + C)** | **$10,423.20 – $11,443.20 MXN (~$579 – $636 USD)** | Mixta — materiales reales + mano de obra real + ver limitaciones en sección 4 |
| *Referencia: dato histórico único del prototipo* | *$10,000.00 (perforación sola, sin desglosar mano de obra)* | *Experimental — un solo caso, ver sección 5.4* |

**No incluido arriba (aparte, según sección 5.1):** la motobomba (~$2,449–$2,700 MXN) es un activo reutilizable de la cuadrilla, no un gasto por nodo — se compra una sola vez y se distribuye entre todos los pozos que se hagan con ella.

**Lectura honesta:** al separar la mano de obra como partida real ($7,000 por día, pagado hasta terminar) en vez de dejarla implícita, el total por nodo en el escenario de 1 día ($10,423–$11,443 MXN) queda muy cerca del dato histórico de $10,000 MXN — lo cual sugiere que ese número original probablemente ya incluía, de manera informal y no desglosada, un costo de trabajo similar. Si el trabajo real toma 2 días, el total sube a **$17,423–$18,443 MXN** (se suman otros $7,000 de mano de obra). La diferencia real de esta cotización es que ahora cada partida (materiales de pozo, mano de obra, materiales de telemetría) se puede facturar, ajustar o negociar por separado, y el riesgo de que el trabajo tome más de un día queda visible en vez de escondido. Preséntese siempre como estimado de referencia por nodo piloto, no como precio de un sistema terminado y certificado que entrega agua potable.

---

## 2. Desglose del kit de telemetría IoT (fuente: `ORDEN_DE_COMPRA_IOT_ESP32.md`)

| Ítem | Componente | Precio (MXN) |
|---|---|---:|
| 01 | Microcontrolador ESP32 DevKit v1 | $145.00 |
| 02 | Sensor de Conductividad/TDS | $190.00 |
| 03 | Sensor de Nivel Ultrasónico IP67 | $175.00 |
| 04 | Caudalímetro de Turbina | $110.00 |
| 05 | Módulo Relevador Optoacoplado | $35.00 |
| 06 | Fuente de Alimentación 12V 2A | $120.00 |
| 07 | Reductor de Voltaje Step-Down | $40.00 |
| 08 | Gabinete Estanco IP65 | $140.00 |
| 09 | Kit de Conectores y Cableado | $85.00 |
| | **Subtotal componentes** | **$1,040.00** |
| | IVA estimado (8% fronterizo BC) | $83.20 |
| | **Total kit IoT** | **$1,123.20** |

Proveedores ya identificados con entrega local (Steren, AG Electrónica, Home Depot Tijuana) o en 24-48h (Amazon MX, Mercado Libre). Este número sí puede llamarse "cotización cercana a oficial": tiene SKU, proveedor y precio unitario trazable.

---

## 3. Lo que esta cotización SÍ cubre y lo que NO cubre

**Sí cubre:**
- Materiales de perforación experimental de un nodo (basado en el único caso documentado del prototipo).
- Hardware completo de un nodo de telemetría IoT (sensores de nivel, conductividad, caudal; control remoto del relevador).

**NO cubre (y no debe presentarse como incluido):**
- Tratamiento de agua para uso potable — el proyecto trata solo un porcentaje del agua para consumo, y ese costo requiere muestra, ensayo de tratabilidad y cotización propia antes de poder afirmarse (regla explícita del `PLAN_MAESTRO.md`, sección de afirmaciones condicionadas).
- Mano de obra formal, permisos, trámites ante CONAGUA, ni el expediente técnico/legal.
- Programación/firmware del ESP32, calibración de sensores, ni conectividad de datos (Wi-Fi/SIM) recurrente.
- Mantenimiento posterior a la instalación.
- Variabilidad de sitio: el costo de perforación puede cambiar según profundidad real, tipo de suelo y si se encuentra roca o relleno.

---

## 4. Qué falta para que esto sea una cotización verdaderamente oficial — actualizado con investigación de mercado (3 sept. 2026)

1. **Marca y modelo exactos de la motobomba** — **parcialmente resuelto** (ver sección 5.1): ya identificamos 2-3 modelos reales disponibles en México con precio de tienda. Falta decidir cuál comprar y confirmar disponibilidad en Tijuana.
2. **Cotización formal de al menos 2 proveedores de PVC y bentonita** — **resuelto** (ver secciones 5.2 y 5.3): ya hay precios reales de 2-3 fuentes distintas para ambos insumos.
3. **Costo de mano de obra** — **resuelto** (ver sección 5.4): el equipo fijó su propia tarifa real — $3,000/día para el líder y $2,000/día para cada uno de 2 ayudantes ($7,000 MXN/día de cuadrilla, cubriendo perforación y telemetría en la misma jornada). Ya no es un estimado externo.
4. **Precio por volumen del kit IoT (10, 20, 50 unidades)** — **investigado, sin resolver del todo** (ver sección 5.5): los proveedores mexicanos tratan la compra al mayoreo como una cotización que hay que pedir directamente (WhatsApp/mensaje al vendedor), no como una tabla de precios pública. Sigue siendo una acción pendiente: contactar a 2-3 vendedores de Mercado Libre/tiendas de electrónica para pedir precio por esas cantidades.

---

## 5. Precios reales de mercado (investigación 3 sept. 2026)

### 5.1 Motobomba a gasolina (~6-6.5 HP) — activo reutilizable, no gasto por pozo

| Modelo | Especificaciones | Precio | Fuente | Confianza |
|---|---|---:|---|---|
| Oakland MG-2055 | 6.5 HP, 500 L/min, 2" | $2,449 (oferta; lista $4,899) | Home Depot México | Alta |
| Pretul 3"x3" autocebante | 6.5 HP, 900 L/min, autocebante | $2,700 | fixferreterias.com | Media |
| Bonhoeffer BON-P-WP2.0-196 | 6.5 HP, 600 L/min, 2" | $5,480 | jardepot.com | Media |

**Nota clave:** el propio `PLAN_MAESTRO.md` ya reconoce esto como una afirmación permitida: *"El equipo de perforación es reutilizable y su costo puede distribuirse."* Es decir, la motobomba (~$2,449–$2,700 MXN) es una inversión única que se reparte entre todos los pozos que se perforen con ella, no un costo que se repite en cada nodo. Por eso no se suma directamente al costo "por nodo" en la sección 1 — debe presupuestarse aparte, una sola vez, como equipo de la cuadrilla.

### 5.2 Tubería PVC cédula 40 (ademe del pozo)

Para ~8-9 m de profundidad se necesitan aproximadamente 2 tramos de 6 m (con acoples), en diámetro de 4" (100 mm):

| Proveedor | Precio por tramo de 6m | Por metro | Confianza |
|---|---:|---:|---|
| Casa Myers | $1,020.14 | ~$170/m | Alta |
| Bricomark (Cresco/Futura) | $1,401.28 | ~$233.5/m | Alta |
| DepotMX (oferta) | $1,865.00 | ~$311/m | Media (precio de oferta, puede no sostenerse) |

Estimado para 2 tramos: **entre $2,040 y $3,730 MXN**, dependiendo del proveedor.

### 5.3 Bentonita sódica (sellado del pozo)

| Proveedor | Presentación | Precio | Por kg | Confianza |
|---|---|---:|---:|---|
| Alcione.mx (marca Volclay/Perfobent) | Saco 25 kg | $258.70 (oferta; lista $344.02) | ~$10.35/kg | Alta |
| Fosforita de México | Saco 50 kg | $519.00 (sin existencias actualmente) | ~$10.38/kg | Media |

Estimado para 1-2 sacos de 25 kg (uso típico de sellado en un pozo somero): **entre $260 y $520 MXN**.

### 5.4 Mano de obra — tarifa real decidida por el equipo (actualizado 3 sept. 2026)

**Ya no es un estimado externo: el equipo fijó su propia tarifa.**

| Rol | Personas | Pago por día (no por hora) | Subtotal por día |
|---|---:|---:|---:|
| Líder (Pablo) | 1 | $3,000.00 | $3,000.00 |
| Ayudante capaz (perfil específico, no mano de obra genérica) | 2 | $2,000.00 | $4,000.00 |
| **Total cuadrilla, por día trabajado** | **3** | | **$7,000.00 MXN/día** |

**Se paga hasta terminar el trabajo** — el número de días a pagar depende de cuánto tarde realmente el nodo, no es un monto fijo. El caso documentado del prototipo tomó ~8 horas (1 jornada), y ese es el escenario base usado en la sección 1; si un sitio en particular toma más tiempo (suelo más difícil, imprevistos), el costo de mano de obra sube $7,000 por cada día adicional.

**Nota importante sobre el perfil de los ayudantes:** no se trata de contratar a cualquier albañil de la calle — son ayudantes con la capacidad específica para este trabajo (manejo de la motobomba, tubería PVC, bentonita, y ahora también la instalación de la telemetría). Por eso las siguientes comparaciones de mercado deben leerse solo como piso de referencia, no como una tarifa equivalente:

- Salario mínimo 2026, Zona Libre de la Frontera Norte (incluye Tijuana): $440.87 MXN/día — fuente: La Jornada. La tarifa de $2,000/día es **más de 4 veces** ese piso legal.
- Sueldo de albañil genérico en Tijuana (Indeed, 8 reportes salariales): promedio $17,706 MXN/mes → aproximadamente $590-680 MXN/día (cálculo nuestro). Este dato es de mano de obra de construcción general, **no de un perfil especializado en perforación/telemetría** como el que se necesita aquí — se incluye solo como referencia de piso de mercado, no como comparación directa.
- **Lectura:** pagar por encima del promedio de mercado general es consistente con requerir un perfil más capaz que un albañil genérico, y es una buena señal de trato justo hacia la cuadrilla, útil para narrativas de impacto social ante financiadores.
- Referencia de mercado para pozos completos (NO comparable directamente): Cronoshare.com.mx reporta pozos excavados de ~10 m entre $13,000 y $60,000 MXN — corresponde a perforación mecanizada formal, no al método manual/frugal de AquaResiliencia. Sirve solo como techo de referencia.

### 5.5 Precio por volumen del kit IoT (ESP32)

- Una tienda mexicana (tiendadeelectronica.mx) vende el ESP32 DevKit V1 a $200 MXN por unidad y ofrece explícitamente "precio especial a venta mayoreo, solicita tu compra" — es decir, **existe precio de mayoreo, pero solo bajo solicitud directa, no publicado**.
- Mercado Libre y Amazon México tienen múltiples listados de ESP32, pero sus páginas de producto bloquean la lectura automatizada, así que no se pudo confirmar una tabla de descuento por cantidad.
- **Conclusión honesta:** no existe un precio de mayoreo público verificable en este momento. La acción pendiente es contactar directamente a 2-3 vendedores (Mercado Libre, AG Electrónica, tiendadeelectronica.mx) y pedir cotización formal para 10, 20 y 50 unidades antes de escalar el despliegue.

---

*Documento construido a partir de dos fuentes internas del proyecto (`ORDEN_DE_COMPRA_IOT_ESP32.md` y `PLAN_MAESTRO.md`), sin inventar cifras nuevas. Las limitaciones de la sección 4 son las mismas que el propio equipo ya había identificado como pendientes.*
