import os
import sys
import subprocess
import time
from pathlib import Path

def main():
    ffmpeg_exe = r"C:\Users\PEPITO\AppData\Local\Microsoft\WinGet\Links\ffmpeg.exe"
    src_video = r"C:\Users\PEPITO\Downloads\TREN DE PROCESO.mp4"
    work_dir = Path(r"C:\Users\PEPITO\Downloads\123portijuana\tren_clips_render")
    work_dir.mkdir(parents=True, exist_ok=True)
    
    output_video = Path(r"C:\Users\PEPITO\Downloads\DEMO_TREN_DE_FILTRADO_PROTOTIPO_REAL.mp4")
    
    print("====================================================================")
    print("   ENSAMBLANDO DEMO MAESTRO: TREN DE PROCESO Y FILTRADO REAL        ")
    print("====================================================================")
    
    if not os.path.exists(src_video):
        print(f"Error: no existe el video origen: {src_video}")
        sys.exit(1)
        
    clips_def = [
        {
            "id": "clip_01",
            "start": 0.0,
            "end": 13.0,
            "tag": "01. CAPTACION SOMERA // EXTRACCION DE BAJO IMPACTO",
            "desc": "Bomba Evans 24V / 2A (Bajo consumo) llenando Tótem IBC superior de 1,000 L",
            "stat": "POTENCIA: 48W | NIVEL: 7.0m"
        },
        {
            "id": "clip_02",
            "start": 13.5,
            "end": 44.0,
            "tag": "02. ELECTROFLOCULACION CONTINUA // INTERCAMBIO IONICO",
            "desc": "Reactor sanitario en acero inoxidable clamp con ánodo sacrificial de fierro (24V / 2A)",
            "stat": "DESESTABILIZACION COLOIDAL"
        },
        {
            "id": "clip_03",
            "start": 58.0,
            "end": 83.0,
            "tag": "03. CLARIFICADOR CICLONICO // PURGA INFERIOR DE LODOS",
            "desc": "Entrada tangencial lenta en remolino para decantar y purgar flóculos de óxido",
            "stat": "SEDIMENTACION TANGENCIAL"
        },
        {
            "id": "clip_04",
            "start": 91.0,
            "end": 105.0,
            "tag": "04. LINEA GENERAL DOMESTICA (80/20) // ALMACENAMIENTO",
            "desc": "Filtración intermedia por gravedad hacia el segundo Tótem IBC para uso sanitario y lavado",
            "stat": "ARQUITECTURA 80/20"
        },
        {
            "id": "clip_05",
            "start": 106.0,
            "end": 142.0,
            "tag": "05. TREN DE ULTRAPURIFICACION POU // AGUA DE INGESTA",
            "desc": "Pichancha flotante + Bomba Booster 120 PSI + Cartucho 5u + Carbón CTO + Ósmosis Inversa",
            "stat": "CUMPLIMIENTO NOM-127-SSA1"
        },
        {
            "id": "clip_06",
            "start": 144.0,
            "end": 160.0,
            "tag": "06. REGULACION ACTIVA Y CERO DESPERDICIO [85-90 PSI]",
            "desc": "Válvula de aguja para presión óptima y recirculación de rechazo al tanque superior",
            "stat": "CIRCUITO CERRADO DE RECHAZO"
        },
        {
            "id": "clip_07",
            "start": 182.0,
            "end": 204.0,
            "tag": "07. INGENIERIA HIDRODINAMICA // LEY DE STOKES",
            "desc": "Velocidad ascensional de extracción menor a la caída de arenas para evitar colmatación",
            "stat": "CONTROL ANTI-AZOLVE"
        },
        {
            "id": "clip_08",
            "start": 266.0,
            "end": 278.0,
            "tag": "08. CIERRE DE CICLO // RECARGA ARTIFICIAL PLUVIAL",
            "desc": "Canalización de lluvia desde la cubierta del techo para recargar el acuífero somero",
            "stat": "RESILIENCIA HIDRICA TOTAL"
        }
    ]
    
    rendered_clips = []
    
    for i, c in enumerate(clips_def, 1):
        dur = c["end"] - c["start"]
        out_part = work_dir / f"{c['id']}.mp4"
        print(f"[{i}/{len(clips_def)}] Procesando {c['id']}: {c['tag']} ({dur:.1f}s)...")
        
        # Filtro de video: fondo borroso + video escalado + cajas HUD y textos
        # Escapar porcentaje para ffmpeg drawtext (%), comillas y dos puntos
        tag_esc = c['tag'].replace("%", "%%").replace(":", r"\:").replace("'", "")
        desc_esc = c['desc'].replace("%", "%%").replace(":", r"\:").replace("'", "")
        stat_esc = c['stat'].replace("%", "%%").replace(":", r"\:").replace("'", "")
        
        vf = (
            f"split=2[v_bg][v_fg];"
            f"[v_bg]scale=1280:720:force_original_aspect_ratio=increase,crop=1280:720,boxblur=25:5[bg];"
            f"[v_fg]scale=-2:720[fg];"
            f"[bg][fg]overlay=(W-w)/2:0[comp];"
            f"[comp]drawbox=y=0:h=75:color=black@0.70:t=fill,"
            f"drawbox=y=650:h=70:color=black@0.70:t=fill,"
            f"drawbox=y=73:h=2:color=0x00E5FF@0.8:t=fill,"
            f"drawbox=y=650:h=2:color=0x10B981@0.8:t=fill,"
            f"drawtext=text='{tag_esc}':fontcolor=0x00E5FF:fontsize=20:x=30:y=15,"
            f"drawtext=text='{desc_esc}':fontcolor=white:fontsize=15:x=30:y=42,"
            f"drawtext=text='AQUARESILIENCIA TIJUANA // PLANTA PILOTO REAL':fontcolor=0x94A3B8:fontsize=14:x=30:y=665,"
            f"drawtext=text='{stat_esc}':fontcolor=0x10B981:fontsize=14:x=1250-tw:y=665"
        )
        
        cmd = [
            ffmpeg_exe, "-y",
            "-ss", str(c["start"]),
            "-to", str(c["end"]),
            "-i", src_video,
            "-filter_complex", vf,
            "-c:v", "libx264", "-preset", "veryfast", "-crf", "20",
            "-c:a", "aac", "-b:a", "192k",
            str(out_part)
        ]
        
        res = subprocess.run(cmd, capture_output=True, text=True)
        if res.returncode != 0:
            print(f"Error procesando {c['id']}:")
            print(res.stderr[-500:])
            sys.exit(1)
            
        rendered_clips.append(out_part)
        print(f"   -> OK: {out_part.name}")
        
    print("\nConcatenando clips en el video final...")
    list_file = work_dir / "concat_list.txt"
    with open(list_file, "w", encoding="utf-8") as f:
        for p in rendered_clips:
            # En ffmpeg concat demuxer, usar forward slashes
            p_str = str(p.resolve()).replace("\\", "/")
            f.write(f"file '{p_str}'\n")
            
    concat_cmd = [
        ffmpeg_exe, "-y",
        "-f", "concat",
        "-safe", "0",
        "-i", str(list_file),
        "-c", "copy",
        str(output_video)
    ]
    
    res = subprocess.run(concat_cmd, capture_output=True, text=True)
    if res.returncode != 0:
        print("Error en concatenación:")
        print(res.stderr[-500:])
        sys.exit(1)
        
    sz_mb = output_video.stat().st_size / (1024 * 1024)
    print(f"\n====================================================================")
    print(f"   DEMO COMPLETADO CON ÉXITO: {output_video.name}")
    print(f"   Ubicación: {output_video}")
    print(f"   Tamaño: {sz_mb:.2f} MB")
    print(f"====================================================================")

if __name__ == "__main__":
    main()
