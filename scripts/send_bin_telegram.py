import urllib.request
import os

token = '9037737988:AAFHi7iAcQ4j0hApLGyF_TZLAEzgRnXzTDc'
chat_id = '7959093224'
bin_path = r'C:\Users\PEPITO\Downloads\123portijuana\firmware_bin\main.ino.bin'

boundary = '----WebKitFormBoundary7MA4YWxkTrZu0gW'
body = bytearray()

# chat_id
body.extend(f'--{boundary}\r\nContent-Disposition: form-data; name="chat_id"\r\n\r\n{chat_id}\r\n'.encode('utf-8'))

# caption
caption = "⚡ FIRMWARE ACTUALIZADO · AGUA DE GÍA\n\n• Pin 5 (GPIO 34) con divisor 4:1 activo.\n• Pin 3 (GPIO 36) galvánico pasivo.\n• Modo 100% manual sin cortes automáticos.\n• Integrador Faraday y Amperaje en vivo.\n\nPara subirlo al ESP32:\n1. Conéctate a Wi-Fi: AguaDeGia_Lavadora\n2. Abre: http://192.168.4.1/update\n3. Sube este archivo .bin"
body.extend(f'--{boundary}\r\nContent-Disposition: form-data; name="caption"\r\n\r\n{caption}\r\n'.encode('utf-8'))

# document
body.extend(f'--{boundary}\r\nContent-Disposition: form-data; name="document"; filename="main.ino.bin"\r\nContent-Type: application/octet-stream\r\n\r\n'.encode('utf-8'))
with open(bin_path, 'rb') as f:
    body.extend(f.read())
body.extend(f'\r\n--{boundary}--\r\n'.encode('utf-8'))

req = urllib.request.Request(
    f'https://api.telegram.org/bot{token}/sendDocument',
    data=bytes(body),
    headers={'Content-Type': f'multipart/form-data; boundary={boundary}'}
)

try:
    with urllib.request.urlopen(req, timeout=40) as resp:
        print("Telegram document sent successfully! Status:", resp.status)
except Exception as e:
    print("Error sending document to Telegram:", e)

