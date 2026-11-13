"""
Servidor Puente USB-Serial <-> Web para el Banco de Pruebas ESP32 (Agua de Gía)
Permite abrir la interfaz gráfica completa (Modo Ingeniero / Lavadora / Alquimia)
en http://localhost:8085 y controlar todos los relés y leer todos los sensores
directamente por el cable USB (sin depender del Wi-Fi).
"""
import http.server
import re
import socketserver
import sys
import threading
import time
import urllib.parse
import webbrowser
import serial
import serial.tools.list_ports

PORT_HTTP = 8085
BAUD_RATE = 115200

# Cargar el HTML directamente desde live_ui.html
LIVE_UI_PATH = r"C:\Users\PEPITO\Downloads\123portijuana\live_ui.html"
UI_H_PATH = r"C:\Users\PEPITO\Downloads\123portijuana\firmware\main\retro_washer_ui.h"

if os.path.exists(LIVE_UI_PATH):
    with open(LIVE_UI_PATH, "r", encoding="utf-8") as f:
        HTML_PAGE = f.read()
else:
    with open(UI_H_PATH, "r", encoding="utf-8") as f:
        ui_content = f.read()
    m = re.search(r'R"rawliteral\((.*?)\)rawliteral"', ui_content, re.DOTALL)
    HTML_PAGE = m.group(1) if m else "<h1>Error cargando UI</h1>"

# Iniciar directamente en Modo Ingeniero (Banco de Pruebas de Relés y Cableado)
HTML_PAGE = HTML_PAGE.replace(
    "let initialSkin = 'alquimia';",
    "let initialSkin = 'ingeniero';"
).replace(
    "localStorage.getItem('aguadegia_skin') || 'alquimia'",
    "'ingeniero'"
).replace(
    "Pin P34 (ADC1_CH6)",
    "LoLin A0 · Borne NO Relé 8 (ACS712)"
).replace(
    "Pin P35 (ADC1_CH7)",
    "LoLin A0 · Borne NC Relé 8 (0-100 PSI)"
).replace(
    "Pin P32 (ADC1_CH4)",
    "LoLin A0 · Presión Manifold (PSI)"
).replace(
    "LSH-01 · Pin P19",
    "TORNILLOS TAPA CONO · Pin S3 (GPIO10)"
).replace(
    "LSHH-02 · Pin P21",
    "TOPE 1 BBL (117 L) · Pin S3 (GPIO10)"
).replace(
    "PSL-03 · Pin P22",
    "PRESOSTATO AGUA TOMAR (<20 PSI)"
)

serial_lock = threading.Lock()
ser_conn = None
detected_com = None
last_status_json = '{"ok":false}'


def find_esp32_port():
    ports = list(serial.tools.list_ports.comports())
    for p in ports:
        desc = (p.description or "").upper()
        hwid = (p.hwid or "").upper()
        if any(k in desc or k in hwid for k in ["CP210", "CH340", "CH910", "USB", "UART", "SILICON"]):
            if p.device not in ("COM1", "COM4"):
                return p.device
    for p in ports:
        if p.device not in ("COM1", "COM4"):
            return p.device
    return None


def send_serial_cmd(cmd, expect_prefix="JSON_"):
    global ser_conn, last_status_json
    with serial_lock:
        if ser_conn is None or not ser_conn.is_open:
            return None
        try:
            ser_conn.reset_input_buffer()
            ser_conn.write((cmd + "\n").encode("utf-8"))
            ser_conn.flush()
            deadline = time.time() + 0.8
            while time.time() < deadline:
                if ser_conn.in_waiting:
                    line = ser_conn.readline().decode("utf-8", errors="ignore").strip()
                    if line.startswith("JSON_STATUS:"):
                        last_status_json = line[len("JSON_STATUS:"):]
                        return last_status_json
                    elif line.startswith("JSON_OK:"):
                        return line[len("JSON_OK:"):]
                else:
                    time.sleep(0.01)
        except Exception as e:
            print(f"[WARN] Error serial: {e}")
    return None


class BridgeHandler(http.server.BaseHTTPRequestHandler):
    def log_message(self, format, *args):
        pass

    def do_GET(self):
        parsed = urllib.parse.urlparse(self.path)
        if parsed.path == "/" or parsed.path == "/index.html":
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.end_headers()
            self.wfile.write(HTML_PAGE.encode("utf-8"))
        elif parsed.path == "/api/status":
            res = send_serial_cmd("STATUS", "JSON_STATUS:")
            payload = res if res else last_status_json
            self.send_response(200)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.send_header("Access-Control-Allow-Origin", "*")
            self.end_headers()
            self.wfile.write(payload.encode("utf-8"))
        else:
            self.send_response(404)
            self.end_headers()

    def do_POST(self):
        parsed = urllib.parse.urlparse(self.path)
        qs = urllib.parse.parse_qs(parsed.query)
        if parsed.path == "/api/relay":
            rid = qs.get("id", ["0"])[0]
            act = qs.get("action", ["off"])[0]
            send_serial_cmd(f"RELAY {rid} {act}")
        elif parsed.path == "/api/servo":
            ang = qs.get("angle", ["90"])[0]
            send_serial_cmd(f"SERVO {ang}")
        elif parsed.path == "/api/mode":
            eng = qs.get("engineer", ["1"])[0]
            send_serial_cmd(f"MODE {eng}")
        elif parsed.path == "/api/washer":
            cmd = qs.get("cmd", [""])[0]
            val = qs.get("val", ["0"])[0]
            send_serial_cmd(f"WASHER {cmd} {val}")
        elif parsed.path == "/api/reset_emergency":
            send_serial_cmd("RESET_EMERGENCY")
        elif parsed.path == "/api/reset_flow":
            send_serial_cmd("RESET_FLOW")

        self.send_response(200)
        self.send_header("Content-Type", "application/json")
        self.send_header("Access-Control-Allow-Origin", "*")
        self.end_headers()
        self.wfile.write(b'{"ok":true}')


def main():
    global ser_conn, detected_com
    port = sys.argv[1] if len(sys.argv) > 1 else find_esp32_port()
    if not port:
        print("Esperando conexión USB del ESP32...")
        while not port:
            time.sleep(1)
            port = find_esp32_port()

    detected_com = port
    print(f"[OK] Conectando al ESP32 en {detected_com} @ {BAUD_RATE} baudios...")
    ser_conn = serial.Serial()
    ser_conn.port = detected_com
    ser_conn.baudrate = BAUD_RATE
    ser_conn.timeout = 0.5
    ser_conn.dtr = False
    ser_conn.rts = False
    for attempt in range(10):
        try:
            ser_conn.open()
            break
        except Exception as e:
            if attempt == 9:
                raise
            time.sleep(0.5)
    time.sleep(1.5)

    # Leer mensajes de arranque si los hay
    while ser_conn.in_waiting:
        line = ser_conn.readline().decode("utf-8", errors="ignore").strip()
        if line:
            print("  [ESP32]", line)

    # Poner en Modo Ingeniero por defecto para probar cableado y relés
    send_serial_cmd("MODE 1")
    send_serial_cmd("STATUS")

    url = f"http://localhost:{PORT_HTTP}"
    print(f"\n========================================================")
    print(f"  🛠️ BANCO DE PRUEBAS ESP32 ACTIVO EN: {url}")
    print(f"  Puerto USB: {detected_com} (10 Relés + Sensores en vivo)")
    print(f"========================================================\n")
    webbrowser.open(url)

    socketserver.ThreadingTCPServer.allow_reuse_address = True
    with socketserver.ThreadingTCPServer(("127.0.0.1", PORT_HTTP), BridgeHandler) as httpd:
        httpd.serve_forever()


if __name__ == "__main__":
    main()
