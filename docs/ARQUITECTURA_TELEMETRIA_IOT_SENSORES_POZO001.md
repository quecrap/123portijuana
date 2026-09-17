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

## 4. Arquitectura de Energía Frugal y Gestión Eléctrica

* **Generación Solar:** Arreglo de 2 a 3 paneles solares de 5W en paralelo (10W - 15W total), aprovechando stock existente de bajo costo.
* **Almacenamiento Eléctrico:** Batería de motocicleta sellada de 12V (AGM / Gel de 4Ah - 7Ah), económica, de fácil reposición local y con capacidad suficiente para soportar operación continua día y noche (24/7).
* **Consumo Eficiente:** Microcontrolador ESP32-S3 operando en modo *Deep Sleep* cíclico cada 15 minutos ($< 15\ \mu\text{A}$), activando la alimentación de sensores solo durante 3 segundos por lectura.

---

## 5. Conectividad Comunitaria: La "Mula de Datos Ciudadana" (Crowdsourced QR Uplink)

Frente al alto costo de una estación del USGS (~$15,000 USD con módem satelital y planes de datos dedicados), AquaResiliencia implementa un modelo de **Sincronización Oportunista y Ciencia Ciudadana Participativa**:

```
┌────────────────────────────────────────────────────────────────────────────────────────┐
│               MECANISMO DE SINCRONIZACIÓN CÍVICA "PEAJE DE DATOS"                      │
│                                                                                        │
│  1. REGISTRO LOCAL (24/7):                                                             │
│     El ESP32 mide NAF, TDS y Temp cada 15 min y guarda el histórico en memoria Flash.  │
│                                                                                        │
│  2. ESCANEO DEL QR (En el Pozo):                                                       │
│     Un vecino, estudiante o visitante escanea el Código QR físico pegado en el brocal. │
│                                                                                        │
│  3. CONEXIÓN LOCAL (BLE / SoftAP):                                                     │
│     El teléfono del usuario lee el lote de datos acumulados desde el ESP32.            │
│                                                                                        │
│  4. PEAJE CÍVICO & UPLINK:                                                             │
│     La WebApp solicita al usuario: "Préstale 5 KB de tus datos móviles al pozo".      │
│     El smartphone hace el POST automático a la base de datos abierta en la nube.       │
│                                                                                        │
│  5. RECONOCIMIENTO EN PANTALLA:                                                        │
│     "¡Gracias Centinela! Los datos de este pozo acaban de sincronizarse gracias a ti." │
└────────────────────────────────────────────────────────────────────────────────────────┘
```

### Ventajas del Esquema:
1. **Cero costo operativo mensual:** No se requiere contratar ni pagar planes de datos SIM 4G/LTE fijos por cada pozo.
2. **Resiliencia:** Si hay WiFi público o residencial cercano disponible, el ESP32 se conecta de forma directa; si no, la sincronización ocurre cada vez que un ciudadano o brigada visita el pozo.
3. **Apropiación Social:** El ciudadano se vuelve parte activa de la red de monitoreo hídrico de su propia comunidad.

