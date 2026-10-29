# Bitácora Técnica y Especificación de Diseño: Sistema Casi Autónomo de Electrofloculación y Purificación (1 BBL)

**Fecha:** 29 de octubre de 2026  
**Volumen del Lote:** 1 BBL (~117.3 Litros)  
**Propósito del Documento:** Consolidar el registro operativo empírico, el algoritmo completo desde el pozo hasta el agua para beber/regar y la arquitectura de control en ESP32 (100% física, sin floculantes químicos externos).

---

## 1. Bitácora Cronológica de Operación

* **Condiciones Iniciales:** 
  * Agua a 22.7 °C – 23.0 °C.
  * Fuente de poder inicial: 24V / 2.5A.
* **Prueba 1 (24V / 2.5A):**
  * Electrodo sumergido superficialmente. Voltaje en carga: **23.7 V**. Caída de tensión ($\Delta V$): 0.3 V.
  * Inicio de formación de nata blanca ligera en periferias al minuto 20. Descenso térmico favorable (disipación de apenas +0.5 °C en el electrodo).
* **Migración de Fuente (11:00 AM):**
  * Cambio a fuente de **12V / 15A**. Voltaje en vacío real: **11.4 V – 11.6 V**.
  * Aumento del área sumergida al **50% de la varilla** (aproximadamente +200% de área respecto al inicio).
  * Voltaje en carga base: **10.6 V** ($\Delta V = 1.0\text{ V}$). Corriente estimada de operación: ~9A a 12A.
* **Pruebas de Dinámica y Agitación:**
  * Se identificó un fenómeno de polarización y capacitancia de doble capa en reposo (la impedancia aparente sube a $6.3\text{ k}\Omega$ y decae lentamente por desgasificación de microburbujas de hidrógeno y difusión iónica).
  * Se implementó un ciclo de recirculación/oxigenación por chorro a presión superficial para romper la capa de polarización.
  * Al activar la bomba, el voltaje de carga desciende transitoriamente hasta **10.30 V** ($\Delta V = 1.14\text{ V}$) debido al barrido mecánico de la resistencia interfacial, recuperando el nivel estacionario tras apagar la bomba.

---

## 2. Decisiones Clave de Diseño y Arquitectura (Cero Químicos / Casi Autónomo)

1. **Descarte de Medición de Impedancia en Reposo:**
   * Medir Ohms con corriente directa (DC) en el electrolito es inviable debido al efecto de polarización de electrodos y la capa aislante de gas hidrógeno ($H_2$).
2. **Arquitectura de Monitoreo (Voltaje Dinámico Continuo):**
   * Se usan **fuentes separadas**: una fuente de alta potencia (12V / 15A) para los electrodos y una fuente lógica independiente para el ESP32.
   * La lectura del proceso se realiza mediante un **divisor de voltaje pasivo** conectado a los bornes de salida de la fuente de potencia hacia el pin `ADC` del ESP32, monitoreando la caída de tensión ($\Delta V$) en tiempo real **sin interrumpir la floculación**.
3. **Control de Agitación Adaptativo:**
   * Relé para controlar la bomba de recirculación/oxigenación (`RELAY_PUMP`).
   * **Patrón operativo:** Pulsos de **30 a 45 segundos cada 5 a 7 minutos**, evitando destruir la nata superficial pero barriendo la pasivación de las placas.
4. **Reposo Forzado por Algoritmo (Sin Biofine ni Químicos):**
   * No se utiliza Biofine ni aditivos químicos. Para garantizar la precipitación y compactación física del flóculo, el algoritmo impone un **bloqueo de reposo forzado (`REPOSO_FORZADO`)** al terminar la fase eléctrica, manteniendo cerradas las válvulas solenoides de trasvase y prohibiendo cualquier agitación hasta cumplir el tiempo de sedimentación.
5. **Automatización Hidráulica Posterior (Solenoides + Válvula de Aguja Motorizada):**
   * **Válvulas Solenoides:** Controlan el llenado del reactor de 1 BBL y el trasvase del agua clarificada hacia el tanque de distribución / filtración una vez concluido el reposo forzado.
   * **Actuador sobre Válvula de Aguja (Rechazo RO):** Un servomotor/actuador acoplado a la válvula de aguja ajusta dinámicamente la restricción de rechazo de la membrana de Ósmosis Inversa para sostener siempre la presión óptima de trabajo (**85–90 PSI**), enviando el 20% a agua potable y el 80% a riego/servicios con cero desperdicio.

---

## 3. Algoritmo Maestro en 6 Etapas (Del Pozo al Vaso y al Riego)

| Etapa | Nombre | Sensores que Miden | Actuadores en Acción | Criterio de Avance |
| :--- | :--- | :--- | :--- | :--- |
| **01** | **Extracción Somera y Cápsula del Tiempo** | Campana de aire (`0–40 kPa`), Batería de Tierra (`Mg/Cu`), Caudalímetro Hall | Bomba de pozo somero (24V bajo caudal anti-azolve) | Nivel seguro en pozo + solicitud de lote |
| **02** | **Llenado de Lote (1 BBL / 117.3 L)** | Caudalímetro Hall (`D7` / interrupción) + Lectura de $V_{\text{vacío}}$ (`11.4–11.6V`) | **Válvula Solenoide de Llenado (`SV-01`)** abierta | Volumen = `117.3 L` (o nivel de lote lleno) |
| **03** | **Electrofloculación Adaptativa (12V/15A)** | Divisor de voltaje $\Delta V$ en `ADC1` (`1 Hz`), vigilancia de umbral `10.30V` | **Relé Potencia 12V (`RELAY_POWER`)** + **Bomba Barrido $H_2$ (`RELAY_PUMP`)** 30–45s c/5–7min | Tiempo meta (`60–90 min`) o punto de inflexión ($\Delta V \rightarrow V_{\text{vacío}}$) |
| **04** | **Reposo Forzado y Separación Física** | Temporizador de bloqueo estricto + Muestreo de **TDS** sin ruido eléctrico | **Todos los relés de flujo bloqueados (OFF)** para no romper el flóculo | Cumplimiento del tiempo obligatorio de sedimentación |
| **05** | **Trasvase Clarificado y Bifurcación 80%** | Sensor de flujo + Prefiltro de malla `80 micras` (retiene limo/mica) | **Válvula Solenoide de Trasvase (`SV-02`)** hacia línea de riego/servicios (80%) | Tanque clarificado disponible |
| **06** | **Ultrapurificación RO 20% con Válvula de Aguja** | Transductor `0–100 PSI` (Inox 316) + Sensor `TDS` en línea de producto | **Bomba Booster RO** + **Actuador en Válvula de Aguja** (autorregula a `85–90 PSI`) | Agua lista para beber (`<150 ppm TDS`) y rechazo reintegrado a riego |
