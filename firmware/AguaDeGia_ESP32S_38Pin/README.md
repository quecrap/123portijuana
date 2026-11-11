# MANUAL DE OPERACIÓN Y COMISIONAMIENTO: AGUA DE GÍA
## NodeMCU ESP32S v1.1 (38 Pines - USB Tipo-C) · Firmware SCADA v4.0

Este documento contiene la guía de instalación física, configuración en el Arduino IDE y protocolo de comisionamiento en campo para la planta potabilizadora **Agua de Gía** (Playas de Tijuana, B.C.).

---

## 1. Mapeo Oficial de Hardware (ESP32S 38-Pin)

### A. Entradas Analógicas (ADC1 - 100% compatibles con WiFi activo)
| Pin Placa | GPIO | Sensor Físico | Rango / Tipo de Señal | Función en el Sistema |
| :--- | :--- | :--- | :--- | :--- |
| **`P34`** | `GPIO 34` | Sensor de Corriente Fuente 40A | 0 - 3.3V (0 - 40A DC) | Integración Faraday segundo a segundo y corte > 38A |
| **`P35`** | `GPIO 35` | Sensor Corriente Bombas DC | 0 - 3.3V (0 - 10A DC) | Detección de atasco / dead-head (> 7.5A) |
| **`P32`** | `GPIO 32` | Presostato / Campana Hidrostática | 0 - 3.3V (0 - 100%) | Nivel continuo de agua en el cono de 1 BBL |

### B. Entradas Digitales (Con Pull-Up Interno)
| Pin Placa | GPIO | Sensor Físico | Lógica de Contacto | Función en el Sistema |
| :--- | :--- | :--- | :--- | :--- |
| **`P4`** | `GPIO 4` | DS18B20 Temperatura Agua | Bus OneWire | Monitoreo térmico de la electrólisis |
| **`P18`** | `GPIO 18`| Caudalímetro Turbina Hall | Interrupción (Pulsos) | Mide caudal instantáneo (L/min) y totaliza litros |
| **`P19`** | `GPIO 19`| Electrodo Nivel Alto Tótem 1 | `INPUT_PULLUP` (GND al mojar) | Detecta llenado de reserva de agua tratada |
| **`P21`** | `GPIO 21`| Paro de Emergencia por Rebose | `INPUT_PULLUP` (2 hilos cobre) | **PARO INMEDIATO TOTAL** si el agua toca la tapa |
| **`P22`** | `GPIO 22`| Presostato Demanda Barril RO 20L | `INPUT_PULLUP` (20-40 PSI) | Arranca y para el tren de Ósmosis Inversa |

### C. Salidas de Potencia y Control (Lógica Active-HIGH: `HIGH` = ON, `LOW` = OFF)
| Pin Placa | GPIO | Actuador / Carga | Etapa de Potencia | Función Hidráulica / Eléctrica |
| :--- | :--- | :--- | :--- | :--- |
| **`P14`** | `GPIO 14` | **Actuador 1:** Bomba Evans Pozo 24V | Bixolon #2 Ch2 | Succión de pozo somero a Tótem (24V DC) |
| **`P27`** | `GPIO 27` | **Actuador 2:** Solenoide Llenado Cono 120VAC | SSR 2 | Llenado del cono de 1 BBL (120V AC) |
| **`P33`** | `GPIO 33` | **Actuador 3:** Fuente ATX Electrofloculación | SSR 1 | Energiza Fuente ATX 12V para Ánodo Fe y Cátodo |
| **`P26`** | `GPIO 26` | **Actuador 4:** Bomba Pecera Whirlpool 120VAC | SSR 3 | Agitación Whirlpool y barrido de microburbujas $H_2$ (120V AC) |
| **`P25`** | `GPIO 25` | **Actuador 5:** Bomba Trasvase 12V vía Relé 30A | Bixolon #2 Ch1 | Trasvase post-racking a filtros de sedimentos (12V DC) |
| **`P23`** | `GPIO 23` | **Actuador 6:** Bomba Manifold + UV | Relé / Etapa Manifold | Presuriza a 50 PSI y esterilización germicida UV |
| **`P15`** | `GPIO 15` | **Actuador 7:** Solenoide Vía A Casa 12V | Relé / Etapa 12V | Llenado de barril presurizado de casa 60L (12V DC) |
| **`P5`**  | `GPIO 5`  | **Actuador 8:** Solenoide Vía B Pre-RO 12V | Bixolon #1 Ch1 | Alimentación previa a membrana de Ósmosis (12V DC) |
| **`P13`** | `GPIO 13` | **Actuador 9:** Solenoide Bypass Lavado RO 12V | Bixolon #1 Ch2 | Retrolavado y purga rápida de membrana RO 1/2" (12V DC) |
| **`P17`** | `GPIO 17` | **Actuador 10:** Buzzer Alarma | Salida Digital (Active-HIGH) | Zumbador de confirmación y emergencias |
| **`P16`** | `GPIO 16` | **Actuador Servo Válvula de Aguja** | PWM (50 Hz) | Regulación de rechazo para sostener 100 PSI en RO |

---

## 2. Instrucciones de Compilación en Arduino IDE

1. Abre el **Arduino IDE** (v1.8.x o v2.x).
2. Asegúrate de tener instalado el paquete de placas **ESP32 by Espressif Systems** en el Gestor de Placas (*Tools -> Board -> Boards Manager*).
3. Selecciona la placa:
   - **Board:** `ESP32 Dev Module`
   - **Upload Speed:** `921600` (o `115200` si hay problemas de cable)
   - **CPU Frequency:** `240MHz (WiFi/BT)`
   - **Flash Frequency:** `80MHz`
   - **Flash Mode:** `QIO`
   - **Partition Scheme:** `Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)`
   - **Core Debug Level:** `None`
4. Abre el archivo:
   `c:\Users\PEPITO\Downloads\123portijuana\firmware\AguaDeGia_ESP32S_38Pin\AguaDeGia_ESP32S_38Pin.ino`
5. Conecta la placa NodeMCU ESP32S mediante el cable **USB Tipo-C** y selecciona el puerto COM correspondiente.
6. Pulsa **Subir (Upload)**.

---

## 3. Conexión desde el Teléfono Celular

1. Al energizar el ESP32, emitirá un breve *beep* de arranque (150 ms).
2. En tu teléfono celular o tablet, ve a **Ajustes de Wi-Fi** y busca la red:
   - **SSID:** `AguaDeGia_Config`
   - **Contraseña:** `agua_resiliente`
3. Una vez conectado al WiFi del ESP32, abre tu navegador web móvil (Chrome, Safari, Firefox) y entra a:
   ```
   http://192.168.4.1
   ```
4. Verás la interfaz móvil en **Modo Oscuro (Dark Mode)** con las dos pestañas principales.

---

## 4. Protocolo de Pruebas en Campo (Pestaña "Test Bench")

Antes de poner a operar la máquina en modo automático, ejecuta este procedimiento de verificación física parado frente al equipo:

### Paso 1: Verificación de Entradas Digitales (Cables y Sensores)
1. Toca o sumerge en un vaso de agua el electrodo del Tótem 1 (`GPIO 19`).
   - **Resultado en pantalla:** La tarjeta de `Nivel Alto Tótem 1` debe cambiar instantáneamente de `⚪ SECO / ABIERTO` a un **`🟢 ¡AGUA DETECTADA!` verde neón resplandeciente**.
2. Junta los 2 hilos de cobre de la tapa del cono (`GPIO 21`).
   - **Resultado en pantalla:** La pantalla desplegará la alerta roja destellante `🚨 ¡REBOSE ACTIVADO! CORTE GENERAL` y el buzzer sonará. Pulsa *"REARMAR"* en la pantalla para restablecer.
3. Sopla levemente la turbina del Caudalímetro Hall (`GPIO 18`).
   - **Resultado en pantalla:** Verás cómo el tacómetro sube a `X.X L/min` y los Hz aumentan en tiempo real.

### Paso 2: Conmutación de Actuadores con "Pulso de 3 Segundos"
1. En la lista de actuadores, localiza el **Actuador 3: Fuente ATX Electrofloculación - SSR 1 (GPIO 33 / P33)**.
2. Pulsa el botón **`⚡ PULSO 3s`**.
   - El botón mostrará una cuenta regresiva: `⏳ 3s... ⏳ 2s... ⏳ 1s...`
   - Verás activarse el LED del SSR 1 (Active-HIGH) durante 3 segundos y apagarse solo sin riesgo de calentar el ánodo en seco.
3. Repite con cada actuador (Actuador 1 en P14, Actuador 2 en P27, Actuador 4 en P26, Actuador 5 en P25, Actuador 6 en P23, Actuador 7 en P15, Actuador 8 en P5, Actuador 9 en P13 y Actuador 10 en P17) para verificar que cada SSR, MOSFET Bixolon y solenoide conmute correctamente.

### Paso 3: Calibración del Actuador Servo de la Válvula de Aguja
1. En la sección **Calibrador Servo Válvula Aguja (GPIO 16)**:
   - Pulsa `0° (Cerrada)`: Comprueba que el brazo mecánico cierre totalmente el orificio de rechazo.
   - Pulsa `90° (1/2 Flujo)`: Comprueba la posición media de trabajo para 100 PSI.
   - Pulsa `180° (Abierta)`: Comprueba la apertura total para despresurización.

---

## 5. Operación Automática (Pestaña "Dashboard Maestro")

Una vez comisionado el hardware en el Test Bench:
1. Cambia a la pestaña **`📊 Dashboard Maestro`**.
2. Pulsa **`▶ INICIAR LOTE`**.
3. El ESP32 ejecutará de forma autónoma la secuencia completa:
   - **Etapa 1:** Llenado con pulsos de solenoide vigilando el amperaje.
   - **Etapa 2:** Medición inicial de conductancia y cálculo estequiométrico de masa de Fe sacrificial.
   - **Etapa 3:** Odómetro de Culombios segundo a segundo por Ley de Faraday con barrido de $H_2$ por la bombita de pecera cada 5 minutos.
   - **Etapa 4:** Bloqueo de reposo forzado (45 minutos) para la decantación de flóculos en el cono.
   - **Etapa 5:** Succión por encima del lodo con bomba de 12V a través del tren de filtración.
   - **Etapa 6 y 7:** Presurización de Manifold a 50 PSI y producción de agua ultrapura por Ósmosis Inversa a 100 PSI.

