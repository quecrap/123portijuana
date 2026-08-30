# Guía de Medición Manual — Pozo 01

Solo con perforación + bomba manual (sin nodo IoT todavía) ya se puede levantar una
bitácora real. Registrar en `data/pozo01_bitacora.csv`, una fila por visita.

## Qué medir y con qué (de más barato/urgente a más caro)

| Parámetro | Instrumento mínimo | Costo aprox. | Cómo |
| :--- | :--- | :--- | :--- |
| **Nivel estático** (m) | Cinta métrica + cordel con peso, o silbato de nivel ("well sounder") | $0–$400 MXN | Antes de bombear. Bajar el cordel hasta oír/sentir el agua, medir longitud mojada. |
| **Nivel dinámico** (m) | Mismo método, mientras bombea | $0 | Repetir la medición a los 5, 15 y 30 min de bombeo continuo. |
| **Caudal** (L/min) | Cubeta de volumen conocido (10-20 L) + cronómetro | $0 (ya lo tienen) | Tiempo en llenar la cubeta con la bomba a ritmo normal. |
| **TDS / Conductividad** | Pluma TDS digital — **ya la tienes** ✅ | $0 | Sumergir en muestra recién sacada, esperar que estabilice. |
| **pH** | Medidor de pH — **ya lo tienes** ✅ | $0 | Calibrar con solución buffer si el medidor lo requiere (revisa el manual); sumergir en la misma muestra. |
| **Temperatura** | La misma pluma TDS suele incluirla, o termómetro de cocina | $0–$100 MXN | Muestra recién sacada, no expuesta al sol. |
| **Color / turbidez** | Ojo — comparar contra un vaso con agua de garrafón | $0 | Anotar: clara / turbia / con sedimento / con color. |
| **Olor** | Nariz | $0 | Anotar: sin olor / cloro / azufre (huevo podrido) / tierra. |

**Lo único que de verdad conviene comprar ya:** la pluma TDS (~$200 MXN) — es el dato que
más pesa para las postulaciones (justifica todo el tren 80/20) y no lo pueden estimar a ojo.

## Frecuencia recomendada
- Primeras 2 semanas: 1 medición cada 2-3 días, para ver si el nivel estático se mueve.
- Después: 1 vez por semana es suficiente para detectar tendencia de abatimiento.

## Por qué importa para las postulaciones
Todo lo que el proyecto ha mostrado hasta ahora (AquaEngine, matriz CONAGUA) es
**estimación por modelo**, no medición de campo. En cuanto tengan 3-4 semanas de bitácora
real del Pozo 01, eso se vuelve el dato más creíble de todo el repo — y es exactamente
lo que Imagine H2O / NADB / eAwards piden como evidencia de "pilot data" o "prueba de
concepto en campo".

## Siguiente paso natural
Cuando tengan el Nodo Centinela armado, estos mismos campos (nivel, TDS, caudal) los
mide solo — la bitácora manual y la telemetría del ESP32 deben coincidir en rango para
poder confiar en el sensor una vez instalado.
