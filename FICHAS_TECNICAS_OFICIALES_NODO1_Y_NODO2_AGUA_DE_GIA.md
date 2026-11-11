# ESPECIFICACIONES TÉCNICAS OFICIALES — PLANTA AGUA DE GIA (NODO 2)

**Documento Operativo Oficial (Google Doc):** [Especificaciones Técnicas Oficiales - Planta Agua de Gia (Nodo 2)](https://docs.google.com/document/d/1fT9YFl_CDJ0bKJujPWT88AnswQVD-DQduo_QPiJt1V8/edit)  
**Autor / Dirección Técnica:** Pablo Campos Moreno  
**Proyecto:** AquaResiliencia Tijuana · Planta Purificadora «Agua de Gía» (Nodo 2)  
**Microcontrolador Oficial:** NodeMCU ESP-32S (38 Pines)  
**Firmware Oficial:** `firmware/main/main.ino` (`hal_electric.h/.cpp`, `washing_cycle_fsm.h/.cpp`, `reactor_ctrl.h/.cpp`, `config.h`)

> [!IMPORTANT]
> A partir del 7 de noviembre de 2026, este documento y su homólogo en Google Docs constituyen el **único punto de partida operativo válido** para el Nodo 2. Toda la documentación de iteraciones anteriores (esquemas con ESP8266, relés de 5A directos a la fuente ATX o configuraciones previas a las etapas MOSFET Bixolon) ha sido archivada en `_Archivo_Historico_Agua_de_Gia_2026`.

---

## 1. MAPA DE PINES OFICIAL ESP-32S (NODO 2)

| GPIO | Tipo / Canal ADC | Etapa de Potencia / Instrumento | Función Confirmada en Inventario Oficial |
| :--- | :--- | :--- | :--- |
| **`GPIO 33`** | Salida Digital | **SSR Fotek #1** | Fuente ATX 15A — Electrofloculación |
| **`GPIO 27`** | Salida Digital | **SSR Fotek #2** | Solenoide de Bronce 3/4" 120V AC — Llenado |
| **`GPIO 26`** | Salida Digital | **Relé Lógico Canal #1** | Bomba Pecera 120V AC — Whirlpool / Lavado |
| **`GPIO 15`** | Salida Digital | **Relé Lógico Canal #2** | Solenoide de Bronce 3/4" 120V AC — Casa |
| **`GPIO 23`** | Salida Digital | **Relé Lógico Canal #3** | Bomba de Diafragma #2 Manifold 12V DC |
| **`GPIO 25`** | Salida Digital | **Bixolon #1 — MOSFET 1** | Solenoide 1/2" Pre-RO 12V DC |
| **`GPIO 13`** | Salida Digital | **Bixolon #1 — MOSFET 2** | Solenoide 1/2" Bypass RO 12V DC |
| **`GPIO 14`** | Salida Digital | **Bixolon #2 — MOSFET 2** | Bomba Evans 24V DC / Timer ASL-52 |
| **`GPIO 5`** | Salida Digital | **Bixolon #2 — MOSFET 1** | Relé Automotriz 30A / Bomba Diafragma #1 Trasvase |
| **`GPIO 16`** | Salida PWM (50 Hz) | **Servomotor MG996R** | Válvula de aguja de rechazo en Ósmosis Inversa |
| **`GPIO 17`** | Salida Digital | **Buzzer / Alarma** | Alarma acústica de panel |
| **`GPIO 18`** | Entrada Interrupción | **Caudalímetro Hall** | Medición de caudal y volumen acumulado (`FALLING`) |
| **`GPIO 19`** | Entrada Digital (`PULLUP`) | **Sensor Nivel Tótem 1** | Nivel alto en Tótem 1 (`LSH-01`) |
| **`GPIO 21`** | Entrada Digital (`PULLUP`) | **Sensor Rebose Tapa** | Paro de seguridad / Rebose de tapa del cono (`LSHH-02`) |
| **`GPIO 22`** | Entrada Digital (`PULLUP`) | **Sensor Barril RO 20L** | Nivel / Presostato de demanda en Barril RO 20L (`PSL-03`) |
| **`GPIO 32`** | Entrada Analógica (`ADC1_CH4`) | **Transductor de Presión** | Señal 0.5V – 4.5V (Presión 0–100 PSI y nivel hidrostático) |
| **`GPIO 34`** | Entrada Analógica (`ADC1_CH6`) | **Sensor de Corriente #1** | Mide corriente AC en la entrada 120V de la fuente ATX |
| **`GPIO 35`** | Entrada Analógica (`ADC1_CH7`) | **Sensor de Corriente #2** | Mide corriente DC de las bombas de diafragma |
| **`GPIO 36 (VP)`** | Entrada Analógica (`ADC1_CH0`) | **Ánodo Pasivo** | Varilla de fierro con divisor de tensión $2 \times 10\text{ k}\Omega$ |

---

## 2. LÓGICA DE CONTROL Y ELECTROFLOCULACIÓN POR CULOMBIOS

### 2.1 Integrador de Faraday (Meta Exacta: 7.60 g de Hierro para 117.3 L)
* En lugar de depender de un temporizador fijo, el ESP-32S integra en cada ciclo no bloqueante la carga eléctrica real acumulada:
  $$Q(t) = \int I_{\text{DC}}(t)\,dt \quad [\text{Culombios}]$$
* Donde $I_{\text{DC}}$ se estima en tiempo real a partir de la corriente AC medida por el Sensor de Corriente #1 (`GPIO 34`) en el primario de 120V AC de la fuente ATX.
* Aplicando la constante electroquímica de Faraday para el hierro ($\text{Fe} \to \text{Fe}^{2+} + 2e^-$, $k_{\text{Fe}} = 0.0002894\text{ g/(A}\cdot\text{s)}$):
  $$m_{\text{Fe}} = Q(t) \times 0.0002894\text{ g/C}$$
* La reacción se detiene de forma automática al alcanzar **exactamente 7.60 gramos de Hierro disuelto** ($\approx 26,262\text{ C}$, equivalente a ~29–35 minutos a 15A DC para los 117.3 Litros).

### 2.2 Protección de la Bomba de Pecera (Whirlpool — `GPIO 26`)
* La bomba de pecera (`GPIO 26`, Relé Lógico Canal #1) **permanece estrictamente apagada** al iniciar el llenado del tanque para evitar trabajar en seco.
* Se enciende automáticamente **únicamente cuando el nivel de agua alcanza el último tercio / nivel superior del tanque** ($\ge 66.7\%$), garantizando el cebado por gravedad desde la espiga de 1/8".

### 2.3 Muestreo del Ánodo Pasivo en `GPIO 36 (VP)` y Registro en LittleFS
* Cada **60 segundos** de electrólisis activa, la máquina de estados ejecuta una **pausa de seguridad de 1.5 segundos** apagando el SSR Fotek #1 (`GPIO 33 = OFF`).
* Durante la ventana de calma sin potencial de la fuente ATX, el ADC1 lee el potencial galvánico en `GPIO 36 (VP)` ($\sim 0.8\text{ V}$ base a través del divisor $2 \times 10\text{ k}\Omega$).
* Cada muestra se almacena en el sistema de archivos Flash **LittleFS** en formato CSV (`/anodo_pasivo.csv`, descargable desde `/api/anodo_pasivo.csv`) para trazar la curva de degradación/mineralización del lote, y al cumplirse los 1.5 segundos se reanuda el SSR Fotek #1 (`GPIO 33 = ON`).
