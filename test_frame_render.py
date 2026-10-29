"""
Ajuste de materiales translúcidos, fluidos visibles y cámaras
"""
import bpy
import mathutils
import math
import os

def aplicar_materiales_y_camaras():
    scene = bpy.context.scene

    # Habilitar trazado de rayos / refracciones en EEVEE
    if hasattr(scene, 'eevee'):
        scene.eevee.taa_render_samples = 32
        if hasattr(scene.eevee, 'use_raytracing'):
            scene.eevee.use_raytracing = True

    # Materiales con transparencia real para ver el interior
    def mat_translucido(nombre, diffuse=(0.9, 0.95, 1.0, 1.0), alpha=0.35, roughness=0.15, transmission=0.85):
        mat = bpy.data.materials.get(nombre)
        if not mat:
            mat = bpy.data.materials.new(name=nombre)
        mat.use_nodes = True
        mat.blend_method = 'HASHED'
        if hasattr(mat, 'use_screen_refraction'):
            mat.use_screen_refraction = True
        bsdf = mat.node_tree.nodes.get("Principled BSDF")
        if bsdf:
            bsdf.inputs['Base Color'].default_value = diffuse
            bsdf.inputs['Alpha'].default_value = alpha
            bsdf.inputs['Roughness'].default_value = roughness
            if 'Transmission Weight' in bsdf.inputs:
                bsdf.inputs['Transmission Weight'].default_value = transmission
            elif 'Transmission' in bsdf.inputs:
                bsdf.inputs['Transmission'].default_value = transmission
            bsdf.inputs['IOR'].default_value = 1.45
        return mat

    def mat_fluido_luminoso(nombre, diffuse=(0.2, 0.7, 0.9, 1.0), emission=(0.1, 0.6, 0.85, 1.0), strength=0.6, alpha=0.85):
        mat = bpy.data.materials.get(nombre)
        if not mat:
            mat = bpy.data.materials.new(name=nombre)
        mat.use_nodes = True
        mat.blend_method = 'HASHED'
        bsdf = mat.node_tree.nodes.get("Principled BSDF")
        if bsdf:
            bsdf.inputs['Base Color'].default_value = diffuse
            bsdf.inputs['Alpha'].default_value = alpha
            bsdf.inputs['Roughness'].default_value = 0.04
            if 'Transmission Weight' in bsdf.inputs:
                bsdf.inputs['Transmission Weight'].default_value = 0.80
            elif 'Transmission' in bsdf.inputs:
                bsdf.inputs['Transmission'].default_value = 0.80
            bsdf.inputs['IOR'].default_value = 1.333
            if 'Emission Color' in bsdf.inputs:
                bsdf.inputs['Emission Color'].default_value = emission
                bsdf.inputs['Emission Strength'].default_value = strength
            elif 'Emission' in bsdf.inputs:
                bsdf.inputs['Emission'].default_value = emission
        return mat

    # Materiales aplicados
    mat_hdpe_trans = mat_translucido("HDPE_Totem", diffuse=(0.95, 0.97, 1.0, 1.0), alpha=0.35, roughness=0.25)
    mat_tanque_trans = mat_translucido("Tanque_Inox_Translucido", diffuse=(0.85, 0.90, 0.95, 1.0), alpha=0.40, roughness=0.15)
    mat_tubo_trans = mat_translucido("PVC_Translucido", diffuse=(0.90, 0.95, 1.0, 1.0), alpha=0.45, roughness=0.10)
    mat_vidrio_vaso = mat_translucido("Vidrio_Borosilicato", diffuse=(0.98, 1.0, 1.0, 1.0), alpha=0.25, roughness=0.02, transmission=0.98)

    # Fluidos con color y luminosidad distintiva
    mat_fluido_crudo = mat_fluido_luminoso("Agua_Pozo_Cruda", diffuse=(0.50, 0.42, 0.22, 1.0), emission=(0.55, 0.40, 0.15, 1.0), strength=0.5)
    mat_fluido_floc = mat_fluido_luminoso("Agua_Electrofloculada", diffuse=(0.20, 0.65, 0.70, 1.0), emission=(0.20, 0.75, 0.85, 1.0), strength=0.7)
    mat_fluido_clarificado = mat_fluido_luminoso("Agua_Clarificada", diffuse=(0.15, 0.60, 0.90, 1.0), emission=(0.15, 0.65, 0.95, 1.0), strength=0.6)
    mat_fluido_puro = mat_fluido_luminoso("Agua_UltraPura_Beber", diffuse=(0.40, 0.85, 1.0, 1.0), emission=(0.45, 0.90, 1.0, 1.0), strength=0.8)

    # Tuberías fluyendo
    mat_flujo_crudo = mat_fluido_luminoso("Flujo_Tuberia_Cruda", diffuse=(0.45, 0.42, 0.20, 1.0), emission=(0.50, 0.40, 0.15, 1.0), strength=0.7)
    mat_flujo_clarificado = mat_fluido_luminoso("Flujo_Tuberia_Clarificada", diffuse=(0.15, 0.60, 0.90, 1.0), emission=(0.20, 0.70, 0.98, 1.0), strength=0.7)
    mat_flujo_puro = mat_fluido_luminoso("Flujo_Tuberia_Pura", diffuse=(0.40, 0.85, 1.0, 1.0), emission=(0.50, 0.92, 1.0, 1.0), strength=0.85)

    # Reasignar materiales a los tanques y tuberías existentes
    for obj in bpy.data.objects:
        if obj.type == 'MESH':
            n = obj.name.lower()
            p = obj.parent.name.lower() if obj.parent else ""
            gp = obj.parent.parent.name.lower() if (obj.parent and obj.parent.parent) else ""
            full = f"{n} {p} {gp}"

            # Tanques IBC
            if "ibc" in full or "totem" in full:
                if "jaula" not in full:
                    obj.data.materials.clear()
                    obj.data.materials.append(mat_hdpe_trans)
            # Tanque cónico
            elif "conico" in full or "1bbl" in full:
                obj.data.materials.clear()
                obj.data.materials.append(mat_tanque_trans)
            # Barril presurizado
            elif "barril" in full:
                obj.data.materials.clear()
                obj.data.materials.append(mat_tanque_trans)
            # Vaso de cristal
            elif "vaso" in full:
                obj.data.materials.clear()
                obj.data.materials.append(mat_vidrio_vaso)
            # Tuberías existentes
            elif "conexion" in full or "casing" in full:
                obj.data.materials.clear()
                obj.data.materials.append(mat_tubo_trans)

    # Re-encuadre de tomas de cámara para máxima visibilidad cinematográfica
    cam = bpy.data.objects.get("Camara_AquaResiliencia")
    cam_target = bpy.data.objects.get("Camara_Objetivo")
    if cam and cam_target:
        cam.data.lens = 35.0  # Lente gran angular más amplia y cinematográfica
        cam.animation_data_clear()
        cam_target.animation_data_clear()

        tomas_optimizadas = [
            # 1. Pozo Somero (-7.50m) en subsuelo de Tijuana
            {"f": 1,   "cam": (1.9, -1.9, -7.0), "tgt": (0.0, 0.0, -7.5)},
            {"f": 45,  "cam": (1.7, -1.7, -4.5), "tgt": (0.0, 0.0, -4.0)},
            {"f": 75,  "cam": (1.5, -1.5, 0.0),  "tgt": (0.0, 0.0, 0.3)},
            # 2. Tubería hacia Tótem 2 Superior y llenado translúcido (+1.15m a +2.05m)
            {"f": 115, "cam": (4.2, -2.4, 2.4),  "tgt": (2.5, 0.0, 1.6)},
            {"f": 160, "cam": (3.2, -1.8, 1.9),  "tgt": (2.2, -0.3, 1.4)},
            # 3. Flujo en solenoide K3 y Caudalímetro
            {"f": 205, "cam": (1.4, -1.3, 1.55), "tgt": (0.52, -0.35, 1.25)},
            # 4. Electrofloculador en vidrio borosilicato efervescente
            {"f": 250, "cam": (0.3, -0.9, 1.35), "tgt": (0.52, -0.15, 1.15)},
            # 5. Tanque Cónico 1 BBL (plano medio amplio mostrando decantación)
            {"f": 320, "cam": (0.1, -1.7, 1.30), "tgt": (0.95, -0.15, 0.65)},
            # 6. Tubería de rebose y llenado de Tótem 1 Inferior clarificado
            {"f": 410, "cam": (3.6, 1.4, 1.3),   "tgt": (2.2, 0.0, 0.5)},
            # 7. Bomba Tri-Clamp presurizando hacia bastidor de Ósmosis Inversa
            {"f": 490, "cam": (0.7, -1.4, 1.1),  "tgt": (1.8, -0.8, 1.0)},
            # 8. Módulo RO y llenado de Barril Presurizado Sanke D (40 PSI)
            {"f": 590, "cam": (0.3, -2.5, 0.85), "tgt": (1.0, -1.6, 0.35)},
            # 9. Primer plano frontal al grifo y vaso de cristal llenándose
            {"f": 670, "cam": (1.65, -1.40, 0.60),"tgt": (1.22, -1.40, 0.42)},
            {"f": 715, "cam": (1.60, -1.40, 0.58),"tgt": (1.22, -1.40, 0.42)},
            # 10. Gran plano cenital panorámico de toda la planta en Tijuana
            {"f": 750, "cam": (4.2, -4.5, 3.4),  "tgt": (1.2, -0.5, 1.0)},
        ]

        for t in tomas_optimizadas:
            cam.location = t["cam"]
            cam.keyframe_insert(data_path="location", frame=t["f"])
            cam_target.location = t["tgt"]
            cam_target.keyframe_insert(data_path="location", frame=t["f"])

    bpy.ops.wm.save_as_mainfile(filepath=r"C:\Users\PEPITO\Downloads\123portijuana\aquaresiliencia_animacion.blend")
    print("¡Ajustes visuales y de cámara guardados con éxito en aquaresiliencia_animacion.blend!")

if __name__ == "__main__":
    aplicar_materiales_y_camaras()
