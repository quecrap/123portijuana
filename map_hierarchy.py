import bpy

glb_path = r"C:\Users\PEPITO\Downloads\sistema binaryfile.glb"
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=glb_path)

print("\n--- HIERARCHY SUMMARY ---")
assembly = bpy.data.objects.get("Assembly-234")
if assembly:
    for child in assembly.children:
        descendant_meshes = [o for o in bpy.data.objects if o.type == 'MESH' and (o == child or o.parent == child or (o.parent and o.parent.parent == child))]
        print(f"Group '{child.name}': {len(descendant_meshes)} mesh children")
