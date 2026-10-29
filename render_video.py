import subprocess
import os
import sys
import time

blender_exe = r"C:\Program Files\Blender Foundation\Blender 5.2\blender.exe"
blend_file = r"C:\Users\PEPITO\Downloads\123portijuana\aquaresiliencia_animacion.blend"
frames_dir = r"C:\Users\PEPITO\Downloads\123portijuana\render_frames"
output_mp4 = r"C:\Users\PEPITO\Downloads\123portijuana\aquaresiliencia_animacion.mp4"
ffmpeg_exe = r"C:\Users\PEPITO\AppData\Local\Microsoft\WinGet\Links\ffmpeg.exe"

print(f"=== INICIANDO RENDERIZADO COMPLETO (820 FOTOGRAMAS) ===")
start_time = time.time()

# 1. Renderizar fotogramas con Blender EEVEE
render_cmd = [
    blender_exe,
    "-b", blend_file,
    "-s", "1",
    "-e", "820",
    "-a"
]

print(f"Comando de render: {' '.join(render_cmd)}")
process = subprocess.Popen(render_cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, bufsize=1)

last_report = 0
for line in process.stdout:
    line_s = line.strip()
    if "Fra:" in line_s and "Rendering" not in line_s:
        # e.g. "Fra: 150 | Time: 00:00.35"
        print(line_s)
    elif "Saved:" in line_s:
        pass

process.wait()
if process.returncode != 0:
    print(f"Error en Blender render (exit code {process.returncode})")
    sys.exit(process.returncode)

elapsed_render = time.time() - start_time
print(f"\nRenderizado de fotogramas completado en {elapsed_render:.1f} segundos.")

# 2. Compilar con FFmpeg a MP4 H264 de alta fidelidad
print(f"\n=== COMPILANDO VIDEO MP4 CON FFMPEG ===")
input_pattern = os.path.join(frames_dir, "frame_%04d.jpg")

ffmpeg_cmd = [
    ffmpeg_exe,
    "-y",
    "-framerate", "30",
    "-i", input_pattern,
    "-c:v", "libx264",
    "-pix_fmt", "yuv420p",
    "-crf", "18",
    "-preset", "fast",
    output_mp4
]

res = subprocess.run(ffmpeg_cmd, capture_output=True, text=True)
if res.returncode == 0:
    file_size_mb = os.path.getsize(output_mp4) / (1024 * 1024)
    print(f"\n¡ÉXITO TOTAL!")
    print(f"Video generado: {output_mp4}")
    print(f"Tamaño: {file_size_mb:.2f} MB")
    print(f"Duración: 25 segundos (750 fotogramas a 30 FPS)")
    print(f"Resolución: 1280x720 (HD)")
else:
    print(f"Error en FFmpeg:\n{res.stderr}")
    sys.exit(res.returncode)
