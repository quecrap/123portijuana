import os
import sys
import time
import socket
import threading
import urllib.request
import urllib.error
from http.server import HTTPServer, BaseHTTPRequestHandler
import mimetypes
import json

BASE_DIR = os.path.abspath(r"c:\Users\PEPITO\Downloads\123portijuana")
ESP32_HOST = "192.168.1.102"
ESP32_PORT = 80
ESP32_URL = f"http://{ESP32_HOST}:{ESP32_PORT}"
LOCAL_PORT = 8080

esp32_online = False
last_check_time = 0
last_scan_time = 0

def probe_esp32_ip(ip, timeout=1.0):
    try:
        req = urllib.request.Request(f"http://{ip}:{ESP32_PORT}/api/status", method="GET")
        with urllib.request.urlopen(req, timeout=timeout) as resp:
            if resp.status == 200:
                body = resp.read(1024).decode("utf-8", errors="ignore")
                if "mode" in body or "relays" in body or "wifi" in body or "washer" in body or "pt1" in body:
                    return True
    except Exception:
        pass
    return False

def check_esp32_reachability():
    global esp32_online, last_check_time, last_scan_time, ESP32_HOST, ESP32_URL
    now = time.time()
    if now - last_check_time < 2.0:
        return esp32_online
    last_check_time = now
    if probe_esp32_ip(ESP32_HOST, 0.35):
        esp32_online = True
        return True
    esp32_online = False
    # Auto-descubrir en 192.168.0.x ("la ultima compuerta") o mDNS cada 10s
    if now - last_scan_time >= 10.0:
        last_scan_time = now
        for host in ("AguaDeGia-ESP32.local", "AguaDeGia-Lavadora.local", "192.168.4.1"):
            if probe_esp32_ip(host, 0.3):
                ESP32_HOST = host
                ESP32_URL = f"http://{ESP32_HOST}:{ESP32_PORT}"
                esp32_online = True
                return True
    return esp32_online

SIMULATED_STATE = {
    "workshopMode": True,
    "online": False,
    "message": "ESP32 apagado por seguridad (cableado y soldadura en curso)",
    "washer": {
        "engineerMode": True,
        "running": False,
        "paused": False,
        "dryRun": True,
        "knobPos": 1,
        "knobAngle": 0,
        "stageTitle": "MODO TALLER · CABLEANDO Y SOLDANDO",
        "stageSub": "ESP32 apagado por seguridad. Consulta diagramas y lógica física.",
        "loadSize": 2,
        "targetLiters": 117.3,
        "soapType": 0,
        "r0Ohms": 5.0,
        "estPpm": 2400,
        "targetFe": 4.69,
        "dissolvedFe": 0.0,
        "faradayPct": 0.0,
        "stageElapsedSec": 0,
        "soakRemainingSec": 2700,
        "waitingValve": False,
        "roFlushing": False,
        "roFlushRemSec": 0,
        "zeroFlowSec": 0,
        "emergency": False,
        "emergencyReason": "",
        "bellCounter": 0
    },
    "multimeter": {
        "ct1Raw": 0, "ct1Volts": 0.0, "ct1Amps": 0.0, "ct1Ppm": 0, "ct1FeGps": 0.0,
        "ct2Raw": 0, "ct2Volts": 0.0, "ct2Amps": 0.0, "ct2DeadHead": False,
        "pt1Pin": 32, "pt1Raw": 0, "pt1RawFloat": 0.0, "pt1Mv": 0.0, "pt1Volts": 0.0,
        "pt1Psi": 0.0, "pt1HeightCm": 0.0, "pt1Pct": 0.0, "pt1Liters": 0.0,
        "pt1ZeroMv": 500.00, "pt1MvPerCm": 0.56893, "pt1Connected": False,
        "tt1TempC": 22.4, "ft1Pulses": 0, "ft1Lpm": 0.0, "ft1TotalL": 0.0,
        "lsh01": False, "lshh02": False, "psl03": False, "srv1Angle": 90,
        "electrodeEnabled": False
    },
    "actuators": [
        {"id":0,"pin":33,"code":"SSR1 / K_40A","board":"Potencia 40A (12VDC)","desc":"Relé Potencia 40A -> Celda E1 (10A-40A)","activeLow":True,"state":False,"pulsing":False},
        {"id":1,"pin":14,"code":"K1","board":"Placa 1 · Relé 1","desc":"Motor Bomba Pozo Evans 24V (M1)","activeLow":True,"state":False,"pulsing":False},
        {"id":2,"pin":27,"code":"K2","board":"Placa 1 · Relé 2","desc":"Solenoide Llenado Cono 120VAC (YV1)","activeLow":True,"state":False,"pulsing":False},
        {"id":3,"pin":26,"code":"K3","board":"Placa 1 · Relé 3","desc":"Bomba Pecera Pulmón Whirlpool 120VAC (M2)","activeLow":True,"state":False,"pulsing":False},
        {"id":4,"pin":25,"code":"K4","board":"Placa 1 · Relé 4","desc":"Bomba Trasvase 12V 5A post-racking (M3)","activeLow":True,"state":False,"pulsing":False},
        {"id":5,"pin":23,"code":"K5","board":"Placa 2 · Relé 1","desc":"Bomba Principal Manifold + Balastro UV (M4+UV1)","activeLow":True,"state":False,"pulsing":False},
        {"id":6,"pin":15,"code":"K6","board":"Placa 2 · Relé 2","desc":"Solenoide NC Vía A - Barril Casa 60L (YV2)","activeLow":True,"state":False,"pulsing":False},
        {"id":7,"pin":5,"code":"K7","board":"Placa 2 · Relé 3","desc":"Solenoide NC Vía B - Pre-Membrana RO (YV3)","activeLow":True,"state":False,"pulsing":False},
        {"id":8,"pin":13,"code":"K8","board":"Placa 2 · Relé 4","desc":"Solenoide NC Bypass Lavado RO 1/2 pulg (YV4)","activeLow":True,"state":False,"pulsing":False},
        {"id":9,"pin":17,"code":"BZ1","board":"Alarma / Campana","desc":"Zumbador / Campana de Fin de Ciclo","activeLow":False,"state":False,"pulsing":False}
    ]
}

def get_simulated_status():
    return SIMULATED_STATE

class WorkshopHandler(BaseHTTPRequestHandler):
    def log_message(self, format, *args):
        if self.path in ('/', '/api/status', '/diagramas', '/portal'):
            return
        super().log_message(format, *args)

    def do_HEAD(self):
        self.handle_request(is_head=True)

    def do_GET(self):
        self.handle_request(is_head=False)

    def do_POST(self):
        path = self.path.split('?')[0]

        # Captura de Leads de Pre-Factibilidad (Formulario de Marca)
        if path == '/api/lead':
            try:
                content_len = int(self.headers.get('Content-Length', 0))
                post_body = self.rfile.read(content_len) if content_len > 0 else b"{}"
                data = json.loads(post_body.decode('utf-8'))
                
                leads_file = os.path.join(BASE_DIR, "leads_prefactibilidad.json")
                leads = []
                if os.path.exists(leads_file):
                    try:
                        with open(leads_file, 'r', encoding='utf-8') as f:
                            leads = json.load(f)
                    except Exception:
                        leads = []
                leads.append(data)
                with open(leads_file, 'w', encoding='utf-8') as f:
                    json.dump(leads, f, indent=2, ensure_ascii=False)
                
                resp = json.dumps({"ok": True, "message": "Lead registrado en base local"}).encode('utf-8')
                self.send_response(200)
                self.send_header("Content-Type", "application/json")
                self.send_header("Content-Length", str(len(resp)))
                self.send_header("Access-Control-Allow-Origin", "*")
                self.end_headers()
                self.wfile.write(resp)
                return
            except Exception as e:
                resp = json.dumps({"ok": False, "error": str(e)}).encode('utf-8')
                self.send_response(500)
                self.send_header("Content-Type", "application/json")
                self.send_header("Content-Length", str(len(resp)))
                self.end_headers()
                self.wfile.write(resp)
                return

        # Proxy al ESP32 si está online
        is_online = check_esp32_reachability()
        if is_online:
            try:
                content_len = int(self.headers.get('Content-Length', 0))
                post_body = self.rfile.read(content_len) if content_len > 0 else b""
                req = urllib.request.Request(
                    f"{ESP32_URL}{self.path}",
                    data=post_body if content_len > 0 else None,
                    headers={k: v for k, v in self.headers.items() if k.lower() != 'host'},
                    method='POST'
                )
                with urllib.request.urlopen(req, timeout=1.5) as resp:
                    data = resp.read()
                    self.send_response(resp.status)
                    for k, v in resp.headers.items():
                        if k.lower() not in ('transfer-encoding', 'connection'):
                            self.send_header(k, v)
                    self.end_headers()
                    self.wfile.write(data)
                    return
            except Exception:
                pass
        
        # Fallback offline interactivo para pruebas de actuadores y relés
        query = urllib.parse.urlparse(self.path).query
        params = urllib.parse.parse_qs(query)

        if path == '/api/relay':
            act_id = int(params.get('id', ['-1'])[0])
            action = params.get('action', [''])[0]
            if act_id == 99 and action == 'off_all':
                for act in SIMULATED_STATE['actuators']:
                    act['state'] = False
                    act['pulsing'] = False
            elif 0 <= act_id < len(SIMULATED_STATE['actuators']):
                act = SIMULATED_STATE['actuators'][act_id]
                if action == 'toggle':
                    act['state'] = not act['state']
                    act['pulsing'] = False
                elif action == 'on':
                    act['state'] = True
                    act['pulsing'] = False
                elif action == 'off':
                    act['state'] = False
                    act['pulsing'] = False
                elif action == 'invert':
                    act['activeLow'] = not act.get('activeLow', True)
                elif action == 'pulse':
                    act['state'] = True
                    act['pulsing'] = True
                    def _reset_pulse(target_act):
                        time.sleep(3.0)
                        target_act['state'] = False
                        target_act['pulsing'] = False
                    threading.Thread(target=_reset_pulse, args=(act,), daemon=True).start()

        elif path == '/api/servo':
            angle = int(params.get('angle', ['90'])[0])
            SIMULATED_STATE['multimeter']['srv1Angle'] = angle

        elif path == '/api/electrode':
            if 'enable' in params:
                SIMULATED_STATE['multimeter']['electrodeEnabled'] = (params['enable'][0] == '1')

        elif path == '/api/pt1/cal':
            cmd = params.get('cmd', [''])[0]
            m = SIMULATED_STATE['multimeter']
            if cmd == 'tare':
                m['pt1ZeroMv'] = m['pt1Mv'] if m['pt1Mv'] >= 150.0 else 500.00
                m['pt1HeightCm'] = 0.0
                m['pt1Pct'] = 0.0
                m['pt1Liters'] = 0.0
            elif cmd == 'height':
                cm = float(params.get('cm', ['0'])[0])
                m['pt1HeightCm'] = cm
                m['pt1Pct'] = min(100.0, max(0.0, (cm / 110.0) * 100.0))
                if cm <= 35.0:
                    m['pt1Liters'] = 15.7904 * ((cm / 35.0) ** 3) if cm > 0 else 0.0
                else:
                    m['pt1Liters'] = 15.7904 + min(75.0, cm - 35.0) * 1.35346
            elif cmd == 'reset':
                m['pt1ZeroMv'] = 500.00
                m['pt1MvPerCm'] = 0.56893
            resp_cal = json.dumps({
                "ok": True,
                "zero_mv": m['pt1ZeroMv'],
                "mv_per_cm": m['pt1MvPerCm'],
                "mv": m['pt1Mv'],
                "height_cm": m['pt1HeightCm'],
                "liters": m['pt1Liters']
            }).encode('utf-8')
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", str(len(resp_cal)))
            self.send_header("Access-Control-Allow-Origin", "*")
            self.end_headers()
            self.wfile.write(resp_cal)
            return

        elif path == '/api/mode':
            is_eng = params.get('engineer', ['0'])[0] == '1'
            SIMULATED_STATE['washer']['engineerMode'] = is_eng

        elif path == '/api/washer':
            cmd = params.get('cmd', [''])[0]
            val = params.get('val', ['0'])[0]
            if cmd == 'pos':
                SIMULATED_STATE['washer']['knobPos'] = int(val)
            elif cmd == 'load':
                SIMULATED_STATE['washer']['loadSize'] = int(val)
            elif cmd == 'soap':
                SIMULATED_STATE['washer']['soapType'] = int(val)
            elif cmd == 'start':
                SIMULATED_STATE['washer']['running'] = True
                SIMULATED_STATE['washer']['paused'] = False
            elif cmd == 'pause':
                SIMULATED_STATE['washer']['paused'] = True

        elif path == '/api/reset_emergency':
            SIMULATED_STATE['washer']['emergency'] = False
            SIMULATED_STATE['washer']['emergencyReason'] = ""

        resp_json = json.dumps({"ok": True, "offline": True, "message": "Comando aplicado en banco de pruebas"}).encode('utf-8')
        self.send_response(200)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(resp_json)))
        self.send_header("Access-Control-Allow-Origin", "*")
        self.end_headers()
        self.wfile.write(resp_json)

    def handle_request(self, is_head=False):
        path = self.path.split('?')[0]
        host = self.headers.get('Host', '').lower()

        # Enrutamiento según Host o Path para el portal de marca 'vg.agua.comandamanda.com'
        is_brand_portal = ('vg.agua' in host) or ('vg.comanda' in host) or ('agua.comanda' in host) or path.startswith('/portal') or path in ('/marca', '/preventa')

        # 1. Si es el subdominio de marca 'vg.agua.comandamanda.com' en la raíz '/'
        if is_brand_portal and path in ('/', '/portal', '/portal/', '/marca', '/preventa'):
            portal_html = os.path.join(BASE_DIR, "portal", "index.html")
            self.serve_file(portal_html, is_head=is_head)
            return

        # 2. Endpoint de salud para sondeo continuo de reconexión
        if path == '/api/health':
            is_online = check_esp32_reachability()
            body = json.dumps({"esp32_online": is_online, "workshop_mode": not is_online}).encode('utf-8')
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", str(len(body)))
            self.send_header("Access-Control-Allow-Origin", "*")
            self.end_headers()
            if not is_head:
                self.wfile.write(body)
            return

        # 3. Status API (/api/status)
        if path == '/api/status':
            is_online = check_esp32_reachability()
            if is_online:
                try:
                    with urllib.request.urlopen(f"{ESP32_URL}/api/status", timeout=0.8) as resp:
                        data = resp.read()
                        self.send_response(200)
                        self.send_header("Content-Type", "application/json")
                        self.send_header("Content-Length", str(len(data)))
                        self.send_header("Access-Control-Allow-Origin", "*")
                        self.end_headers()
                        if not is_head:
                            self.wfile.write(data)
                        return
                except Exception:
                    pass
            body = json.dumps(get_simulated_status()).encode('utf-8')
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", str(len(body)))
            self.send_header("Access-Control-Allow-Origin", "*")
            self.end_headers()
            if not is_head:
                self.wfile.write(body)
            return

        # 4. Ruta directa a diagramas y taller
        if path in ('/bitacora', '/sensores', '/tds', '/ocr'):
            self.serve_file(os.path.join(BASE_DIR, "bitacora_sensores_gia.html"), is_head=is_head)
            return

        if path in ('/diagramas', '/atlas', '/taller'):
            self.serve_file(os.path.join(BASE_DIR, "workshop_ui.html"), is_head=is_head)
            return

        # 5. Raíz en aguadegia.comandamanda.com (ESP32 si está online, o Taller si está offline)
        if path == '/':
            is_online = check_esp32_reachability()
            if is_online:
                try:
                    with urllib.request.urlopen(f"{ESP32_URL}/", timeout=1.0) as resp:
                        data = resp.read()
                        self.send_response(200)
                        self.send_header("Content-Type", "text/html; charset=utf-8")
                        self.send_header("Content-Length", str(len(data)))
                        self.end_headers()
                        if not is_head:
                            self.wfile.write(data)
                        return
                except Exception:
                    pass
            self.serve_file(os.path.join(BASE_DIR, "live_ui.html"), is_head=is_head)
            return

        # 6. Imagen del Vitrolero
        if path == '/vitrolero.jpg':
            local_img = os.path.join(BASE_DIR, "assets", "agua_de_gia_jarra.jpg")
            if not os.path.exists(local_img):
                local_img = os.path.join(BASE_DIR, "vitrolero_gia_playas.jpg")
            self.serve_file(local_img, is_head=is_head)
            return

        # 7. Buscar archivos estáticos locales
        clean_path = path.lstrip('/')
        # Normalizar si viene con prefijo portal/
        if clean_path.startswith('portal/'):
            clean_path = clean_path[7:]

        local_candidates = [
            os.path.join(BASE_DIR, clean_path),
            os.path.join(BASE_DIR, "portal", clean_path),
            os.path.join(BASE_DIR, "assets", "videos", os.path.basename(clean_path)),
            os.path.join(BASE_DIR, "assets", clean_path),
            os.path.join(BASE_DIR, "assets", os.path.basename(clean_path))
        ]

        for cand in local_candidates:
            if os.path.isfile(cand):
                self.serve_file(cand, is_head=is_head)
                return

        # 8. Si es otra ruta y el ESP32 está online, retransmitir
        if check_esp32_reachability():
            try:
                with urllib.request.urlopen(f"{ESP32_URL}{self.path}", timeout=1.0) as resp:
                    data = resp.read()
                    self.send_response(resp.status)
                    for k, v in resp.headers.items():
                        if k.lower() not in ('transfer-encoding', 'connection'):
                            self.send_header(k, v)
                    self.end_headers()
                    if not is_head:
                        self.wfile.write(data)
                    return
            except Exception:
                pass

        # 404
        self.send_response(404)
        self.send_header("Content-Type", "text/plain; charset=utf-8")
        self.end_headers()
        if not is_head:
            self.wfile.write(b"Recurso no encontrado en servidor")

    def serve_file(self, filepath, is_head=False):
        try:
            mime_type, _ = mimetypes.guess_type(filepath)
            if mime_type is None:
                if filepath.endswith('.mp4'):
                    mime_type = 'video/mp4'
                elif filepath.endswith('.html'):
                    mime_type = 'text/html; charset=utf-8'
                elif filepath.endswith('.jpg') or filepath.endswith('.jpeg'):
                    mime_type = 'image/jpeg'
                elif filepath.endswith('.png'):
                    mime_type = 'image/png'
                else:
                    mime_type = 'application/octet-stream'

            file_size = os.path.getsize(filepath)

            # Soporte Range requests para streaming de video HTML5 en móviles
            range_header = self.headers.get('Range')
            if range_header and range_header.startswith('bytes='):
                range_match = range_header[6:].split('-')
                start = int(range_match[0]) if range_match[0] else 0
                end = int(range_match[1]) if range_match[1] else file_size - 1
                length = end - start + 1

                self.send_response(206)
                self.send_header('Content-Type', mime_type)
                self.send_header('Content-Range', f'bytes {start}-{end}/{file_size}')
                self.send_header('Content-Length', str(length))
                self.send_header('Accept-Ranges', 'bytes')
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()

                if not is_head:
                    with open(filepath, 'rb') as f:
                        f.seek(start)
                        self.wfile.write(f.read(length))
                return

            self.send_response(200)
            self.send_header("Content-Type", mime_type)
            self.send_header("Content-Length", str(file_size))
            self.send_header("Accept-Ranges", "bytes")
            self.send_header("Access-Control-Allow-Origin", "*")
            self.send_header("Cache-Control", "public, max-age=86400")
            self.end_headers()

            if not is_head:
                with open(filepath, 'rb') as f:
                    while chunk := f.read(65536):
                        self.wfile.write(chunk)
        except Exception as e:
            self.send_response(500)
            self.end_headers()
            if not is_head:
                self.wfile.write(str(e).encode('utf-8'))

def run_server():
    server = HTTPServer(('0.0.0.0', LOCAL_PORT), WorkshopHandler)
    print(f"[WORKSHOP SERVER] Escuchando en http://0.0.0.0:{LOCAL_PORT}")
    print(f"[WORKSHOP SERVER] Hostnames: aguadegia.comandamanda.com y vg.agua.comandamanda.com")
    server.serve_forever()

if __name__ == '__main__':
    run_server()
