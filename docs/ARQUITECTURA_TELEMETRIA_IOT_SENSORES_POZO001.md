# Especificación Técnica — Estación de Telemetría IoT (Pozo Piloto 001)
**Proyecto:** AquaResiliencia Tijuana / Colectivo 1, 2, 3 por Tijuana  
**Ubicación de Referencia:** Pozo Piloto 001 (Playas de Tijuana, Sección Los Sauces: `32.5225, -117.1180`)  
**Fecha:** Septiembre de 2026  
**Objetivo:** Especificar el hardware, sensores sumergibles de grado de campo, protocolo de bajo consumo y la integración con la API de datos abiertos del USGS para monitoreo transfronterizo.

---

## 1. Arquitectura de Hardware y Diagrama de Conexión

```
┌────────────────────────────────────────────────────────────────────────────────────────┐
│                                GABINETE SUPERIOR (IP67)                                │
│                                                                                        │
│  [ Panel Solar 25W ] ──> [ Controlador MPPT 12V ] ──> [ Batería LiFePO4 12V 6Ah ]      │
│                                                                  │                     │
│                                                       [ Step-Down 12V -> 3.3V/5V ]     │
│                                                                  │                     │
│                           ┌──────────────────────────────────────▼──────────────────┐  │
│                           │        MICROCONTROLADOR ESP32-S3 (Dual Core)            │  │
│                           │  - Modo Deep-Sleep cíclico (15 min)                     │  │
│                           │  - Transmisión WiFi / Mesh / Opción LoRa SX1262         │  │
│                           │  - Memoria Flash SPI / Almacenamiento local CSV         │  │
│                           └──────────────┬───────────────────────────┬──────────────┘  │
│                                          │                           │                 │
│                               [ Interfaz RS485 Modbus ]       [ GPIOs ADC / 1-Wire ]   │
└──────────────────────────────────────────┼───────────────────────────┼─────────────────┘
                                           │                           │
                   CABLE DE GRADO SUBMARINO CON TUBO CAPILAR BAROMÉTRICO (IP68)
                                           │                           │
┌──────────────────────────────────────────▼───────────────────────────▼─────────────────┐
│                                 DENTRO DEL POZO SOMERO                                 │
│                                                                                        │
│  1. Sensor Hidrostático de Nivel Piezométrico (0-5m / RS485 Modbus RTU / Inox 316L)    │
│  2. Sonda Industrial de Conductividad Eléctrica (CE / TDS) con Excitación AC y ATC     │
│  3. Sonda de pH Industrial con Electrodo de Vidrio Blindado y Referencia en Gel PTFE   │
│  4. Sensor Digital de Temperatura DS18B20 (Acero Inoxidable 1-Wire)                    │
└────────────────────────────────────────────────────────────────────────────────────────┘
```

---

## 2. Especificación de Sensores de Campo

### A) Nivel Freático Piezométrico (Transmisor Hidrostático de Presión)
* **Principio:** Presión hidrostática de columna de agua ($P = \rho \cdot g \cdot h$).
* **Rango:** 0 a 5.0 metros de columna de agua.
* **Cuerpo:** Acero inoxidable 316L con diafragma cerámico/piezorresistivo.
* **Cable:** Poliuretano (PUR) con tubo capilar interno de venteo para compensación automática de presión barométrica en tiempo real.
* **Salida:** RS485 Modbus RTU (evita pérdidas de señal analógica y ruido eléctrico).

### B) Conductividad Eléctrica (CE) y Sólidos Disueltos Totales (TDS)
* **Principio:** Excitación por corriente alterna (AC) de cuatro electrodos o anillo de grafito (para evitar electrólisis y polarización catódica).
* **Rango de Medición:** 0 a 10,000 µS/cm (0 a 5,000 ppm TDS).
* **Compensación de Temperatura:** Sensor NTC/PT1000 integrado con compensación automática a $25^\circ\text{C}$ ($2.0\% / ^\circ\text{C}$).

### C) Potencial de Hidrógeno (pH)
* **Cuerpo:** Vidrio de alta resistencia con camisa de protección de policarbonato/POM.
* **Sistema de Referencia:** Doble unión con electrolito de gel de alta viscosidad y diafragma anular de teflón poroso (resistente a ensuciamiento biológico y sedimentos finos).

### D) Temperatura Subterránea
* **Sensor:** Dallas DS18B20 digital.
* **Precisión:** $\pm 0.5^\circ\text{C}$ de $-10^\circ\text{C}$ a $+85^\circ\text{C}$.
* **Resolución:** Configurable a 12 bits ($0.0625^\circ\text{C}$).

---

## 3. Integración Binacional: Consumo de Datos del USGS

Para contrastar el comportamiento del acuífero costero de Tijuana frente a la cuenca baja transfronteriza, el sistema integrará lecturas de la estación oficial del **Servicio Geológico de los Estados Unidos (USGS)**:

* **Estación USGS:** `11013500` (*Tijuana River near Nestor, CA*).
* **Endpoint API REST Abierta:**
  ```http
  https://waterservices.usgs.gov/nwis/iv/?format=json&sites=11013500&parameterCd=00065,00095,00010
  ```
* **Parámetros descargados:**
  * `00065`: Nivel de agua / Gage height (pies -> metros).
  * `00095`: Conductividad específica a $25^\circ\text{C}$ ($\mu\text{S/cm}$).
  * `00010`: Temperatura del agua ($^\circ\text{C}$).

---

## 4. Estrategia de Gestión de Energía y Datos

1. **Ciclo de Operación (15 minutos):**
   * *Despertar (Wakeup):* El RTC interno del ESP32 despierta el microcontrolador.
   * *Alimentación de sensores:* El relé MOSFET enciende la línea de 12V/RS485 durante 3 segundos para estabilización.
   * *Adquisición:* Lectura de NAF, TDS, pH y Temperatura.
   * *Transmisión / Almacenamiento:* Envío por protocolo HTTP POST/MQTT a la base de datos abierta y respaldo en tarjeta micro-SD local.
   * *Dormir (Deep Sleep):* El microcontrolador entra en modo de reposo profundo ($< 15\ \mu\text{A}$ de consumo).
2. **Autonomía:** Con una batería LiFePO4 de 12V 6Ah y un panel solar de 25W, el nodo tiene **autonomía ilimitada en operación normal** y hasta 25 días continuos de operación en caso de oscuridad total.
