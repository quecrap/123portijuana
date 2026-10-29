import bpy

glb_path = r"C:\Users\PEPITO\Downloads\sistema binaryfile.glb"
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=glb_path)

import mathutils

for grp_name in ["1_Pozo_Y_Bomba", "3_IBC_Superior", "4_Tanque_Conico_1BBL", "5_Electrofloculador_Detallado", "8_Modulo_Osmosis_Inversa", "9_Barril_Presurizado"]:
    obj = bpy.data.objects.get(grp_name)
    if obj:
        # compute bounding box in world space
        meshes = [o for o in bpy.data.objects if o.type == 'MESH' and (o == obj or o.parent == obj or (o.parent and o.parent.parent == obj))]
        if meshes:
            min_co = [1e9, 1e9, 1e9]
            max_co = [-1e9, -1e9, -1e9]
            for m in meshes:
                for v in m.bound_box:
                    world_v = m.matrix_world @ mathutils.Vector(v)
                    for i in range(3):
                        min_co[i] = min(min_co[i], world_v[i])
                        max_co[i] = max(max_co[i], world_v[i])
            print(f"'{grp_name}': Min=({min_co[0]:.2f}, {min_co[1]:.2f}, {min_co[2]:.2f}), Max=({max_co[0]:.2f}, {max_co[1]:.2f}, {max_co[2]:.2f})")
