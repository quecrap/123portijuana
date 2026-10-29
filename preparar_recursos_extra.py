import os
import subprocess
from PIL import Image, ImageFilter, ImageEnhance, ImageOps, ImageDraw

ffmpeg = r"C:\Users\PEPITO\AppData\Local\Microsoft\WinGet\Links\ffmpeg.exe"
pozo_dir = r"C:\Users\PEPITO\Downloads\123portijuana\Pozo"
portada_dir = r"C:\Users\PEPITO\Downloads\Portada para pozo -1-001\Portada para pozo"
frames_dir = r"C:\Users\PEPITO\Downloads\123portijuana\montage_frames"
os.makedirs(frames_dir, exist_ok=True)

target_w, target_h = 1280, 720

def process_photo(src_path, out_name):
    with Image.open(src_path) as img:
        img = ImageOps.exif_transpose(img).convert('RGB')
        orig_w, orig_h = img.size
        aspect = orig_w / orig_h
        
        bg_ratio = max(target_w / orig_w, target_h / orig_h)
        bg_w = int(orig_w * bg_ratio)
        bg_h = int(orig_h * bg_ratio)
        bg = img.resize((bg_w, bg_h), Image.Resampling.LANCZOS)
        left = (bg_w - target_w) // 2
        top = (bg_h - target_h) // 2
        bg = bg.crop((left, top, left + target_w, top + target_h))
        bg = bg.filter(ImageFilter.GaussianBlur(radius=30))
        bg = ImageEnhance.Brightness(bg).enhance(0.55)
        
        max_fg_w = target_w - 32
        max_fg_h = target_h - 32
        fg_ratio = min(max_fg_w / orig_w, max_fg_h / orig_h)
        fg_w = int(orig_w * fg_ratio)
        fg_h = int(orig_h * fg_ratio)
        fg = img.resize((fg_w, fg_h), Image.Resampling.LANCZOS)
        
        offset_x = (target_w - fg_w) // 2
        offset_y = (target_h - fg_h) // 2
        
        shadow = Image.new('RGBA', (fg_w + 20, fg_h + 20), (0, 0, 0, 0))
        draw = ImageDraw.Draw(shadow)
        draw.rectangle([10, 10, fg_w + 10, fg_h + 10], fill=(0, 0, 0, 160))
        shadow = shadow.filter(ImageFilter.GaussianBlur(radius=8))
        
        bg_rgba = bg.convert('RGBA')
        bg_rgba.paste(shadow, (offset_x - 10, offset_y - 10), shadow)
        bg_rgba.paste(fg, (offset_x, offset_y))
        final_frame = bg_rgba.convert('RGB')
        
        out_path = os.path.join(frames_dir, out_name)
        final_frame.save(out_path, quality=95)
        print(f"Foto procesada: {out_name} desde {os.path.basename(src_path)}")

# 1. Procesar fotos adicionales
process_photo(os.path.join(portada_dir, "IMG_20220215_192451.jpg"), "montage_03b_barretas.jpg")
process_photo(os.path.join(portada_dir, "IMG_20220226_175631.jpg"), "montage_04b_boca_pozo.jpg")
process_photo(os.path.join(pozo_dir, "IMG_20220305_103546.jpg"), "montage_06b_bomba_pasto.jpg")

# 2. Procesar clips de video B-Roll a 1280x720 30 fps
def convert_broll_landscape(src_path, out_name, duration, start_sec=0):
    out_path = os.path.join(frames_dir, out_name)
    cmd = [
        ffmpeg, "-y",
        "-ss", str(start_sec), "-t", str(duration),
        "-i", src_path,
        "-vf", "scale=1280:720,fps=30",
        "-c:v", "libx264", "-preset", "fast", "-crf", "19",
        "-an",
        out_path
    ]
    subprocess.run(cmd, check=True)
    print(f"Video B-Roll landscape listo: {out_name} ({duration}s)")

def convert_broll_vertical(src_path, out_name, duration, start_sec=0, loop_count=1):
    out_path = os.path.join(frames_dir, out_name)
    # Loop input if duration exceeds source
    filter_chain = (
        "[0:v]split[bg][fg];"
        "[bg]scale=1280:720:force_original_aspect_ratio=increase,crop=1280:720,boxblur=25:5,eq=brightness=-0.1:contrast=0.9[bg_blur];"
        "[fg]scale=-1:720[fg_scaled];"
        "[bg_blur][fg_scaled]overlay=(W-w)/2:(H-h)/2,fps=30[vout]"
    )
    
    cmd = [ffmpeg, "-y"]
    if loop_count > 1:
        cmd.extend(["-stream_loop", str(loop_count)])
    cmd.extend([
        "-ss", str(start_sec), "-t", str(duration),
        "-i", src_path,
        "-filter_complex", filter_chain,
        "-map", "[vout]",
        "-c:v", "libx264", "-preset", "fast", "-crf", "19",
        "-an",
        out_path
    ])
    subprocess.run(cmd, check=True)
    print(f"Video B-Roll vertical pillarbox listo: {out_name} ({duration}s)")

# B-Roll 1: Canal de Tijuana (14s)
convert_broll_landscape(os.path.join(pozo_dir, "02_B-ROLL_Contexto_Canal_Tijuana.mp4"), "broll_01_canal.mp4", 14.0, start_sec=0)

# B-Roll 2: Perforación en el patio (14s)
convert_broll_vertical(os.path.join(pozo_dir, "04_B-ROLL_Proceso_Perforacion_Patio_Toma2.mp4"), "broll_02_perforacion.mp4", 14.0, start_sec=2)

# B-Roll 3: Inspección de fondo de pozo con cámara nocturna (8s)
convert_broll_vertical(os.path.join(pozo_dir, "03_B-ROLL_Inspeccion_Fondo_Pozo.mp4"), "broll_03_inspeccion.mp4", 8.0, start_sec=5)

# B-Roll 4: Bioensayo con langostinos (10s, loop x2 porque el video dura 4.6s)
convert_broll_vertical(os.path.join(pozo_dir, "08_B-ROLL_Bioensayo_Langostinos.mp4"), "broll_04_langostinos.mp4", 10.0, start_sec=0, loop_count=2)

# B-Roll 5: Sistema de recirculación y tambos (30s)
convert_broll_vertical(os.path.join(pozo_dir, "VID_20220403_160509_0_COMPRESSED.mp4"), "broll_05_recirculacion.mp4", 30.0, start_sec=3)

print("¡TODOS LOS RECURSOS EXTRA FUERON PREPARADOS Y ESTANDARIZADOS EXITOSAMENTE!")
