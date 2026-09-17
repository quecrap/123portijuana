# Comparativa de Costos e Ingeniería — Telemetría USGS vs. AquaResiliencia Tijuana
**Proyecto:** AquaResiliencia Tijuana / Colectivo 1, 2, 3 por Tijuana  
**Fecha:** Septiembre de 2026  
**Propósito:** Sustentar con datos auditables la viabilidad económica del nodo IoT Frugal frente a la tecnología tradicional de monitoreo hidrogeológico, y proveer la estructura técnica para el guion de video.

---

## 📊 1. Tabla Comparativa de Costos Reales de Mercado

```
┌──────────────────────────────────────┬────────────────────────────────────────┬────────────────────────────────────────┐
│ Componente                           │ Estación Tradicional (USGS / NOAA)     │ Nodo Centinela (AquaResiliencia)       │
├──────────────────────────────────────┼────────────────────────────────────────┼────────────────────────────────────────┤
│ 1. Datalogger / Cerebro Central      │ **Campbell Scientific CR1000X**        │ **Microcontrolador ESP32-S3**          │
│                                      │ Costo: $3,500 – $5,000 USD             │ Costo: $15 USD                         │
├──────────────────────────────────────┼────────────────────────────────────────┼────────────────────────────────────────┤
│ 2. Sonda de Calidad de Agua          │ **Sonda YSI EXO2 Base**                │ **Sonda Industrial de Grafito (AC)**   │
│                                      │ Costo: $6,500 – $8,450 USD             │ + **pH Gel PTFE**                      │
│                                      │                                        │ Costo: $75 USD                         │
├──────────────────────────────────────┼────────────────────────────────────────┼────────────────────────────────────────┤
│ 3. Sensores Integrados               │ Smart Sensor Wiped Cond ($1,950 USD)   │ Sonda Hidrostática Inox 316L (Capilar) │
│                                      │ pH Smart Sensor ($785 USD)             │ + Temp Digital DS18B20                 │
│                                      │ Turbidez ($2,150 USD)                  │                                        │
│                                      │ Costo sensores: ~$4,885 USD            │ Costo sensores: $59 USD                │
├──────────────────────────────────────┼────────────────────────────────────────┼────────────────────────────────────────┤
│ 4. Sistema Eléctrico / Energía       │ Panel 50W + Batería AGM Grado Militar  │ 2-3 Paneles 5W en paralelo             │
│                                      │ + Regulador Campbell ($1,200 USD)      │ + Batería de moto 12V 5Ah ($35 USD)    │
├──────────────────────────────────────┼────────────────────────────────────────┼────────────────────────────────────────┤
│ 5. Telemetría y Comunicaciones       │ Transmisor Satelital GOES/NOAA         │ **Mula de Datos Ciudadana (QR)**       │
│                                      │ + Módem Celular Sierra ($2,500 USD)    │ + WiFi Oportunista                     │
│                                      │ Gasto mensual SIM: $50 USD/mes         │ **Costo mensual: $0.00 USD**           │
├──────────────────────────────────────┼────────────────────────────────────────┼────────────────────────────────────────┤
│ 6. Gabinete y Montaje                │ Gabinete NEMA 4X Fibra de Vidrio       │ Gabinete Estanco IP67                  │
│                                      │ + Torre meteorológica ($1,100 USD)     │ con pasacables glándula ($12 USD)      │
├──────────────────────────────────────┼────────────────────────────────────────┼────────────────────────────────────────┤
│ 💰 **COSTO TOTAL POR ESTACIÓN**       │ **$19,685 – $23,135 USD**              │ **$196 USD**                           │
│                                      │ *(~$390,000 – $460,000 MXN)*           │ *(~$3,900 MXN)*                        │
└──────────────────────────────────────┴────────────────────────────────────────┴────────────────────────────────────────┘
```

> **Conclusión de Impacto:** Por el costo de **UNA** sola estación tradicional del USGS (~$20,000 USD), AquaResiliencia puede construir e instalar **100 Nodos Centinela Ciudadanos**, cubriendo la totalidad de las 9 delegaciones de Tijuana.

---

## 🔬 2. Interoperabilidad Científica (Compatibilidad con el USGS)

Aunque nuestro hardware cuesta 100 veces menos, los datos son **100% compatibles y homologables** con las bases de datos de la CILA, CONAGUA y el USGS porque exportamos en el estándar internacional de parámetros:

* `parameterCd: 00065`: Nivel Piezométrico de Columna de Agua ($h$ en metros).
* `parameterCd: 00095`: Conductividad Eléctrica Específica compensada a $25^\circ\text{C}$ ($\mu\text{S/cm}$).
* `parameterCd: 00400`: Potencial de Hidrógeno (pH).
* `parameterCd: 00010`: Temperatura del Agua Subterránea ($^\circ\text{C}$).

---

## 🎬 3. Estructura y Guion para el Video (Hechos, no palabras)

* **Gancho (0-15s):** "Al norte de la cerca, una estación para medir el agua cuesta 20 mil dólares. Al sur, en Tijuana, creamos una que hace lo mismo por 190 dólares... y el internet lo pone la gente."
* **El Problema (15-35s):** Mostrar la dependencia del acueducto y la falta de datos sobre el agua somera que corre bajo nuestros pies.
* **La Ingeniería (35-65s):** Mostrar el Pozo 001, los paneles de 5W, la batería de moto y los sensores sumergibles de acero inoxidable.
* **El Peaje Cívico / Mula de Datos (65-90s):** Explicar cómo el ciudadano escanea el QR en el pozo, dona 5 KB de sus datos y se convierte en Centinela del Agua.
* **Cierre (90-110s):** "La resiliencia hídrica no se compra con presupuestos millonarios: se construye con ciencia comunitaria, datos abiertos y pies en el pozo."
