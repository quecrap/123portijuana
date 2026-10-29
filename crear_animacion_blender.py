"""
Animación Cinemática 'AquaResiliencia' Dual en Blender 5.2+
==========================================================
Sistema Integrado de Resiliencia Hídrica en Tijuana:
1. CAPTACIÓN DE AGUA DE LLUVIA (Temporada pluvial / Tormentas):
   - Vivienda con techo inclinado en Tijuana (X=-2.0, Y=4.5, Z=3.47m).
   - Simulación de lluvia cayendo del cielo sobre la cubierta.
   - Escorrentía recolectada por canaleta perimetral de techo.
   - Bajante con filtro separador de primeras aguas ("First Flush") vertical.
   - Llenado dinámico del Tótem IBC Pluvial (1,000 L) translúcido.
   - Tubería de interconexión con válvula pluvial hacia la planta.

2. CAPTACIÓN EN POZO SOMERO (Temporada seca / Estiaje / Acuífero Somero a -7.50m):
   - Boca de succión inferior abierta a -7.50m en estrato de grava del aluvión de Tijuana.
   - Columna de agua ascendente por ademe liso de 2".
   - Bomba de superficie y línea de impulsión superior.

3. TREN UNIFICADO DE POTABILIZACIÓN:
   - Tótem 2 / IBC Superior (+1.15m a +2.05m) para agua cruda.
   - Línea con solenoide K3 y caudalímetro digital P-08.
   - Electrofloculador en vidrio borosilicato con efervescencia de microburbujas.
   - Tanque Cónico 1 BBL (decantador con lodos al fondo y agua clarificada arriba).
   - Tótem 1 / IBC Inferior con agua clarificada azul celeste.
   - Bomba Tri-Clamp presurizando a bastidor de Ósmosis Inversa (70 PSI).
   - Barril Presurizado Sanke D en acero inoxidable (40 PSI).
   - Grifo dispensador sanitario y vaso de cristal llenándose de agua pura.
   - Gran plano panorámico cenital de toda la infraestructura dual en Tijuana.
"""

import bpy
import mathutils
import math
import os

def crear_animacion():
    # 1. Limpiar escena actual
    bpy.ops.wm.read_factory_settings(use_empty=True)
    scene = bpy.context.scene

    # Configuración de Render y Tiempo
    scene.render.fps = 30
    scene.frame_start = 1
    scene.frame_end = 820  # 27.3 segundos a 30 FPS
    scene.render.resolution_x = 1280
    scene.render.resolution_y = 720
    scene.render.resolution_percentage = 100

    # Configuración de salida a carpeta de fotogramas
    render_dir = r"C:\Users\PEPITO\Downloads\123portijuana\render_frames"
    os.makedirs(render_dir, exist_ok=True)
    scene.render.image_settings.file_format = 'JPEG'
    scene.render.image_settings.quality = 95
    scene.render.filepath = os.path.join(render_dir, "frame_")

    try:
        scene.render.engine = 'BLENDER_EEVEE_NEXT'
    except:
        scene.render.engine = 'BLENDER_EEVEE'

    if hasattr(scene, 'eevee'):
        scene.eevee.taa_render_samples = 32
        if hasattr(scene.eevee, 'use_raytracing'):
            scene.eevee.use_raytracing = True

    # Gestión de color cinematográfica
    scene.view_settings.view_transform = 'AgX' if 'AgX' in [c.name for c in bpy.types.ColorManagedViewSettings.bl_rna.properties['view_transform'].enum_items] else 'Filmic'
    scene.view_settings.look = 'High Contrast'

    # 2. Iluminación Ambiente y Luces de Escena
    world = bpy.data.worlds.new("AquaWorld")
    scene.world = world
    world.use_nodes = True
    bg_node = world.node_tree.nodes.get("Background")
    if bg_node:
        bg_node.inputs[0].default_value = (0.08, 0.12, 0.18, 1.0)
        bg_node.inputs[1].default_value = 1.4

    # Sol Principal
    bpy.ops.object.light_add(type='SUN', location=(6.0, -7.0, 12.0))
    sun = bpy.context.active_object
    sun.name = "Sol_Tijuana"
    sun.data.energy = 5.5
    sun.data.color = (1.0, 0.96, 0.91)
    sun.rotation_euler = (math.radians(48), math.radians(18), math.radians(-38))

    # Luz de Relleno General
    bpy.ops.object.light_add(type='AREA', location=(0.0, -4.0, 3.5))
    fill = bpy.context.active_object
    fill.name = "Luz_Relleno_Planta"
    fill.data.energy = 190.0
    fill.data.size = 6.0
    fill.data.color = (0.75, 0.88, 1.0)

    # Luz de Techo y Lluvia
    bpy.ops.object.light_add(type='AREA', location=(-2.0, 4.0, 7.0))
    luz_techo = bpy.context.active_object
    luz_techo.name = "Luz_Techo_Lluvia"
    luz_techo.data.energy = 240.0
    luz_techo.data.size = 5.0
    luz_techo.data.color = (0.80, 0.90, 1.0)

    # Luz de Acento sobre el vaso, grifo y barril
    bpy.ops.object.light_add(type='SPOT', location=(1.22, -1.35, 1.30))
    spot_vaso = bpy.context.active_object
    spot_vaso.name = "Spot_Vaso_Consumo"
    spot_vaso.data.energy = 220.0
    spot_vaso.data.spot_size = math.radians(50)
    spot_vaso.data.color = (1.0, 0.98, 0.94)
    spot_vaso.rotation_euler = (math.radians(10), 0, 0)

    # 3. Importar Modelo GLB más actual
    glb_path = r"C:\Users\PEPITO\Downloads\sistema binaryfile.glb"
    if os.path.exists(glb_path):
        print(f"Cargando GLB más actual: {glb_path}")
        bpy.ops.import_scene.gltf(filepath=glb_path)
    else:
        print("ADVERTENCIA: GLB no encontrado.")

    # 4. Funciones Creadoras de Materiales
    def mat_pbr(nombre, diffuse=(0.8, 0.8, 0.8, 1.0), metallic=0.0, roughness=0.3, transmission=0.0, ior=1.45, alpha=1.0, blend_method='OPAQUE', emission=(0,0,0,1), emission_strength=0.0):
        mat = bpy.data.materials.get(nombre)
        if not mat:
            mat = bpy.data.materials.new(name=nombre)
        mat.use_nodes = True
        mat.blend_method = blend_method
        if hasattr(mat, 'use_screen_refraction'):
            mat.use_screen_refraction = True
        bsdf = mat.node_tree.nodes.get("Principled BSDF")
        if bsdf:
            bsdf.inputs['Base Color'].default_value = diffuse
            bsdf.inputs['Metallic'].default_value = metallic
            bsdf.inputs['Roughness'].default_value = roughness
            bsdf.inputs['Alpha'].default_value = alpha
            if 'Transmission Weight' in bsdf.inputs:
                bsdf.inputs['Transmission Weight'].default_value = transmission
            elif 'Transmission' in bsdf.inputs:
                bsdf.inputs['Transmission'].default_value = transmission
            bsdf.inputs['IOR'].default_value = ior
            if 'Emission Color' in bsdf.inputs:
                bsdf.inputs['Emission Color'].default_value = emission
                bsdf.inputs['Emission Strength'].default_value = emission_strength
            elif 'Emission' in bsdf.inputs:
                bsdf.inputs['Emission'].default_value = emission
        return mat

    def mat_fluido_luminoso(nombre, diffuse=(0.2, 0.7, 0.9, 1.0), emission=(0.1, 0.6, 0.85, 1.0), strength=0.7, alpha=0.90):
        mat = bpy.data.materials.get(nombre)
        if not mat:
            mat = bpy.data.materials.new(name=nombre)
        mat.use_nodes = True
        mat.blend_method = 'BLEND'
        bsdf = mat.node_tree.nodes.get("Principled BSDF")
        if bsdf:
            bsdf.inputs['Base Color'].default_value = diffuse
            bsdf.inputs['Alpha'].default_value = alpha
            bsdf.inputs['Roughness'].default_value = 0.04
            if 'Transmission Weight' in bsdf.inputs:
                bsdf.inputs['Transmission Weight'].default_value = 0.85
            elif 'Transmission' in bsdf.inputs:
                bsdf.inputs['Transmission'].default_value = 0.85
            bsdf.inputs['IOR'].default_value = 1.333
            if 'Emission Color' in bsdf.inputs:
                bsdf.inputs['Emission Color'].default_value = emission
                bsdf.inputs['Emission Strength'].default_value = strength
            elif 'Emission' in bsdf.inputs:
                bsdf.inputs['Emission'].default_value = emission
        return mat

    # Materiales aplicados
    mat_inox = mat_pbr("Inox_Sanitario_304", diffuse=(0.88, 0.89, 0.91, 1.0), metallic=1.0, roughness=0.18)
    mat_vidrio = mat_pbr("Vidrio_Borosilicato", diffuse=(0.96, 0.99, 1.0, 1.0), roughness=0.02, transmission=0.98, alpha=0.25, blend_method='BLEND')
    mat_tanque_trans = mat_pbr("Tanque_Inox_Translucido", diffuse=(0.88, 0.92, 0.96, 1.0), metallic=0.3, roughness=0.15, transmission=0.75, alpha=0.40, blend_method='BLEND')
    mat_casing_pvc = mat_pbr("Casing_2pulg_Liso", diffuse=(0.86, 0.88, 0.88, 1.0), roughness=0.30)
    mat_casa_vidrio = mat_pbr("Casa_Vidrio_Arquitectonico", diffuse=(0.7, 0.8, 0.9, 0.2), roughness=0.15, transmission=0.85, ior=1.4, alpha=0.30, blend_method='BLEND')
    mat_muro_casa = mat_pbr("Muro_Vivienda", diffuse=(0.86, 0.88, 0.90, 1.0), roughness=0.45)
    mat_techo_casa = mat_pbr("Techo_Cubierta", diffuse=(0.28, 0.32, 0.36, 1.0), roughness=0.35)

    # Fluidos con color y luminosidad distintiva
    mat_agua_lluvia = mat_fluido_luminoso("Agua_Lluvia_Limpia", diffuse=(0.35, 0.75, 0.95, 1.0), emission=(0.30, 0.70, 1.0, 1.0), strength=0.70)
    mat_agua_cruda = mat_fluido_luminoso("Agua_Pozo_Cruda", diffuse=(0.52, 0.44, 0.22, 1.0), emission=(0.58, 0.42, 0.15, 1.0), strength=0.55)
    mat_agua_floc = mat_fluido_luminoso("Agua_Electrofloculada", diffuse=(0.20, 0.65, 0.70, 1.0), emission=(0.22, 0.75, 0.85, 1.0), strength=0.75)
    mat_agua_clarificada = mat_fluido_luminoso("Agua_Clarificada", diffuse=(0.15, 0.60, 0.90, 1.0), emission=(0.15, 0.65, 0.95, 1.0), strength=0.65)
    mat_agua_pura = mat_fluido_luminoso("Agua_UltraPura_Beber", diffuse=(0.40, 0.85, 1.0, 1.0), emission=(0.48, 0.90, 1.0, 1.0), strength=0.85)

    # Tuberías fluyendo
    mat_flujo_lluvia = mat_fluido_luminoso("Flujo_Tuberia_Lluvia", diffuse=(0.35, 0.78, 0.98, 1.0), emission=(0.35, 0.75, 1.0, 1.0), strength=0.80)
    mat_flujo_crudo = mat_fluido_luminoso("Flujo_Tuberia_Cruda", diffuse=(0.50, 0.42, 0.18, 1.0), emission=(0.55, 0.40, 0.15, 1.0), strength=0.75)
    mat_flujo_clarificado = mat_fluido_luminoso("Flujo_Tuberia_Clarificada", diffuse=(0.15, 0.60, 0.90, 1.0), emission=(0.20, 0.70, 0.98, 1.0), strength=0.75)
    mat_flujo_puro = mat_fluido_luminoso("Flujo_Tuberia_Pura", diffuse=(0.40, 0.85, 1.0, 1.0), emission=(0.50, 0.92, 1.0, 1.0), strength=0.90)

    # Material de lluvia (gotas cayendo)
    mat_gotas_lluvia = mat_pbr("G_Lluvia", diffuse=(0.7, 0.9, 1.0, 1.0), transmission=0.88, roughness=0.05, alpha=0.75, blend_method='BLEND', emission=(0.5, 0.85, 1.0, 1.0), emission_strength=0.85)

    # Configurar materiales directos del GLB (Gia_IBC, Gia_PVC, G_IBC_Translucido, etc.)
    for mat_ibc_name in ["Gia_IBC", "G_IBC_Translucido"]:
        m = bpy.data.materials.get(mat_ibc_name)
        if m:
            m.use_nodes = True
            m.blend_method = 'BLEND'
            b = m.node_tree.nodes.get("Principled BSDF")
            if b:
                b.inputs['Base Color'].default_value = (0.95, 0.98, 1.0, 1.0)
                b.inputs['Alpha'].default_value = 0.35
                b.inputs['Roughness'].default_value = 0.22
                if 'Transmission Weight' in b.inputs:
                    b.inputs['Transmission Weight'].default_value = 0.85
                elif 'Transmission' in b.inputs:
                    b.inputs['Transmission'].default_value = 0.85

    for mat_pvc_name in ["Gia_PVC", "G_PVC"]:
        m = bpy.data.materials.get(mat_pvc_name)
        if m:
            m.use_nodes = True
            m.blend_method = 'BLEND'
            b = m.node_tree.nodes.get("Principled BSDF")
            if b:
                b.inputs['Base Color'].default_value = (0.90, 0.94, 0.98, 1.0)
                b.inputs['Alpha'].default_value = 0.45
                b.inputs['Roughness'].default_value = 0.15
                if 'Transmission Weight' in b.inputs:
                    b.inputs['Transmission Weight'].default_value = 0.70
                elif 'Transmission' in b.inputs:
                    b.inputs['Transmission'].default_value = 0.70

    # Asignar materiales a mallas importadas
    for obj in bpy.data.objects:
        if obj.type == 'MESH':
            n = obj.name.lower()
            p = obj.parent.name.lower() if obj.parent else ""
            gp = obj.parent.parent.name.lower() if (obj.parent and obj.parent.parent) else ""
            full = f"{n} {p} {gp}"

            if "lluvia" in full and ("geom3d_.216" in full or "gota" in full):
                obj.data.materials.clear()
                obj.data.materials.append(mat_gotas_lluvia)
            elif "muro" in full or "geom3d.001" in full:
                obj.data.materials.clear()
                obj.data.materials.append(mat_muro_casa)
            elif "techo" in full or "geom3d.002" in full:
                obj.data.materials.clear()
                obj.data.materials.append(mat_techo_casa)
            elif "casa_captacion" in full:
                obj.data.materials.clear()
                obj.data.materials.append(mat_casa_vidrio)
            elif "vidrio" in full or "sight" in full or "mesh 93" in full:
                obj.data.materials.clear()
                obj.data.materials.append(mat_vidrio)
            elif "tanque_conico" in full or "1bbl" in full:
                obj.data.materials.clear()
                obj.data.materials.append(mat_tanque_trans)
            elif "barril" in full:
                obj.data.materials.clear()
                obj.data.materials.append(mat_tanque_trans)
            elif "vaso" in full:
                obj.data.materials.clear()
                obj.data.materials.append(mat_vidrio)

    # 5. POZO SOMERO REAL DE TIJUANA (-7.50m)
    bpy.ops.mesh.primitive_cylinder_add(radius=0.030, depth=7.50, location=(0.0, 0.0, -3.75))
    tubo_2_pulg = bpy.context.active_object
    tubo_2_pulg.name = "Tubo_Ademe_2pulg_Liso_Instalado"
    tubo_2_pulg.data.materials.append(mat_casing_pvc)

    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0.0, 0.0, -5.25))
    estrato_acuifero = bpy.context.active_object
    estrato_acuifero.name = "Estrato_Grava_Acuifero_Tijuana"
    estrato_acuifero.scale = (3.5, 3.5, 4.5)
    mat_grava = mat_pbr("Grava_Acuifera", diffuse=(0.22, 0.25, 0.28, 1.0), roughness=0.9)
    estrato_acuifero.data.materials.append(mat_grava)
    estrato_acuifero.display_type = 'WIRE'

    bpy.ops.mesh.primitive_torus_add(major_radius=0.032, minor_radius=0.005, location=(0.0, 0.0, -7.50))
    boca_abierta = bpy.context.active_object
    boca_abierta.name = "Boca_Succion_Inferior_Abierta"
    boca_abierta.data.materials.append(mat_casing_pvc)

    bpy.ops.mesh.primitive_cone_add(radius1=0.065, radius2=0.022, depth=0.18, location=(0.0, 0.0, -7.60))
    cono_flujo_in = bpy.context.active_object
    cono_flujo_in.name = "Flujo_Succion_Inferior"
    cono_flujo_in.data.materials.append(mat_agua_cruda)

    bpy.ops.mesh.primitive_cylinder_add(radius=0.025, depth=1.0, location=(0.0, 0.0, -7.0))
    columna_agua = bpy.context.active_object
    columna_agua.name = "Columna_Agua_Ascendente"
    columna_agua.data.materials.append(mat_flujo_crudo)

    # 6. FUNCIÓN AUXILIAR PARA TRAMOS DE AGUA FLUYENDO EN TUBERÍAS
    def crear_tubo_flujo(nombre, p1, p2, radio=0.016, material=mat_flujo_crudo, f_start=1, f_end=30):
        v1 = mathutils.Vector(p1)
        v2 = mathutils.Vector(p2)
        dir_vec = v2 - v1
        length = dir_vec.length
        if length < 0.001:
            return None
        mid = (v1 + v2) / 2.0

        bpy.ops.mesh.primitive_cylinder_add(radius=radio, depth=1.0, location=(0, 0, 0))
        tubo = bpy.context.active_object
        tubo.name = nombre
        tubo.data.materials.append(material)

        rot_quat = mathutils.Vector((0, 0, 1)).rotation_difference(dir_vec.normalized())
        tubo.rotation_mode = 'QUATERNION'
        tubo.rotation_quaternion = rot_quat

        tubo.location = v1
        tubo.scale = (1.0, 1.0, 0.001)
        tubo.keyframe_insert(data_path="location", frame=f_start)
        tubo.keyframe_insert(data_path="scale", frame=f_start)

        tubo.location = mid
        tubo.scale = (1.0, 1.0, length)
        tubo.keyframe_insert(data_path="location", frame=f_end)
        tubo.keyframe_insert(data_path="scale", frame=f_end)

        return tubo

    # 7. FUNCIÓN AUXILIAR PARA VOLÚMENES DE AGUA LLENANDO TANQUES
    def crear_volumen_llenado(nombre, pos_base, dim_xy, altura_max, material, f_start, f_end, tipo='CUBE', radio=0.2):
        pos_b = mathutils.Vector(pos_base)
        pos_lleno = pos_b + mathutils.Vector((0, 0, altura_max / 2.0))

        if tipo == 'CUBE':
            bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0))
            vol = bpy.context.active_object
            dx, dy = dim_xy
            scale_init = (dx, dy, 0.001)
            scale_full = (dx, dy, altura_max)
        else: # CYLINDER
            bpy.ops.mesh.primitive_cylinder_add(radius=radio, depth=1.0, location=(0, 0, 0))
            vol = bpy.context.active_object
            scale_init = (1.0, 1.0, 0.001)
            scale_full = (1.0, 1.0, altura_max)

        vol.name = nombre
        vol.data.materials.append(material)

        vol.location = pos_b
        vol.scale = scale_init
        vol.keyframe_insert(data_path="location", frame=f_start)
        vol.keyframe_insert(data_path="scale", frame=f_start)

        vol.location = pos_lleno
        vol.scale = scale_full
        vol.keyframe_insert(data_path="location", frame=f_end)
        vol.keyframe_insert(data_path="scale", frame=f_end)

        return vol

    # -------------------------------------------------------------
    # 8. SISTEMA DE CAPTACIÓN DE AGUA DE LLUVIA (ANIMACIÓN)
    # -------------------------------------------------------------
    # A) Gotas de lluvia cayendo del cielo sobre el techo
    lluvia_mesh = bpy.data.objects.get("Geom3D_.216")
    if lluvia_mesh:
        lluvia_mesh.scale = (1.0, 1.0, 1.0)
        # Animar caída de lluvia en bucle de frames 1 a 140
        for f in range(1, 140, 20):
            lluvia_mesh.location.z = 0.0
            lluvia_mesh.keyframe_insert(data_path="location", frame=f)
            lluvia_mesh.location.z = -1.8
            lluvia_mesh.keyframe_insert(data_path="location", frame=f + 19)
        # Ocultar lluvia después de frame 150 (transición al pozo y verano)
        lluvia_mesh.scale = (1.0, 1.0, 1.0)
        lluvia_mesh.keyframe_insert(data_path="scale", frame=140)
        lluvia_mesh.scale = (0.001, 0.001, 0.001)
        lluvia_mesh.keyframe_insert(data_path="scale", frame=150)

    # B) Agua en la canaleta del techo fluyendo hacia el bajante
    crear_tubo_flujo("Flujo_Canaleta_Techo", (-3.8, 1.80, 2.98), (-1.0, 1.80, 2.98), radio=0.035, material=mat_flujo_lluvia, f_start=15, f_end=45)

    # C) Agua bajando por el filtro First Flush
    crear_tubo_flujo("Flujo_Bajante_FirstFlush", (-1.0, 1.80, 2.98), (-1.0, 1.80, 1.15), radio=0.030, material=mat_flujo_lluvia, f_start=35, f_end=65)

    # D) Llenado del Tótem IBC Pluvial (1,000 Litros)
    crear_volumen_llenado(
        "Agua_IBC_Lluvia_Volumen",
        pos_base=(-1.000, 1.100, 0.15),
        dim_xy=(0.92, 1.12),
        altura_max=0.90,
        material=mat_agua_lluvia,
        f_start=45,
        f_end=130,
        tipo='CUBE'
    )

    # E) Tubería pluvial de salida hacia la planta potabilizadora
    crear_tubo_flujo("Flujo_Tuberia_Pluvial_Salida", (-1.0, 0.46, 0.22), (-0.05, 0.20, 0.18), radio=0.020, material=mat_flujo_lluvia, f_start=90, f_end=135)

    # -------------------------------------------------------------
    # 9. CONSTRUCCIÓN DE LOS VOLÚMENES DE AGUA EN EL TREN DE TRATAMIENTO
    # -------------------------------------------------------------
    # A) Tótem 2 / IBC Superior (+1.15m a +2.05m): Se llena entre frames 250 y 330
    crear_volumen_llenado(
        "Agua_IBC_Superior_Volumen",
        pos_base=(2.539, 0.0, 1.15),
        dim_xy=(0.95, 0.98),
        altura_max=0.88,
        material=mat_agua_cruda,
        f_start=250,
        f_end=330,
        tipo='CUBE'
    )

    # B) Electrofloculador en Vidrio (+0.70m a +1.45m): Se llena y reacciona entre frames 345 y 390
    crear_volumen_llenado(
        "Agua_Electrofloculador_Volumen",
        pos_base=(0.515, -0.157, 0.70),
        dim_xy=(0, 0),
        altura_max=0.75,
        material=mat_agua_floc,
        f_start=345,
        f_end=390,
        tipo='CYLINDER',
        radio=0.062
    )

    # C) Tanque Cónico 1 BBL (+0.22m a +1.25m): Se llena y sedimenta entre frames 400 y 480
    bpy.ops.mesh.primitive_cone_add(radius1=0.23, radius2=0.04, depth=0.30, location=(0.950, -0.150, 0.37))
    cono_agua = bpy.context.active_object
    cono_agua.name = "Agua_Conico_Fondo"
    cono_agua.data.materials.append(mat_agua_cruda)
    cono_agua.scale = (0.001, 0.001, 0.001)
    cono_agua.keyframe_insert(data_path="scale", frame=400)
    cono_agua.scale = (1.0, 1.0, 1.0)
    cono_agua.keyframe_insert(data_path="scale", frame=430)

    crear_volumen_llenado(
        "Agua_Conico_Cuerpo_Clarificado",
        pos_base=(0.950, -0.150, 0.52),
        dim_xy=(0, 0),
        altura_max=0.70,
        material=mat_agua_clarificada,
        f_start=430,
        f_end=480,
        tipo='CYLINDER',
        radio=0.235
    )

    # D) Tótem 1 / IBC Inferior (+0.10m a +0.88m): Se llena entre frames 495 y 560
    crear_volumen_llenado(
        "Agua_IBC_Inferior_Volumen",
        pos_base=(2.539, 0.0, 0.10),
        dim_xy=(0.95, 0.98),
        altura_max=0.78,
        material=mat_agua_clarificada,
        f_start=495,
        f_end=560,
        tipo='CUBE'
    )

    # E) Barril Presurizado Sanke D Inox (+0.05m a +0.55m a 40 PSI): Se llena entre frames 610 y 675
    crear_volumen_llenado(
        "Agua_Barril_Sanke_Volumen",
        pos_base=(1.000, -1.600, 0.06),
        dim_xy=(0, 0),
        altura_max=0.48,
        material=mat_agua_pura,
        f_start=610,
        f_end=675,
        tipo='CYLINDER',
        radio=0.185
    )

    # -------------------------------------------------------------
    # 10. TUBERÍAS DE LA PLANTA CON AGUA FLUYENDO DINÁMICAMENTE
    # -------------------------------------------------------------
    crear_tubo_flujo("Flujo_T1_Pozo_Vertical", (0.0, 0.0, 0.28), (0.0, 0.0, 2.38), radio=0.018, material=mat_flujo_crudo, f_start=225, f_end=245)
    crear_tubo_flujo("Flujo_T1_Pozo_Horizontal", (0.0, 0.0, 2.38), (2.54, 0.0, 2.38), radio=0.018, material=mat_flujo_crudo, f_start=240, f_end=260)
    crear_tubo_flujo("Flujo_T1_Descarga_IBCSup", (2.54, 0.0, 2.38), (2.54, 0.0, 2.05), radio=0.018, material=mat_flujo_crudo, f_start=255, f_end=270)

    crear_tubo_flujo("Flujo_T2_IBCSup_Salida", (2.54, 0.0, 1.15), (2.54, -0.46, 1.15), radio=0.016, material=mat_flujo_crudo, f_start=320, f_end=338)
    crear_tubo_flujo("Flujo_T2_Linea_Caudalimetro", (2.54, -0.46, 1.25), (0.52, -0.46, 1.25), radio=0.016, material=mat_flujo_crudo, f_start=335, f_end=360)
    crear_tubo_flujo("Flujo_T2_Entrada_Electro", (0.52, -0.46, 1.25), (0.52, -0.16, 0.70), radio=0.016, material=mat_flujo_crudo, f_start=355, f_end=375)

    crear_tubo_flujo("Flujo_T3_Electro_A_Conico", (0.52, -0.16, 1.40), (0.95, -0.16, 1.20), radio=0.016, material=mat_flujo_crudo, f_start=380, f_end=405)

    crear_tubo_flujo("Flujo_T4_Conico_A_Codo", (0.95, -0.16, 1.14), (1.26, 0.0, 1.14), radio=0.016, material=mat_flujo_clarificado, f_start=465, f_end=485)
    crear_tubo_flujo("Flujo_T4_Codo_A_IBCInf", (1.26, 0.0, 1.14), (2.54, 0.0, 0.95), radio=0.016, material=mat_flujo_clarificado, f_start=480, f_end=500)

    crear_tubo_flujo("Flujo_T5_IBCInf_A_Bomba", (2.54, 0.0, 0.15), (1.96, 0.0, 0.20), radio=0.016, material=mat_flujo_clarificado, f_start=550, f_end=570)
    crear_tubo_flujo("Flujo_T5_Bomba_A_BastidorRO", (1.96, 0.0, 0.25), (1.80, -0.78, 0.40), radio=0.014, material=mat_flujo_clarificado, f_start=565, f_end=590)
    crear_tubo_flujo("Flujo_T5_Membrana_4040", (1.80, -0.78, 0.40), (1.80, -0.78, 1.40), radio=0.014, material=mat_flujo_puro, f_start=585, f_end=615)

    crear_tubo_flujo("Flujo_T6_RO_A_Codo", (1.80, -0.78, 0.80), (1.00, -0.78, 0.45), radio=0.012, material=mat_flujo_puro, f_start=610, f_end=635)
    crear_tubo_flujo("Flujo_T6_Codo_A_BarrilSanke", (1.00, -0.78, 0.45), (1.00, -1.60, 0.55), radio=0.012, material=mat_flujo_puro, f_start=630, f_end=655)

    crear_tubo_flujo("Flujo_T7_Barril_A_GrifoBase", (1.00, -1.60, 0.55), (1.22, -1.55, 0.32), radio=0.010, material=mat_flujo_puro, f_start=670, f_end=690)
    crear_tubo_flujo("Flujo_T7_Grifo_Ascendente", (1.22, -1.55, 0.32), (1.22, -1.55, 0.88), radio=0.010, material=mat_flujo_puro, f_start=685, f_end=702)

    # -------------------------------------------------------------
    # 11. DISPENSADOR, GRIFO INOX Y VASO DE CRISTAL
    # -------------------------------------------------------------
    bpy.ops.mesh.primitive_cylinder_add(radius=0.015, depth=0.65, location=(1.22, -1.55, 0.32))
    soporte_grifo = bpy.context.active_object
    soporte_grifo.name = "Soporte_Inox_Grifo"
    soporte_grifo.data.materials.append(mat_inox)

    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(1.22, -1.40, 0.35))
    drip_tray = bpy.context.active_object
    drip_tray.name = "Bandeja_Goteo_Inox"
    drip_tray.scale = (0.16, 0.16, 0.02)
    drip_tray.data.materials.append(mat_inox)

    bpy.ops.mesh.primitive_cylinder_add(radius=0.008, depth=0.25, location=(1.22, -1.55, 0.76))
    grifo_vert = bpy.context.active_object
    grifo_vert.name = "Grifo_CuelloGanso"
    grifo_vert.data.materials.append(mat_inox)

    bpy.ops.mesh.primitive_cylinder_add(radius=0.007, depth=0.15, location=(1.22, -1.48, 0.88))
    grifo_horiz = bpy.context.active_object
    grifo_horiz.name = "Grifo_Pico"
    grifo_horiz.rotation_euler = (math.radians(90), 0, 0)
    grifo_horiz.data.materials.append(mat_inox)

    bpy.ops.mesh.primitive_cylinder_add(radius=0.038, depth=0.12, location=(1.22, -1.40, 0.42))
    vaso = bpy.context.active_object
    vaso.name = "Vaso_Cristal_Agua"
    vaso.data.materials.append(mat_vidrio)

    bpy.ops.mesh.primitive_cylinder_add(radius=0.036, depth=0.10, location=(1.22, -1.40, 0.41))
    agua_vaso = bpy.context.active_object
    agua_vaso.name = "Agua_Vaso_Consumo"
    agua_vaso.data.materials.append(mat_agua_pura)

    bpy.ops.mesh.primitive_cylinder_add(radius=0.004, depth=0.35, location=(1.22, -1.40, 0.68))
    chorro = bpy.context.active_object
    chorro.name = "Chorro_Agua_Pura"
    chorro.data.materials.append(mat_agua_pura)

    # -------------------------------------------------------------
    # 12. ANIMACIÓN DE AGUA EN POZO Y EN EL VASO
    # -------------------------------------------------------------
    # Sube el agua por el pozo (frames 185 a 235)
    columna_agua.scale = (1.0, 1.0, 0.001)
    columna_agua.location = (0.0, 0.0, -7.5)
    columna_agua.keyframe_insert(data_path="scale", frame=185)
    columna_agua.keyframe_insert(data_path="location", frame=185)

    columna_agua.scale = (1.0, 1.0, 7.6)
    columna_agua.location = (0.0, 0.0, -3.7)
    columna_agua.keyframe_insert(data_path="scale", frame=235)
    columna_agua.keyframe_insert(data_path="location", frame=235)

    # Chorro del grifo (frames 700 a 760)
    chorro.scale = (0.001, 0.001, 0.001)
    chorro.keyframe_insert(data_path="scale", frame=700)
    chorro.scale = (1.0, 1.0, 1.0)
    chorro.keyframe_insert(data_path="scale", frame=708)
    chorro.keyframe_insert(data_path="scale", frame=760)
    chorro.scale = (0.001, 0.001, 0.001)
    chorro.keyframe_insert(data_path="scale", frame=768)

    # Vaso llenándose de agua pura (frames 708 a 760)
    agua_vaso.scale = (1.0, 1.0, 0.001)
    agua_vaso.location = (1.22, -1.40, 0.36)
    agua_vaso.keyframe_insert(data_path="scale", frame=708)
    agua_vaso.keyframe_insert(data_path="location", frame=708)

    agua_vaso.scale = (1.0, 1.0, 1.0)
    agua_vaso.location = (1.22, -1.40, 0.41)
    agua_vaso.keyframe_insert(data_path="scale", frame=760)
    agua_vaso.keyframe_insert(data_path="location", frame=760)

    # -------------------------------------------------------------
    # 13. CÁMARA CINEMÁTICA Y TRAYECTORIA MULTI-PLANO DUAL
    # -------------------------------------------------------------
    bpy.ops.object.empty_add(type='SPHERE', radius=0.15, location=(0.0, 0.0, 0.0))
    cam_target = bpy.context.active_object
    cam_target.name = "Camara_Objetivo"

    bpy.ops.object.camera_add(location=(-2.8, -0.8, 4.2))
    cam = bpy.context.active_object
    cam.name = "Camara_AquaResiliencia"
    cam.data.lens = 35.0
    cam.data.dof.use_dof = False
    scene.camera = cam

    track = cam.constraints.new(type='TRACK_TO')
    track.target = cam_target
    track.track_axis = 'TRACK_NEGATIVE_Z'
    track.up_axis = 'UP_Y'

    tomas = [
        # 1. Tormenta pluvial sobre el techo de la casa y canaleta (Captación de Lluvia)
        {"f": 1,   "cam": (-2.8, -0.8, 4.2),  "tgt": (-2.0, 3.2, 2.8)},
        {"f": 40,  "cam": (-2.6, 0.2, 3.5),   "tgt": (-1.5, 2.2, 2.5)},
        # 2. Bajante, filtro First Flush y llenado de Tótem IBC de Lluvia
        {"f": 75,  "cam": (-2.4, -0.4, 2.0),  "tgt": (-1.0, 1.1, 0.85)},
        {"f": 120, "cam": (-2.1, -0.2, 1.5),  "tgt": (-0.8, 0.8, 0.60)},
        # 3. Transición: Válvula pluvial hacia planta y descenso al Pozo Somero (-7.50m)
        {"f": 150, "cam": (-0.5, -1.2, 0.8),  "tgt": (0.0, 0.2, 0.10)},
        {"f": 185, "cam": (1.9, -1.9, -7.0),  "tgt": (0.0, 0.0, -7.5)},
        # 4. Agua ascendiendo por el pozo somero en Tijuana (-7.50m a superficie)
        {"f": 215, "cam": (1.7, -1.7, -4.5),  "tgt": (0.0, 0.0, -4.0)},
        {"f": 240, "cam": (1.5, -1.5, 0.0),   "tgt": (0.0, 0.0, 0.3)},
        # 5. Tubería hacia Tótem 2 Superior y llenado (+1.15m a +2.05m)
        {"f": 280, "cam": (4.2, -2.4, 2.4),   "tgt": (2.5, 0.0, 1.6)},
        {"f": 325, "cam": (3.2, -1.8, 1.9),   "tgt": (2.2, -0.3, 1.4)},
        # 6. Flujo en solenoide K3, Caudalímetro y Electrofloculador en vidrio
        {"f": 355, "cam": (1.4, -1.3, 1.55),  "tgt": (0.52, -0.35, 1.25)},
        {"f": 385, "cam": (0.3, -0.9, 1.35),  "tgt": (0.52, -0.15, 1.15)},
        # 7. Tanque Cónico 1 BBL (decantación y sedimentación)
        {"f": 440, "cam": (0.1, -1.7, 1.30),  "tgt": (0.95, -0.15, 0.65)},
        # 8. Tubería de rebose y llenado de Tótem 1 Inferior clarificado
        {"f": 525, "cam": (3.6, 1.4, 1.3),    "tgt": (2.2, 0.0, 0.5)},
        # 9. Bomba Tri-Clamp presurizando hacia bastidor de Ósmosis Inversa (70 PSI)
        {"f": 585, "cam": (0.7, -1.4, 1.1),   "tgt": (1.8, -0.8, 1.0)},
        # 10. Módulo RO y llenado de Barril Presurizado Sanke D (40 PSI)
        {"f": 645, "cam": (0.3, -2.5, 0.85),  "tgt": (1.0, -1.6, 0.35)},
        # 11. Primer plano frontal al grifo y vaso de cristal llenándose de agua pura
        {"f": 710, "cam": (1.80, -1.22, 0.68), "tgt": (1.20, -1.42, 0.48)},
        {"f": 750, "cam": (1.75, -1.24, 0.66), "tgt": (1.20, -1.42, 0.48)},
        # 12. Gran plano cenital panorámico dual (Lluvia + Pozo Somero + Planta Potabilizadora)
        {"f": 820, "cam": (3.8, -5.2, 3.8),   "tgt": (0.5, 0.8, 1.2)},
    ]

    for t in tomas:
        cam.location = t["cam"]
        cam.keyframe_insert(data_path="location", frame=t["f"])
        cam_target.location = t["tgt"]
        cam_target.keyframe_insert(data_path="location", frame=t["f"])

    # 14. Guardar archivo .blend
    blend_path = r"C:\Users\PEPITO\Downloads\123portijuana\aquaresiliencia_animacion.blend"
    bpy.ops.wm.save_as_mainfile(filepath=blend_path)
    print(f"\n¡Éxito total! Archivo Blend guardado en: {blend_path}")

if __name__ == "__main__":
    crear_animacion()
