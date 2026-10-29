import bpy
import os

# Clear existing objects in default scene
bpy.ops.wm.read_factory_settings(use_empty=True)

glb_path = r"C:\Users\PEPITO\Downloads\sistema binaryfile.glb"

if os.path.exists(glb_path):
    print(f"Importing GLB from: {glb_path}")
    bpy.ops.import_scene.gltf(filepath=glb_path)
    print(f"Total objects imported: {len(bpy.data.objects)}")
    for obj in bpy.data.objects[:30]:
        print(f" - {obj.name} (type={obj.type}, parent={obj.parent.name if obj.parent else None})")
else:
    print(f"GLB not found at {glb_path}")
