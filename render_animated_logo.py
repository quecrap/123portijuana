import os, math
import numpy as np
from PIL import Image, ImageDraw, ImageFont, ImageFilter
import subprocess

def create_animated_logo():
    out_dir = r"C:\Users\PEPITO\Downloads\123portijuana\temp_logo_frames"
    os.makedirs(out_dir, exist_ok=True)
    
    fps = 30
    duration_sec = 5.0
    total_frames = int(fps * duration_sec) # 150 frames
    
    font_title = ImageFont.truetype(r"C:\Windows\Fonts\segoeuib.ttf", 44)
    font_sub = ImageFont.truetype(r"C:\Windows\Fonts\segoeuib.ttf", 19)
    font_icon = ImageFont.truetype(r"C:\Windows\Fonts\seguiemj.ttf", 52) # Windows emoji font for droplet
    
    w, h = 640, 160
    
    print(f"Generando {total_frames} fotogramas para animación de logo 'Agua de Gaia'...")
    
    for i in range(total_frames):
        t = i / float(fps) # 0.0 to 5.0s
        
        # Alpha curve (fade in 0.0-0.7s, hold 0.7-4.3s, fade out 4.3-5.0s)
        if t < 0.7:
            alpha_master = t / 0.7
            y_float = (1.0 - alpha_master) * 25.0
        elif t > 4.3:
            alpha_master = max(0.0, (5.0 - t) / 0.7)
            y_float = (1.0 - alpha_master) * -15.0
        else:
            alpha_master = 1.0
            # Gentle breathing bobbing
            y_float = math.sin((t - 0.7) * 2.5) * 4.0
            
        frame = Image.new("RGBA", (w, h), (0, 0, 0, 0))
        draw = ImageDraw.Draw(frame)
        
        # Pill bounds
        bx0, by0, bx1, by1 = 15, int(15 + y_float), w - 15, int(h - 15 + y_float)
        
        # Ripple effect behind droplet
        ripple_t = (t * 1.5) % 1.0 # 0.0 to 1.0 loop
        r_radius = 25 + int(ripple_t * 55)
        r_alpha = int((1.0 - ripple_t) * 160 * alpha_master)
        drop_cx, drop_cy = 75, int(by0 + (by1 - by0) / 2)
        
        if r_alpha > 0:
            draw.ellipse(
                [drop_cx - r_radius, drop_cy - r_radius, drop_cx + r_radius, drop_cy + r_radius],
                outline=(0, 220, 255, r_alpha),
                width=3
            )
            
        # Draw background pill with glassmorphism
        pill_fill = (12, 28, 48, int(210 * alpha_master))
        pill_outline = (0, 235, 255, int(240 * alpha_master))
        draw.rounded_rectangle([bx0, by0, bx1, by1], radius=32, fill=pill_fill, outline=pill_outline, width=3)
        
        # Inner subtle glow highlight
        inner_pill = [bx0 + 2, by0 + 2, bx1 - 2, by0 + int((by1 - by0)*0.45)]
        draw.rounded_rectangle(inner_pill, radius=30, fill=(255, 255, 255, int(22 * alpha_master)))
        
        # Draw Water Droplet (Polygon art water droplet)
        # We draw a crisp vector water droplet
        dw_x, dw_y = 75, drop_cy
        drop_pts = [
            (dw_x, dw_y - 28),
            (dw_x + 18, dw_y + 4),
            (dw_x + 14, dw_y + 22),
            (dw_x, dw_y + 28),
            (dw_x - 14, dw_y + 22),
            (dw_x - 18, dw_y + 4)
        ]
        draw.polygon(drop_pts, fill=(0, 220, 255, int(245 * alpha_master)))
        # Inner droplet reflection
        draw.ellipse([dw_x - 7, dw_y - 8, dw_x + 2, dw_y + 12], fill=(255, 255, 255, int(180 * alpha_master)))
        
        # Texts
        text_white = (255, 255, 255, int(255 * alpha_master))
        text_cyan = (0, 235, 255, int(245 * alpha_master))
        
        draw.text((125, by0 + 22), "AGUA DE GIA", font=font_title, fill=text_white)
        draw.text((128, by0 + 78), "AQUARESILIENCIA TIJUANA  ·  2026", font=font_sub, fill=text_cyan)
        
        # Save frame
        frame_path = os.path.join(out_dir, f"frame_{i:04d}.png")
        frame.save(frame_path)
        
    print("Fotogramas de logo generados. Compilando a video MOV con canal Alfa...")
    mov_out = r"C:\Users\PEPITO\Downloads\123portijuana\logo_agua_de_gia_animado.mov"
    ffmpeg_exe = r"C:\Users\PEPITO\AppData\Local\Microsoft\WinGet\Links\ffmpeg.exe"
    
    cmd = [
        ffmpeg_exe, "-y",
        "-framerate", str(fps),
        "-i", os.path.join(out_dir, "frame_%04d.png"),
        "-c:v", "png",
        "-pix_fmt", "rgba",
        mov_out
    ]
    res = subprocess.run(cmd, capture_output=True, text=True)
    if res.returncode == 0:
        print(f"¡Éxito! Logo animado generado: {mov_out} ({os.path.getsize(mov_out)/1024:.1f} KB)")
    else:
        print("Error en FFmpeg:", res.stderr)

if __name__ == "__main__":
    create_animated_logo()
