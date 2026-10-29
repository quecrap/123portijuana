import json
from pathlib import Path

def fmt_ass_time(sec):
    h = int(sec // 3600)
    m = int((sec % 3600) // 60)
    s = int(sec % 60)
    cs = int(round((sec - int(sec)) * 100))
    if cs >= 100:
        cs = 99
    return f"{h}:{m:02d}:{s:02d}.{cs:02d}"

json_file = Path(r"C:\Users\PEPITO\Downloads\123portijuana\transcripcion_whisper_3min.json")
segments = json.loads(json_file.read_text(encoding="utf-8"))

replacements = [
    ("El pasaría", "¿Qué pasaría"),
    ("por lo que el se conoce", "Solo lo que se conoce"),
    ("agua de guía", "Agua de Gaia"),
    ("agua de gua", "Agua de Gaia"),
    ("Agua de guía", "Agua de Gaia"),
    ("agua de galla", "Agua de Gaia"),
    ("galla", "Gaia"),
    ("Galla", "Gaia"),
    ("agua y residencia", "AquaResiliencia"),
    ("y generara accesible", "ingeniería accesible"),
    ("y generaría accesible", "ingeniería accesible"),
    ("automatización y hasta recargan de la cuífero", "automatización y hasta recarga del acuífero"),
    ("la cuífero", "el acuífero"),
    ("Tu nombre viene", "Su nombre viene"),
    ("sino de comprender", "se trata de comprender"),
    ("transformarnos en torno", "transformar nuestro entorno"),
    ("absecimiento", "abastecimiento"),
    ("depende en la medida", "depende en gran medida"),
    ("milerzos", "muy lejos"),
    ("reo colgado", "Río Colorado"),
    ("contratado de más de 40 años", "con tratados de más de 40 años"),
    ("agua someda, yo con el equipo", "agua con el equipo"),
    ("agua someda", "agua somera"),
    ("baja realidad", "baja calidad"),
    ("un hueltsudor", "como el sudor"),
    ("eso me da que quede", "esa humedad que queda"),
    ("eso me da", "esa humedad"),
    ("un cerquito abajo", "un charquito abajo"),
    ("mínimas se las comparas", "mínimas si las comparas"),
    ("para su publicación", "para su purificación"),
    ("procuración", "biofloculación"),
    ("procuracin", "biofloculación"),
    ("llones", "iones"),
    ("la buen cuestión", "al agua en cuestión"),
    ("como nozo", "como un pozo"),
    ("caudal y el sol, es un chiste", "caudal sutil, casi un chiste"),
    ("por telos", "por minuto"),
    ("al que pedir si no también hay que dar", "hay que pedir, sino también hay que dar"),
    ("aprende a la humanidad", "ha aprendido la humanidad"),
    ("400 sin colguitos", "400 o 500 litros"),
    ("posto tendr", "pozo tendr"),
    ("posto", "pozo"),
    ("es una mayoría de ella", "en su mayoría de ella"),
    ("Agua de Yía", "Agua de Gaia"),
    ("Agua de Ya", "Agua de Gaia"),
]

def clean_text(txt):
    for k, v in replacements:
        txt = txt.replace(k, v)
        txt = txt.replace(k.lower(), v)
    return txt.strip()

ass_header = """[Script Info]
Title: Agua de Gaia - Subtitulos Estilizados y Badges
ScriptType: v4.00+
PlayResX: 1280
PlayResY: 720
ScaledBorderAndShadow: yes

[V4+ Styles]
Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding
Style: TitleBadge,Montserrat,26,&H00FFFFFF,&H000000FF,&H00000000,&H90000000,-1,0,0,0,100,100,1,0,3,10,0,8,40,40,36,1
Style: SubBadge,Montserrat,17,&H0000FFFF,&H000000FF,&H00000000,&H90000000,0,0,0,0,100,100,1,0,3,6,0,8,40,40,70,1
Style: SubtitleVoice,Arial,22,&H00FFFFFF,&H000000FF,&H00000000,&H80000000,-1,0,0,0,100,100,0,0,3,8,0,2,30,30,32,1

[Events]
Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text
"""

badges = [
    (1.0, 9.5, "AGUA DE GAIA", "¿QUÉ HAY BAJO NUESTROS PIES?"),
    (9.5, 29.5, "GAIA: MI HIJA Y MI HERENCIA", "El origen de AquaResiliencia Tijuana"),
    (33.0, 47.0, "CRISIS HÍDRICA EN TIJUANA", "Dependencia del 90% del Río Colorado"),
    (47.0, 63.0, "ACUEDUCTO RÍO COLORADO - TIJUANA", "Presa El Carrizo (Reserva Crítica: 3 Semanas)"),
    (63.0, 75.5, "TECNOLOGÍA FRUGAL DE FERRETERÍA", "Bajo costo con alto rigor de ingeniería"),
    (75.5, 98.0, "POZO SOMERO A 7 METROS", "Perforación en aluvión y manto freático"),
    (98.0, 112.5, "DE LA MAQUILA A LA ALQUIMIA", "Planta Piloto de Tratamiento y Sedimentación"),
    (112.5, 137.0, "ELECTROFLOCULACIÓN CONTINUA", "Intercambio de iones: bajando dureza en minutos"),
    (137.0, 159.5, "CAPTACIÓN PLUVIAL EN VIVIENDA", "En el pedir está el dar · Recarga del acuífero"),
    (160.0, 180.5, "400 LITROS POR DÍA", "Suficiente para abastecer un hogar"),
    (181.0, 201.5, "SCADA IoT: DATOS Y CÓDIGO ABIERTO", "ESP32 en tiempo real · Nivel, temperatura y caudal"),
    (201.5, 216.5, "EL VERDADERO ALQUIMISTA", "El oro puede comprar casi todo... jamás la vida.")
]

events = []

for t_start, t_end, title, sub in badges:
    s_str = fmt_ass_time(t_start)
    e_str = fmt_ass_time(t_end)
    events.append(f"Dialogue: 1,{s_str},{e_str},TitleBadge,,0,0,0,,{{\\c&H00FFFF&}}{title}{{\\r}}")
    events.append(f"Dialogue: 1,{s_str},{e_str},SubBadge,,0,0,0,,{sub}")

for seg in segments:
    t_start = seg["start"]
    t_end = seg["end"]
    txt = clean_text(seg["text"])
    
    for kw in ["Agua de Gaia", "AquaResiliencia", "Río Colorado", "Presa El Carrizo", "Tijuana", "Gaia", "alquimista"]:
        if kw in txt:
            txt = txt.replace(kw, f"{{\\c&H00FFFF&}}{kw}{{\\r}}")
            
    s_str = fmt_ass_time(t_start)
    e_str = fmt_ass_time(t_end)
    events.append(f"Dialogue: 0,{s_str},{e_str},SubtitleVoice,,0,0,0,,{txt}")

out_path = Path(r"C:\Users\PEPITO\Downloads\123portijuana\subtitulos_completos.ass")
out_path.write_text(ass_header + "\n".join(events), encoding="utf-8")
print(f"Subtítulos pulidos generados con éxito: {out_path} ({len(events)} eventos)")
