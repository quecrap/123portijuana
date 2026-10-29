# FICHAS TÉCNICAS OFICIALES — PROYECTO "AGUA DE GIA" (AQUARESILIENCIA)

**Autor / Dirección Técnica:** Pablo Campos Moreno  
**Proyecto:** AquaResiliencia · Pozo 001 "Agua de GIA" (Tijuana, Baja California)  
**Arquitectura:** Sistema Dual de Telemetría Subterránea Resiliente (Nodo 1) y Planta de Tratamiento Automatizada por Lotes con Interfaz de Voz (Nodo 2)

---

## 1. FICHA TÉCNICA OFICIAL: NODO 1 — Telemetría Subterránea del Pozo ("Modo Cápsula del Tiempo")

### 1.1 Propósito e Instalación Física
* **Propósito:** Monitoreo autónomo y oculto del nivel del pozo y calidad del agua (TDS) a prueba de apagones prolongados y sin depender de la red eléctrica de superficie.
* **Instalación:** Ubicado dentro de un registro de jardín enterrado a ras de suelo (cubierto con plástico y grava para permitir radiofrecuencia), haciendo una salida lateral hacia el tubo del pozo de **2 pulgadas** (donde ya corre un tubo de extracción de **CPVC de 1/2"**).

### 1.2 Componentes de Hardware
* **Cerebro y Comunicaciones:** Placa **LilyGO T-A7670G** (ESP32 con módem **4G LTE** y **Wi-Fi** integrado).
* **Sensor de Nivel de Agua ("Campana de Aire"):** Manguera de poliuretano flexible de **4 mm** bajada junto al tubo de extracción de **CPVC de 1/2"**, conectada arriba en la caja seca a un módulo de presión de aire **HX710B / MPS20N0040D** de **0–40 kPa**.
* **Sensor de Calidad (TDS):** Módulo analógico de TDS **DFRobot Gravity** con electrodos protegidos (apagados mediante un transistor **MOSFET** el **99% del tiempo** para evitar electrólisis y sarro).
* **Alimentación y Energía ("Batería de Tierra"):** Pila galvánica subterránea construida con una barra de **Ánodo de Magnesio** y un tubo de **Cobre** enterrados en la tierra húmeda a un lado del pozo (**cero contaminación al agua**). Esta celda recarga continuamente una **batería de litio reciclada de terminal bancaria** mediante un módulo cosechador de energía (**BQ25504 / regulador solar**).

### 1.3 Lógica de Resiliencia / Cápsula del Tiempo
1. **Persistencia Primaria en Flash:** Guarda cada lectura primero en memoria Flash interna (**LittleFS** en formato **CSV**: `/capsula_tiempo.csv`) por si se requiere extracción local por **USB** o **Wi-Fi Direct**.
2. **Operación Normal:** Transmisión por **Wi-Fi** cada **30–60 minutos**.
3. **Contingencia Celular:** Si no hay Wi-Fi, transmite por red celular **4G LTE 1 vez al día**.
4. **Backoff Exponencial de Supervivencia en Memoria RTC:** Si pierde conectividad celular por completo:
   * **Días 1 a 7:** 1 vez al día
   * **Días 8 a 30:** 1 vez a la semana
   * **Meses 2 a 12:** 1 vez al mes
   * **Más de 1 año sin red:** 1 vez al año
5. **Amortización Financiera:**
   * Costo base estimado: **~$2,150 MXN** (+ **$300 MXN** de consumibles a mitad de vida)
   * Amortizado a **5 años (1,825 días)** = **~$1.34 MXN al día**.

---

## 2. FICHA TÉCNICA OFICIAL: NODO 2 — Planta de Tratamiento, Electrocoagulación y Lógica de Potencia por Lotes

### 2.1 Propósito
Automatización completa del ciclo de purificación de agua por lotes en taller/superficie, controlado mediante una interfaz web local en el celular con soporte bidireccional de comandos por voz (**Web Speech API**).

### 2.2 Hardware Utilizado
* **Microcontrolador:** **NodeMCU (ESP8266)**.
* **Actuadores de Potencia:** Módulo de relés de **4 canales**:
  * **Relé 1:** Válvula solenoide de llenado.
  * **Relé 2:** Fuente DC de electrocoagulación.
  * **Relé 3:** Bomba aireadora.
  * **Relé 4:** Bomba de diafragma presurizadora de transferencia/ósmosis protegida en la succión con **prefiltro de malla de 80 micras** contra limo y mica.
* **Medición de Volumen:** Medidor de flujo por **efecto Hall** en interrupción.
* **Control de Presión:** Transductor de presión de **acero inoxidable 316 de 0–100 PSI** (rosca **1/4" NPT**, alimentación **5V DC**, salida analógica **0.5V–4.5V**).
* **Control de Rechazo (Ósmosis):** Servomotor **MG996R** (**180 grados** con engranajes metálicos) acoplado mecánicamente a la válvula reguladora manual de rechazo.
* **Protección Eléctrica:** Módulo sensor de corriente **ACS712 (30A AC/DC)** para censar los amperios de la fuente de electrocoagulación y cortar el relé en caso de sobrecarga o corto por lodo (`> 12.0 A`).
* **Calidad de Agua en Lote:** Electrodo de **TDS** encendido únicamente durante **pausas de 1.5 segundos** con la fuente de electrocoagulación apagada para evitar ruido eléctrico.

### 2.3 Mapa Oficial de Pines del Nodo 2 (ESP8266 NodeMCU)

| Pin Placa | GPIO | Función Asignada |
| :--- | :--- | :--- |
| **D1** | `GPIO5` | **Relé 1:** Válvula Solenoide de Llenado |
| **D2** | `GPIO4` | **Relé 2:** Fuente DC de Electrocoagulación |
| **D5** | `GPIO14` | **Relé 3:** Bomba Aireadora |
| **D6** | `GPIO12` | **Relé 4:** Bomba Presurizadora RO / Transferencia |
| **D7** | `GPIO13` | **Señal del Medidor de Flujo** (Interrupción) |
| **D8** | `GPIO15` | **Señal PWM del Servomotor MG996R** (Válvula de Rechazo) |
| **D0** | `GPIO16` | **Alimentación VCC del Sensor de TDS** (encendido por pulso) |
| **A0** | `ADC0` | **Entrada Analógica** (Lectura por fase de Corriente ACS712, TDS y Presión 100 PSI) |

---

## 3. Ubicación de los Archivos de Firmware en el Proyecto
* **Nodo 1 (ESP32 LilyGO T-A7670G):** `firmware/Nodo1_Capsula_Tiempo/Nodo1_Capsula_Tiempo.ino`
* **Nodo 2 (ESP8266 NodeMCU):** `firmware/Nodo2_Planta_Agua/Nodo2_Planta_Agua.ino`



---

## 4. BITÁCORA TÉCNICA Y CONTROLADOR ESP32 DE ELECTROFLOCULACIÓN (LOTE 1 BBL / ~117.3 L)

**Fecha de Registro Empírico:** 29 de octubre de 2026  
**Firmware Asociado:** `firmware/Electrofloculacion_1BBL_ESP32/Electrofloculacion_1BBL_ESP32.ino`

### 4.1 Hallazgos Empíricos de Operación (1 BBL a 22.7 °C – 23.0 °C)
* **Migración de Potencia (12V / 15A):**
  * Voltaje real en vacío ($V_{\text{vacío}}$): **11.4 V – 11.6 V**.
  * Inmersión óptima: **50% de la varilla** (+200% de área frente a prueba superficial inicial de 24V/2.5A).
  * Voltaje en carga estacionario ($V_{\text{base}}$): **10.6 V** ($\Delta V = 1.0\text{ V}$), corriente operativa de **~9A a 12A**.
* **Descarte de Medición de Impedancia en Reposo:**
  * Medir Ohms en DC con el electrolito en reposo es inviable por la polarización de electrodos y la película aislante de microburbujas de hidrógeno ($H_2$), que eleva la impedancia aparente a $6.3\text{ k}\Omega$.
* **Barrido Hidráulico de Polarización:**
  * El chorro superficial de recirculación/oxigenación barre la resistencia interfacial; durante el pulso de bomba el voltaje desciende transitoriamente hasta **10.30 V** ($\Delta V = 1.14\text{ V}$) y recupera la línea base al apagar la bomba sin romper la nata blanca superficial.
* **Post-Tratamiento (Biofine):**
  * El ácido silícico coloidal con carga negativa (**Biofine**) se reserva exclusivamente como etapa de pulido final una vez concluido el ciclo eléctrico y asentado el lote.

### 4.2 Mapa de Pines y Divisor de Voltaje (ESP32)

| Pin ESP32 | Identificador | Función en el Sistema 1 BBL |
| :--- | :--- | :--- |
| **`GPIO34` (`ADC1_CH6`)** | `PIN_ADC_VOLTAJE` | Lectura continua ($1\text{ Hz}$) del divisor de voltaje pasivo ($R_1 = 33\text{ k}\Omega$, $R_2 = 10\text{ k}\Omega$, factor $4.30$) conectado a los bornes de salida de la fuente de **12V / 15A** ($0\text{–}12\text{V} \rightarrow 0\text{–}2.79\text{V}$). |
| **`GPIO26`** | `PIN_RELAY_POWER` | **Salida Digital 1 (`RELAY_POWER`):** Conmuta la potencia de 12V hacia los electrodos. |
| **`GPIO27`** | `PIN_RELAY_PUMP` | **Salida Digital 2 (`RELAY_PUMP`):** Activa la bomba de recirculación/oxigenación en pulsos de **30–45 s** cada **5–7 min**. |
| **`GND`** | Tierra Común | Referencia común entre el negativo (`-`) de la fuente de 12V/15A y `GND` de la fuente lógica independiente del ESP32. |
