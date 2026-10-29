import os, sys, subprocess, time
from pathlib import Path

def main():
    ffmpeg_exe = r"C:\Users\PEPITO\AppData\Local\Microsoft\WinGet\Links\ffmpeg.exe"
    frames_dir = Path(r"C:\Users\PEPITO\Downloads\123portijuana\montage_frames")
    tren_dir = Path(r"C:\Users\PEPITO\Downloads\123portijuana\tren_clips_render")
    out_dir = Path(r"C:\Users\PEPITO\Downloads")
    audio_file = Path(r"C:\Users\PEPITO\Downloads\123portijuana\Audio_Voz_Condensada_CapCut_3min.wav")
    output_video = out_dir / "Agua_de_Gaia_Edicion_Corta_3min_Con_Tren_Real.mp4"
    
    print("====================================================================")
    print("   MASTERIZACIÓN CORTE DINÁMICO (3 MIN) CON TREN DE FILTRADO REAL   ")
    print("====================================================================")
    
    # Timeline con sincronización milimétrica a los 7 bloques narrativos:
    # Duración total: 217.47s (3:37 min)
    # Formato: (nombre, path, t_start, t_end, is_video)
    timeline = [
        # Bloque 1: Hook Inicial y Gaia (0.0 a 33.0s)
        ("01_creador_jardin", frames_dir / "clip_03a_creador_jardin.mp4", 0.0, 8.0, True),
        ("02_gia_papa_noche", frames_dir / "montage_01b_gia_papa_real.jpg", 8.0, 20.0, False),
        ("03_filosofia_hoja", frames_dir / "montage_01_hoja1.jpg", 20.0, 33.0, False),
        
        # Bloque 2: Crisis de Tijuana y Dependencia Río Colorado (33.0 a 63.0s)
        ("04_canal_seco", frames_dir / "broll_01_canal.mp4", 33.0, 47.0, True),
        ("05_mapa_rio_colorado", frames_dir / "montage_10_mapa_rio_colorado.jpg", 47.0, 63.0, False),
        
        # Bloque 3: Descubrimiento: Pozo Somero a 7m y Ferretería (63.0 a 98.0s)
        ("06_tripode_derrick", frames_dir / "clip_03b_perforador_tripode.mp4", 63.0, 71.0, True),
        ("07_barretas", frames_dir / "montage_03b_barretas.jpg", 71.0, 78.0, False),
        ("08_perforacion_real", frames_dir / "clip_02_perforacion_real.mp4", 78.0, 87.0, True),
        ("09_endoscopio_pozo", frames_dir / "clip_03_inspeccion_endoscopio.mp4", 87.0, 94.0, True),
        ("10_boca_pozo", frames_dir / "montage_04b_boca_pozo.jpg", 94.0, 98.0, False),
        
        # Bloque 4: Tratamiento: Electrofloculación y Sedimentador Real (98.0 a 136.0s)
        ("11_sketchup_tratamiento", frames_dir / "clip_sketchup_01_tratamiento.mp4", 98.0, 106.0, True),
        ("12_reactor_floculacion_real", tren_dir / "clip_02.mp4", 106.0, 113.0, True),
        ("13_sedimentador_purga_real", tren_dir / "clip_03.mp4", 113.0, 118.0, True),
        ("14_floculacion_vaso", frames_dir / "clip_08c_floculacion_vaso.mp4", 118.0, 125.0, True),
        ("15_tira_reactiva", frames_dir / "montage_08b_tira_reactiva.jpg", 125.0, 130.0, False),
        ("16_bioensayo_langostinos", frames_dir / "broll_04_langostinos.mp4", 130.0, 136.0, True),
        
        # Bloque 5: Captación Pluvial y Recarga del Acuífero (136.0 a 171.0s)
        ("17_pizarron_calculo", frames_dir / "montage_09a_pizarron_calculo.jpg", 136.0, 144.0, False),
        ("18_pluvial_real_video", tren_dir / "clip_08.mp4", 144.0, 151.0, True),
        ("19_render_arquitectonico", frames_dir / "montage_10c_render_arquitectonico.jpg", 151.0, 161.0, False),
        ("20_sketchup_lluvia_acuifero", frames_dir / "clip_sketchup_02_lluvia_acuifero.mp4", 161.0, 171.0, True),
        
        # Bloque 6: Telemetría SCADA ESP32 y Tren POU en Vivo (171.0 a 199.0s)
        ("21_scada_dashboard", frames_dir / "montage_10a_scada_dashboard.jpg", 171.0, 183.0, False),
        ("22_planta_fisica_tren_ro", tren_dir / "clip_05.mp4", 183.0, 191.0, True),
        ("23_agua_manos", frames_dir / "clip_07e_agua_manos.mp4", 191.0, 199.0, True),
        
        # Bloque 7: El Verdadero Alquimista y Cierre (199.0 a 217.47s)
        ("24_gia_papa_cierre", frames_dir / "montage_11a_gia_papa.jpg", 199.0, 208.0, False),
        ("25_hoja_cierre_alquimista", frames_dir / "montage_12_hoja4_cierre.jpg", 208.0, 217.47, False)
    ]
    
    print(f"Total de segmentos visuales: {len(timeline)}")
    for name, path, t_start, t_end, is_video in timeline:
        if not path.exists():
            print(f"ERROR: Archivo no existe: {path}")
            sys.exit(1)
            
    fade_duration = 0.4
    
    cmd = [ffmpeg_exe, "-y"]
    # Input 0: Audio condensado
    cmd.extend(["-i", str(audio_file)])
    
    filter_parts = []
    
    # Inputs 1..N: Los elementos visuales
    for idx, (name, path, t_start, t_end, is_video) in enumerate(timeline, start=1):
        dur = t_end - t_start
        is_next_contiguous = (idx < len(timeline))
        
        if is_video:
            cmd.extend(["-i", str(path)])
        else:
            loop_dur = dur + 2.0
            cmd.extend(["-loop", "1", "-framerate", "30", "-t", f"{loop_dur:.2f}", "-i", str(path)])
            
        fade_in_str = f"fade=t=in:st=0:d={fade_duration}:alpha=1"
        
        if is_next_contiguous:
            fade_filter = f"[{idx}:v]format=yuva420p,{fade_in_str},setpts=PTS-STARTPTS+{t_start}/TB[ov{idx}]"
        else:
            fade_out_st = max(0.0, dur - fade_duration)
            fade_out_str = f"fade=t=out:st={fade_out_st:.2f}:d={fade_duration}:alpha=1"
            fade_filter = f"[{idx}:v]format=yuva420p,{fade_in_str},{fade_out_str},setpts=PTS-STARTPTS+{t_start}/TB[ov{idx}]"
            
        filter_parts.append(fade_filter)
        
    # Crear fondo negro base de 1280x720
    filter_parts.append(f"color=c=black:s=1280x720:d=217.47:r=30[base]")
    
    prev_stream = "[base]"
    for idx, (name, path, t_start, t_end, is_video) in enumerate(timeline, start=1):
        is_next_contiguous = (idx < len(timeline))
        render_end = t_end + (fade_duration if is_next_contiguous else 0.0)
        
        out_stream = f"[v{idx}]" if idx < len(timeline) else "[vout]"
        filter_parts.append(f"{prev_stream}[ov{idx}]overlay=0:0:enable='between(t,{t_start:.2f},{render_end:.2f})'{out_stream}")
        prev_stream = out_stream
        
    full_filter = ";\n".join(filter_parts)
    
    cmd.extend(["-filter_complex", full_filter])
    cmd.extend([
        "-map", "[vout]",
        "-map", "0:a",
        "-c:v", "libx264",
        "-crf", "18",
        "-preset", "fast",
        "-pix_fmt", "yuv420p",
        "-c:a", "aac",
        "-b:a", "192k",
        "-shortest",
        str(output_video)
    ])
    
    print("\nIniciando renderizado con FFmpeg...")
    start_t = time.time()
    
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, bufsize=1)
    
    for line in proc.stdout:
        line_s = line.strip()
        if "frame=" in line_s:
            print(f"\r{line_s[:85]}", end="", flush=True)
            
    proc.wait()
    print("\n")
    
    if proc.returncode == 0:
        elapsed = time.time() - start_t
        file_size_mb = output_video.stat().st_size / (1024 * 1024)
        print("====================================================================")
        print("           ¡CORTE DINÁMICO DE 3 MINUTOS GENERADO CON ÉXITO!         ")
        print("====================================================================")
        print(f"Archivo de salida: {output_video}")
        print(f"Tamaño total:      {file_size_mb:.2f} MB")
        print(f"Tiempo de proceso: {elapsed:.1f} segundos ({elapsed/60:.2f} min)")
        print(f"Duración:          03:37 min (217.47 segundos)")
        print("Audio:             Voz auténtica condensada sin tiempos muertos")
        print("Visuales:          25 cortes dinámicos con fundidos suaves de 0.4s")
        print("====================================================================")
    else:
        print(f"Error durante el renderizado (código {proc.returncode})")
        sys.exit(proc.returncode)

if __name__ == "__main__":
    main()
