# 💧 AquaResiliencia Tijuana — Sistema Automatizado de Gestión Hídrica (ESP8266MOD / ESP32)

Firmware industrial en C++ para microcontroladores **ESP8266 (ESP8266MOD / NodeMCU v2-v3 / WeMos D1 con CH340G)** y **ESP32 DevKit**, diseñado para la automatización hidráulica de pozos someros, protección de bombas contra trabajo en seco, control de presurización de ósmosis inversa, dosificación por gravedad para electrofloculación/sedimentación, servidor web embebido mediante **WebSockets** y **actualización inalámbrica vía Wi-Fi (OTA)**.

---

## 📌 1. Mapeo de Hardware y Conexiones en ESP8266MOD (NodeMCU / CH340G)

En las placas ESP8266MOD / NodeMCU, los pines impresos en la serigrafía (`D0` a `D8` y `A0`) se conectan de la siguiente manera:

### Sensores e Instrumentación ([P-01] a [P-08]):
| ID | Sensor / Instrumento | Pin Serigrafía ESP8266 | GPIO Interno | Tipo de Señal | Función / Lógica |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **[P-01]** | Presencia / Nivel de Agua en Pozo | **`D0`** | `GPIO 16` | Entrada Digital | Detección de agua en succión. Si no hay agua (`¡SECO!`), corta la bomba inmediatamente (protección anti-cavitación). |
| **[P-02]** | Presostato / Transductor Ósmosis | **`A0`** | `ADC0` | Entrada Analógica (0–3.3V) | Monitorea ciclo de membrana en rango 60 – 80 PSI. Arranca a < 60 PSI y corta a >= 80 PSI. |
| **[P-05]** | Nivel Máximo Tótem 2 (Superior) | **`D3`** | `GPIO 0` | Digital (Pull-up 10k en placa) | Flotador magnético de nivel máximo (100%). Corta la bomba de pozo al llenarse. |
| **[P-06]** | Nivel Mínimo Tótem 1 (Inferior) | **`D4`** | `GPIO 2` | Digital (Pull-up 10k en placa) | Umbral de reserva segura >= 20 Litros. Si cae por debajo, inhabilita la ósmosis. |
| **[P-07]** | Presostato Auxiliar Barril | **`D8`** | `GPIO 15` | Digital (Pull-down 10k en placa) | Presostato calibrado a 60 PSI para depósito presurizado. Al alcanzar 60 PSI, corta la ósmosis. |
| **[P-08]** | Caudalímetro de Turbina Hall | **`D7`** | `GPIO 13` | Digital con ISR (`RISING`) | Sensor tipo YF-S201 (7.5 Hz/L/min). Calcula caudal instantáneo (L/min), total acumulado (L) y lote (L). |

### Módulo de 4 Relés (SRD-05VDC Optoacoplado):
| Relé | Canal | Pin Serigrafía ESP8266 | GPIO Interno | Carga Conectada | Polaridad |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Relé 1** | `IN1` | **`D1`** | `GPIO 5` | **Bomba de Pozo** (Carga de potencia AC/DC) | Activo en `LOW` (0V activa relé) |
| **Relé 2** | `IN2` | **`D2`** | `GPIO 4` | **Válvula Solenoide Ósmosis Inversa** | Activo en `LOW` |
| **Relé 3** | `IN3` | **`D5`** | `GPIO 14` | **Válvula Solenoide Gravedad Floculación** | Activo en `LOW` |
| **Relé 4** | `IN4` | **`D6`** | `GPIO 12` | **Canal Auxiliar / Expansión Libre** | Activo en `LOW` |

> **Nota de Seguridad de Hardware:** Los pines `D1`, `D2`, `D5`, `D6` fueron seleccionados por ser los pines 100% seguros de arranque (*boot-safe*) del ESP8266. No parpadean ni alteran el modo de arranque al encender la placa, protegiendo las bombas de activaciones accidentales.

---

## ⚡ 2. Chip USB CH340G y Velocidad a 9600 Baudios

* **Controlador USB:** Tu placa utiliza el chip **CH340G**. Windows ya lo tiene reconocido en el puerto **COM6** (`USB-SERIAL CH340`).
* **Velocidad Serial:** Configurada estrictamente a **9600 baudios** (`Serial.begin(9600)`), tal como lo requiere tu dispositivo.
* Al abrir el Monitor Serie en Arduino IDE, selecciona **9600 baud**.

---

## ⚙️ 3. Reglas de Automatización Implementadas

1. **Bomba de Pozo (Relé 1 / D1):**
   - **Arranque:** Se enciende automáticamente si `[P-01]` (D0) detecta agua Y `[P-05]` (D3) no está lleno.
   - **Corte:** Se apaga de inmediato si `[P-05]` se llena al 100% O si `[P-01]` detecta nivel bajo (**protección activa contra marcha en seco**).

2. **Sistema de Ósmosis Inversa (Relé 2 / D2):**
   - **Arranque:** Se activa si el Tótem 1 `[P-06]` (D4) tiene reserva (>= 20L) Y la presión `[P-02]` (A0) cae por debajo de 60 PSI, siempre que el barril `[P-07]` (D8) no esté lleno (< 60 PSI).
   - **Corte:** Se apaga si la presión `[P-02]` alcanza 80 PSI, o si el barril `[P-07]` llega a 60 PSI, o si el Tótem 1 se queda sin agua (< 20L).

3. **Floculación por Caudal y Gravedad (Relé 3 / D5 y Sensor [P-08] / D7):**
   - El caudalímetro en `D7` monitorea el caudal en L/min y acumula los litros dosificados.
   - Mediante el control deslizante (*slider*) interactivo de la interfaz web, puedes fijar el volumen objetivo (1 a 100 L).
   - Al pulsar *"Iniciar Lote"*, la solenoide de gravedad (D5) se abre y dosifica agua hacia la celda; al alcanzar la meta de litros, se cierra automáticamente.

---

## 🌐 4. Pantalla Web de Control & WebSockets (192.168.4.1)

El ESP8266 genera su propia red Wi-Fi local sin requerir internet ni routers:
* **SSID Wi-Fi:** `AquaResiliencia_AP`
* **Contraseña:** `AquaResiliencia2026`
* **URL del Dashboard:** `http://192.168.4.1`

### Funciones del Panel Web:
* **Telemetría en tiempo real cada 1 segundo por WebSockets:** Valores en vivo de `[P-01]` a `[P-08]` y estado de los 4 relés.
* **Control manual de actuadores:** Conmutadores tipo switch para encender/apagar cada relé y selector de modo `AUTO` / `MANUAL`.
* **Paro de Emergencia (E-STOP):** Botón rojo industrial que apaga de inmediato los 4 relés.
* **Slider dinámico de Floculación:** Envío instantáneo de setpoints de litros por lote con barra de progreso porcentual.
* **Actualización Web OTA:** Puedes subir un nuevo archivo binario `.bin` directamente desde el navegador de tu celular o PC sin cables.

---

## 💻 5. Uso Automatizado con PowerShell

Desde la terminal PowerShell en la carpeta del proyecto:

### A) Compilar el Firmware:
```powershell
cd c:\Users\PEPITO\Downloads\123portijuana\firmware\aquaresiliencia_control
.\compilar.ps1
```
*(Ya compilado exitosamente: genera `build/aquaresiliencia_control.ino.bin` de 411 KB).*

### B) Subir a tu ESP8266 por USB (COM6):
```powershell
.\flashear_usb.ps1
```
*(Detecta automáticamente el chip CH340G en COM6 y flashea la placa).*

### C) Actualizar vía Wi-Fi (Sin Cables):
Una vez que el ESP8266 esté funcionando y conectado:
```powershell
.\actualizar_ota.ps1 -Ip "192.168.4.1"
```

---

## 🖥️ 6. Uso desde la interfaz de Arduino IDE

1. Abre **Arduino IDE**.
2. Ve a **Archivo** $\rightarrow$ **Abrir** y selecciona:  
   `c:\Users\PEPITO\Downloads\123portijuana\firmware\aquaresiliencia_control\aquaresiliencia_control.ino`.
3. Selecciona la placa: **NodeMCU 1.0 (ESP-12E Module)**.
4. Selecciona el puerto: **COM6** (CH340).
5. Abre el **Monitor Serie** y configúralo en **9600 baudios**.
6. Haz clic en **Subir** (flecha).
