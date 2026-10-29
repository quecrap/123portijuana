import os, subprocess, json

def get_res(p):
    cmd = ['ffprobe', '-v', 'error', '-show_entries', 'stream=width,height', '-of', 'json', p]
    res = subprocess.run(cmd, capture_output=True, text=True)
    try:
        data = json.loads(res.stdout)
        s = data['streams'][0]
        return f"{s['width']}x{s['height']}"
    except:
        return 'unknown'

d = r'C:\Users\PEPITO\Downloads\123portijuana\montage_frames'
if os.path.exists(d):
    for f in sorted(os.listdir(d)):
        if f.endswith('.mp4') or f.endswith('.jpg'):
            print(f, get_res(os.path.join(d, f)))
