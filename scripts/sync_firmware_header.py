import os
import gzip

BASE_DIR = r"c:\Users\PEPITO\Downloads\123portijuana"
LIVE_UI = os.path.join(BASE_DIR, "live_ui.html")
HEADER_H = os.path.join(BASE_DIR, "firmware", "main", "retro_washer_ui.h")

with open(LIVE_UI, "rb") as f:
    raw = f.read()

gz = gzip.compress(raw, 9)

# Formatear array C en líneas de 16 bytes
lines = []
for i in range(0, len(gz), 16):
    chunk = gz[i:i+16]
    lines.append("  " + ", ".join(f"0x{b:02X}" for b in chunk) + ",")

array_str = "\n".join(lines)

header_code = f"""#ifndef RETRO_WASHER_UI_H
#define RETRO_WASHER_UI_H

#include <Arduino.h>
#include "vitrolero_img.h"

// HTML comprimido con GZIP ({len(gz)} bytes en flash vs {len(raw)} bytes en plano)
const uint8_t RETRO_WASHER_HTML_GZ[] PROGMEM = {{
{array_str}
}};
const size_t RETRO_WASHER_HTML_GZ_LEN = {len(gz)};

#endif // RETRO_WASHER_UI_H
"""

with open(HEADER_H, "w", encoding="utf-8") as f:
    f.write(header_code)

print(f"[OK] retro_washer_ui.h generado con gzip ({len(gz)} bytes en flash vs {len(raw)} bytes en plano).")
