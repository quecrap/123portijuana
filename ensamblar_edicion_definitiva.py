import os
import sys
import subprocess
import time

def main():
    ffmpeg_exe = r"C:\Users\PEPITO\AppData\Local\Microsoft\WinGet\Links\ffmpeg.exe"
    base_video = r"C:\Users\PEPITO\Downloads\Agua de Gaia.mp4"
    output_video = r"C:\Users\PEPITO\Downloads\Agua_de_Gaia_Edicion_Definitiva.mp4"
    frames_dir = r"C:\Users\PEPITO\Downloads\123portijuana\montage_frames"
    
    print("====================================================================")
    print("   MASTERIZACIÓN CINEMATOGRÁFICA DEFINITIVA (EDICIÓN SUPREMA - 36 SEG) ")
    print("====================================================================")
    
    # 36 segmentos con integración total de videos de floculación, SketchUp 3D,
    # render fotorrealista arquitectónico, planta piloto física, pizarra de cálculo,
    # dashboard SCADA ESP32, análisis químico y perforación real.
    # Formato de tupla: (nombre, path, t_start, t_end, is_video)
    segments = [
        # Acto 1: Introducción, Familia y Filosofía
        ("01a_hoja1", os.path.join(frames_dir, "montage_01_hoja1.jpg"), 38.0, 48.0, False),
        ("01b_gia_papa_real", os.path.join(frames_dir, "montage_01b_gia_papa_real.jpg"), 48.0, 58.0, False),
        ("02_hoja5", os.path.join(frames_dir, "montage_02_hoja5.jpg"), 58.0, 72.0, False),
        ("broll_01_canal", os.path.join(frames_dir, "broll_01_canal.mp4"), 100.0, 114.0, True),
        
        # Acto 2: El Origen, Los Inicios y La Perforación Épica
        ("03a_creador_jardin", os.path.join(frames_dir, "clip_03a_creador_jardin.mp4"), 165.0, 173.0, True),
        ("03b_perforador_tripode", os.path.join(frames_dir, "clip_03b_perforador_tripode.mp4"), 173.0, 181.0, True),
        ("03c_barretas", os.path.join(frames_dir, "montage_03b_barretas.jpg"), 181.0, 188.0, False),
        ("03d_derrick_foto", os.path.join(frames_dir, "montage_03d_derrick_foto.jpg"), 188.0, 198.0, False),
        ("broll_02_perforacion_real", os.path.join(frames_dir, "clip_02_perforacion_real.mp4"), 198.0, 212.0, True),
        ("04_excavacion", os.path.join(frames_dir, "montage_04_excavacion.jpg"), 212.0, 222.0, False),
        ("04b_boca_pozo", os.path.join(frames_dir, "montage_04b_boca_pozo.jpg"), 222.0, 230.0, False),
        ("05_corte_geol", os.path.join(frames_dir, "montage_05_corte_geol.jpg"), 230.0, 243.0, False),
        
        # Acto 3: El Brote, Inspección Subterránea y Tratamiento Avanzado
        ("06_aforo", os.path.join(frames_dir, "montage_06_aforo.jpg"), 243.0, 258.0, False),
        ("broll_03_inspeccion_endoscopio", os.path.join(frames_dir, "clip_03_inspeccion_endoscopio.mp4"), 258.0, 266.0, True),
        ("06b_bomba_pasto", os.path.join(frames_dir, "montage_06b_bomba_pasto.jpg"), 266.0, 272.0, False),
        ("07a_sketchup_tratamiento", os.path.join(frames_dir, "clip_sketchup_01_tratamiento.mp4"), 272.0, 282.0, True),
        ("07b_reactor_floculacion", os.path.join(frames_dir, "clip_07b_reactor_floculacion.mp4"), 282.0, 288.0, True),
        ("07c_burbujas_floculacion", os.path.join(frames_dir, "clip_07c_burbujas_floculacion.mp4"), 288.0, 294.0, True),
        ("07d_detalle_conico", os.path.join(frames_dir, "montage_10d_detalle_conico_floculador.jpg"), 294.0, 300.0, False),
        ("07e_planta_fisica", os.path.join(frames_dir, "montage_07d_planta_fisica.jpg"), 300.0, 306.0, False),
        ("07f_agua_manos", os.path.join(frames_dir, "clip_07e_agua_manos.mp4"), 306.0, 312.0, True),
        
        # Acto 4: Certificación, Laboratorio y Bioensayo Vivo
        ("08a_laboratorio_cert", os.path.join(frames_dir, "montage_08_laboratorio.jpg"), 312.0, 322.0, False),
        ("08b_tira_reactiva", os.path.join(frames_dir, "montage_08b_tira_reactiva.jpg"), 322.0, 332.0, False),
        ("08c_floculacion_vaso", os.path.join(frames_dir, "clip_08c_floculacion_vaso.mp4"), 332.0, 342.0, True),
        ("08d_membrana_ro", os.path.join(frames_dir, "montage_08d_membrana_ro.jpg"), 342.0, 348.0, False),
        ("broll_04_langostinos", os.path.join(frames_dir, "broll_04_langostinos.mp4"), 348.0, 355.0, True),
        
        # Acto 5: Captación Pluvial, Balance Hídrico y Recirculación
        ("09a_pizarron_calculo", os.path.join(frames_dir, "montage_09a_pizarron_calculo.jpg"), 355.0, 366.0, False),
        ("09b_pluvial_real", os.path.join(frames_dir, "montage_09a_pluvial_real.jpg"), 366.0, 376.0, False),
        ("09c_render_arquitectonico", os.path.join(frames_dir, "montage_10c_render_arquitectonico.jpg"), 376.0, 386.0, False),
        ("09d_sketchup_lluvia", os.path.join(frames_dir, "clip_sketchup_02_lluvia_acuifero.mp4"), 386.0, 398.0, True),
        ("09e_scada_dashboard", os.path.join(frames_dir, "montage_10a_scada_dashboard.jpg"), 398.0, 410.0, False),
        ("broll_05_recirculacion", os.path.join(frames_dir, "broll_05_recirculacion.mp4"), 410.0, 425.0, True),
        
        # Acto 6: Inteligencia Territorial, Familia y Manifiesto de Cierre
        ("10_mapa", os.path.join(frames_dir, "montage_10_mapa.jpg"), 440.0, 470.0, False),
        ("11a_gia_papa", os.path.join(frames_dir, "montage_11a_gia_papa.jpg"), 535.0, 555.0, False),
        ("11b_portada_pozo", os.path.join(frames_dir, "montage_11b_portada_pozo.jpg"), 555.0, 575.0, False),
        ("12_hoja4_cierre", os.path.join(frames_dir, "montage_12_hoja4_cierre.jpg"), 582.0, 600.47, False)
    ]
    
    print(f"Total de inserciones a procesar: {len(segments)}")
    
    # Validar que todos los archivos existan
    missing = []
    for name, path, t_start, t_end, is_video in segments:
        if not os.path.exists(path):
            missing.append(path)
    if missing:
        print("ERROR: Archivos faltantes:")
        for m in missing:
            print(f" - {m}")
        sys.exit(1)
        
    cmd = [ffmpeg_exe, "-y"]
    cmd.extend(["-i", base_video])
    
    filter_parts = []
    
    for idx, (name, path, t_start, t_end, is_video) in enumerate(segments, start=1):
        dur = t_end - t_start
        is_next_contiguous = False
        if idx < len(segments):
            next_t_start = segments[idx][2]
            if abs(next_t_start - t_end) < 0.1:
                is_next_contiguous = True
                
        is_prev_contiguous = False
        if idx > 1:
            prev_t_end = segments[idx-2][3]
            if abs(prev_t_end - t_start) < 0.1:
                is_prev_contiguous = True
                
        if is_video:
            cmd.extend(["-i", path])
        else:
            loop_dur = dur + 2.0
            cmd.extend(["-loop", "1", "-framerate", "30", "-t", f"{loop_dur:.2f}", "-i", path])
            
        fade_duration = 0.5
        fade_in_str = f"fade=t=in:st=0:d={fade_duration}:alpha=1"
        
        if is_next_contiguous:
            fade_filter = f"[{idx}:v]format=yuva420p,{fade_in_str},setpts=PTS-STARTPTS+{t_start}/TB[ov{idx}]"
            render_end = t_end + fade_duration
        else:
            fade_out_st = max(0.0, dur - fade_duration)
            fade_out_str = f"fade=t=out:st={fade_out_st:.2f}:d={fade_duration}:alpha=1"
            fade_filter = f"[{idx}:v]format=yuva420p,{fade_in_str},{fade_out_str},setpts=PTS-STARTPTS+{t_start}/TB[ov{idx}]"
            render_end = t_end
            
        filter_parts.append(fade_filter)
        
    prev_stream = "[0:v]"
    for idx, (name, path, t_start, t_end, is_video) in enumerate(segments, start=1):
        is_next_contiguous = False
        if idx < len(segments):
            next_t_start = segments[idx][2]
            if abs(next_t_start - t_end) < 0.1:
                is_next_contiguous = True
        render_end = t_end + (0.5 if is_next_contiguous else 0.0)
        
        out_stream = f"[v{idx}]" if idx < len(segments) else "[vout]"
        filter_parts.append(f"{prev_stream}[ov{idx}]overlay=0:0:enable='between(t,{t_start:.2f},{render_end:.2f})'{out_stream}")
        prev_stream = out_stream
        
    full_filter_complex = ";\n".join(filter_parts)
    
    cmd.extend(["-filter_complex", full_filter_complex])
    cmd.extend([
        "-map", "[vout]",
        "-map", "0:a",
        "-c:v", "libx264",
        "-crf", "19",
        "-preset", "fast",
        "-pix_fmt", "yuv420p",
        "-c:a", "copy",
        output_video
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
        file_size_mb = os.path.getsize(output_video) / (1024 * 1024)
        print("====================================================================")
        print("           ¡VIDEO MASTERIZADO Y COMPILADO CON ÉXITO!                ")
        print("====================================================================")
        print(f"Archivo de salida: {output_video}")
        print(f"Tamaño total:      {file_size_mb:.2f} MB")
        print(f"Tiempo de proceso: {elapsed:.1f} segundos ({elapsed/60:.2f} min)")
        print("Audio:             100% Original, intacto y perfectamente sincronizado")
        print("Transiciones:      Fundidos cruzados suaves de 0.5s")
        print(f"Inserciones:       {len(segments)} segmentos con videos de floculación en vivo,")
        print("                   planta física real, cálculo en pizarra, tiras de análisis,")
        print("                   renders 3D de alta gama y bioensayo vivo.")
        print("====================================================================")
    else:
        print(f"Error durante el renderizado (código {proc.returncode})")
        sys.exit(proc.returncode)

if __name__ == "__main__":
    main()
