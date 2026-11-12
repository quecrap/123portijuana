with open('visor_satelital_tijuana.html', 'r', encoding='utf-8') as f:
    lines = f.readlines()

# Encontramos la sección dentro de zonasInSAR
start_idx = None
end_idx = None

for i, l in enumerate(lines[:700]):
    if "// Renderizado condicional de series temporales" in l and i < 650:
        start_idx = i
        break

if start_idx is not None:
    for j in range(start_idx, start_idx + 30):
        if "const popupHtml = `" in lines[j]:
            end_idx = j
            break

print(f"Borrando lineas desde {start_idx+1} hasta {end_idx+1}")
del lines[start_idx:end_idx]

with open('visor_satelital_tijuana.html', 'w', encoding='utf-8') as f:
    f.writelines(lines)

print("visor_satelital_tijuana.html limpiado.")
