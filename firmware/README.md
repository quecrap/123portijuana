# 🔌 GUÍA DE FIRMWARE & DIAGRAMA DE CONEXIONES IoT (ESP32)
## Nodo Centinela AquaResiliencia v1.0

Este directorio contiene el firmware embebido en C++ para el microcontrolador **ESP32 DevKit v1 (30 pines)**, encargado de la telemetría hídrica en tiempo real y el corte preventivo de extracción para evitar la sobreexplotación del acuífero.

---

### 📌 Tabla de Asignación de Pines (Pinout)

| Componente / Sensor | Modelo / Tipo | Pin ESP32 | Modo / Función |
| :--- | :--- | :--- | :--- |
| **Sensor de Conductividad TDS** | Analógico Gravity DFRobot / CQRobot | `GPIO 34` | ADC1_CH6 (Entrada Analógica 0–3.3V) |
| **Caudalímetro de Turbina** | YF-S201 (Efecto Hall, 1/2") | `GPIO 18` | Entrada Digital con Interrupción (`RISING`) |
| **Sensor de Nivel Freático (Trig)** | JSN-SR04T Impermeable | `GPIO 5` | Salida Digital (Pulso 10µs) |
| **Sensor de Nivel Freático (Echo)** | JSN-SR04T Impermeable | `GPIO 19` | Entrada Digital (Retorno de Eco) |
| **Relevador de Corte de Bomba** | Relé 1 Canal 5V/12V Optoacoplado | `GPIO 23` | Salida Digital (Activo en `LOW`) |
| **LED Indicador de Estado** | LED Onboard / Externo Verde-Rojo | `GPIO 2` | Salida Digital (Estado normal / alarma) |
| **Alimentación del Sistema** | Regulador Step-Down 12V a 5V 2A | `VIN / GND` | Alimentación de placa y sensores |

---

### ⚡ Diagrama Esquemático de Conexión

```
              ┌────────────────────────────────────────────────────────┐
              │                   ESP32 DEVKIT V1                      │
              │                                                        │
(TDS Signal) ──► GPIO 34 (ADC1)                              3V3 / 5V ──► (Alimentación Sensores)
(Pulse Hall) ──► GPIO 18 (Interrupt)                              GND ──► (Tierra Común)
(Trig Echo)  ──► GPIO 5  / GPIO 19 ◄── JSN-SR04T                      │
(Relay Ctrl) ──► GPIO 23 ────────────► Relevador 5V ──────────────────► [Corte Bomba 12V/120V]
(Status LED) ──► GPIO 2                                                │
              └────────────────────────────────────────────────────────┘
```

---

### 🛡️ Reglas de Seguridad Implementadas en Firmware:

1. **Límite de Volumen Diario (500 L/día):**
   - El caudalímetro acumula el volumen extraído. Al alcanzar los 500 litros, el relé apaga la bomba y cambia el estado a `CUTOFF_DAILY_LIMIT_REACHED`.
2. **Protección contra Abatimiento de Pozo (< 2.0 m):**
   - Si el nivel dinámico del pozo desciende por debajo de 2 metros sobre la pichancha de succión, el sistema corta la bomba para prevenir cavitación y sobreexplotación local (`CUTOFF_AQUIFER_DRAWDOWN`).
3. **Alarma por Intrusión Salina / Contaminación (> 1,800 ppm):**
   - Si la conductividad TDS excede el umbral seguro, se interrumpe la extracción de emergencia (`CUTOFF_HIGH_TDS_CONTAMINATION`).
4. **Punto de Acceso WiFi y API JSON Abierta:**
   - Genera una red local WiFi `AquaResiliencia_AP` con IP `192.168.4.1` para diagnóstico directo desde cualquier celular o exportación de datos a la plataforma web.

---

### 🛠️ Instrucciones de Compilación (Arduino IDE / PlatformIO):
1. Instalar el paquete de placas **ESP32 by Espressif Systems** en el Gestor de Tarjetas.
2. Instalar la librería **ArduinoJson** (versión 6.x o superior) desde el Gestor de Librerías.
3. Seleccionar la tarjeta **"DOIT ESP32 DEVKIT V1"**.
4. Subir el archivo `firmware_centinela.ino` a 115200 baudios.
