import bpy
import mathutils

bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=r"C:\Users\PEPITO\Downloads\sistema binaryfile.glb")

con = bpy.data.objects.get("6_Conexiones_Hidraulicas")
if con:
    def traverse(o):
        if o.type == 'MESH':
            bb = [o.matrix_world @ mathutils.Vector(c) for c in o.bound_box]
            min_c = mathutils.Vector((min([v.x for v in bb]), min([v.y for v in bb]), min([v.z for v in bb])))
            max_c = mathutils.Vector((max([v.x for v in bb]), max([v.y for v in bb]), max([v.z for v in bb])))
            print(f"Mesh: {o.name} | parent={o.parent.name} | center={(min_c+max_c)/2} | dim={max_c-min_c}")
        for c in o.children:
            traverse(c)
    traverse(con)
