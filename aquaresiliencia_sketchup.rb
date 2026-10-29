# ==============================================================================
# SCRIPT MAESTRO DE ACTUALIZACIÓN Y MEJORA DE DIAGRAMA - AQUARESILIENCIA v2.2
# Compatible con SketchUp 2021, 2022, 2023, 2024, 2026.2+
# Sistema de 2 BOMBAS EXACTAS:
#   Bomba 1: Pozo Somero (-7.5m) -> Tótem 2 Superior
#   Bomba 2: Impulsora Evans 24V (Tótem 1 de abajo) -> Pergolado Solar -> Manifold
# Manifold Rediseñado: 80% Doméstico (Válvula Azul) / 20% RO (85-90 PSI)
# Recirculación 100% de Salmuera de RO hacia Tótem 2 Superior (Cero Desperdicio)
# ==============================================================================

model = Sketchup.active_model

# Abortar operaciones previas colgadas si existen
begin
  model.abort_operation
rescue
end

model.start_operation("Actualizar Diagrama AquaResiliencia 2 Bombas y Manifold", true)

begin
  entities = model.active_entities
  mats     = model.materials
  layers   = model.layers

  # --- 1. LIMPIEZA AUTOMÁTICA DE EJECUCIONES PREVIAS DEL SCRIPT ---
  entities.grep(Sketchup::Group).each do |g|
    if g.valid? && g.name =~ /Actualizacion_Celdas|Diagrama_Mejorado_AquaResiliencia/
      g.erase!
    end
  end

  # --- 2. CREACIÓN DE CAPAS (TAGS) ORGANIZADAS ---
  layer_galv  = layers["01_Etapa1_Galvanica_ABS_3m"]     || layers.add("01_Etapa1_Galvanica_ABS_3m")
  layer_perg  = layers["02_Etapa2_Pergola_Solar_Techo"]  || layers.add("02_Etapa2_Pergola_Solar_Techo")
  layer_elec  = layers["03_Energia_Solar_y_EVE_POS"]     || layers.add("03_Energia_Solar_y_EVE_POS")
  layer_hidr  = layers["04_Hidraulica_y_Manifold_90PSI"] || layers.add("04_Hidraulica_y_Manifold_90PSI")
  layer_notes = layers["05_Etiquetas_Tecnicas_3D"]       || layers.add("05_Etiquetas_Tecnicas_3D")

  # --- 3. MATERIALES NATIVOS ROBUSTOS ---
  def get_mat(mats, name, r, g, b, alpha = 1.0)
    m = mats[name]
    unless m
      m = mats.add(name)
      m.color = Sketchup::Color.new(r, g, b)
    end
    m.alpha = alpha if alpha < 1.0 && m.alpha == 1.0
    m
  end

  mat_abs_vis   = get_mat(mats, "Gia_ABS_Corte_Translucido", 30, 35, 40, 0.45)
  mat_abs_solid = get_mat(mats, "G_NegroComp", 35, 40, 45)
  mat_inox      = get_mat(mats, "Gia_Inox", 210, 215, 220)
  mat_zinc      = get_mat(mats, "Gia_Electrodo", 185, 195, 205)
  mat_cobre     = get_mat(mats, "Gia_Cobre_Catodo", 184, 115, 51)
  mat_galv      = get_mat(mats, "Gia_Jaula_Galv", 170, 175, 180)
  mat_laton     = get_mat(mats, "Gia_Laton", 205, 170, 60)
  mat_manguera  = get_mat(mats, "Gia_Manguera", 25, 25, 25)
  mat_roja      = get_mat(mats, "G_MangueraRoja", 215, 45, 45)
  mat_azul      = get_mat(mats, "G_MangueraAzul", 30, 110, 210)
  mat_salmuera  = get_mat(mats, "G_MangueraSalmuera", 230, 160, 40)
  mat_agua      = get_mat(mats, "G_FlujoAzulLluvia", 60, 160, 240, 0.35)
  mat_panel     = get_mat(mats, "Gia_Rejilla_Panel", 20, 40, 75)
  mat_bat       = get_mat(mats, "G_PCB_Azul", 25, 90, 170)
  mat_valv_roja = get_mat(mats, "G_RojoValvula", 220, 30, 30)
  mat_valv_azul = get_mat(mats, "Gia_Valvula_Azul", 30, 80, 200)
  mat_bomba     = get_mat(mats, "Gia_Bomba_Diafragma", 50, 55, 60)
  mat_cabeza    = get_mat(mats, "Gia_Cabeza_Bomba", 20, 20, 20)
  mat_gabinete  = get_mat(mats, "G_GabineteIoT", 220, 220, 220)
  mat_cable_pos = get_mat(mats, "G_CablePotencia", 220, 30, 30)
  mat_cable_neg = get_mat(mats, "G_Cable", 20, 20, 20)
  mat_cable_sig = get_mat(mats, "G_CableSenal", 240, 180, 20)
  mat_filtro_az = get_mat(mats, "G_VasoFiltroAzul", 20, 70, 170, 0.70)
  mat_filtro_wh = get_mat(mats, "Gia_Filtro_Blanco", 245, 245, 250)

  # --- 4. FUNCIONES GEOMÉTRICAS CORREGIDAS Y PROTEGIDAS ---

  def add_solid_box(parent_ents, x, y, z, dx, dy, dz, mat, name = "", layer = nil)
    grp = parent_ents.add_group
    grp.name = name unless name.empty?
    grp.layer = layer if layer
    pts = [
      Geom::Point3d.new(x, y, z),
      Geom::Point3d.new(x + dx, y, z),
      Geom::Point3d.new(x + dx, y + dy, z),
      Geom::Point3d.new(x, y + dy, z)
    ]
    face = grp.entities.add_face(pts)
    if face && face.valid?
      face.reverse! if face.normal.z < 0
      face.pushpull(dz)
    end
    grp.material = mat if mat && grp.valid?
    grp
  end

  def add_cylinder_tube(parent_ents, cx, cy, cz, od, height, mat, id = 0.mm, name = "", layer = nil, segs = 24)
    grp = parent_ents.add_group
    grp.name = name unless name.empty?
    grp.layer = layer if layer
    ents = grp.entities
    center = Geom::Point3d.new(cx, cy, cz)
    up = Geom::Vector3d.new(0, 0, 1)

    if id > 0.mm && id < od
      c_out = ents.add_circle(center, up, od / 2.0, segs)
      f_base = ents.add_face(c_out)
      if f_base && f_base.valid?
        ents.add_circle(center, up, id / 2.0, segs)
        ents.grep(Sketchup::Face).each do |fc|
          if fc.valid? && fc.classify_point(center) != Sketchup::Face::PointOutside
            fc.erase!
          end
        end
        annular = ents.grep(Sketchup::Face).first
        if annular && annular.valid?
          annular.reverse! if annular.normal.z < 0
          annular.pushpull(height)
        end
      end
    else
      c_out = ents.add_circle(center, up, od / 2.0, segs)
      f_out = ents.add_face(c_out)
      if f_out && f_out.valid?
        f_out.reverse! if f_out.normal.z < 0
        f_out.pushpull(height)
      end
    end

    grp.material = mat if mat && grp.valid?
    grp
  end

  def add_pipe_along_path(parent_ents, points_array, od, mat, name = "", layer = nil, segs = 16)
    return nil if points_array.nil? || points_array.length < 2
    
    clean_pts = [points_array.first]
    points_array[1..-1].each do |pt|
      clean_pts << pt if pt.distance(clean_pts.last) > 1.mm
    end
    return nil if clean_pts.length < 2

    grp = parent_ents.add_group
    grp.name = name unless name.empty?
    grp.layer = layer if layer
    ents = grp.entities

    path_edges = ents.add_edges(clean_pts)
    if path_edges.nil? || path_edges.empty?
      grp.erase! if grp.valid?
      return nil
    end

    start_pt = clean_pts[0]
    dir_vec  = clean_pts[1] - start_pt
    if dir_vec.length < 0.1.mm
      grp.erase! if grp.valid?
      return nil
    end

    circle = ents.add_circle(start_pt, dir_vec, od / 2.0, segs)
    face   = ents.add_face(circle)

    if face && face.valid?
      face.followme(path_edges)
    end
    grp.material = mat if mat && grp.valid?
    grp
  end

  def generate_u_bend_points_xz(cx1, cx2, y_const, z_top, z_bottom_center, num_arc_segs = 12)
    r = ((cx2 - cx1).abs) / 2.0
    mid_x = (cx1 + cx2) / 2.0
    pts = []
    pts << Geom::Point3d.new(cx1, y_const, z_top)
    pts << Geom::Point3d.new(cx1, y_const, z_bottom_center)
    (1...num_arc_segs).each do |i|
      angle = Math::PI + (Math::PI * i.to_f / num_arc_segs.to_f)
      px = mid_x + (r * Math.cos(angle))
      pz = z_bottom_center + (r * Math.sin(angle))
      pts << Geom::Point3d.new(px, y_const, pz)
    end
    pts << Geom::Point3d.new(cx2, y_const, z_bottom_center)
    pts << Geom::Point3d.new(cx2, y_const, z_top)
    pts
  end

  # ==============================================================================
  # 5. DETECCIÓN DE ELEMENTOS EXISTENTES EN EL MODELO SKETCHUP
  # ==============================================================================
  # REGLA FUNDAMENTAL: En todo el sistema AquaResiliencia existen EXACTAMENTE 2 BOMBAS:
  #   - BOMBA 1: Pozo Somero (-7.5m -> Tótem 2 Superior) [Capa '1_Pozo_Y_Bomba']
  #   - BOMBA 2: Bomba Evans 24V al pie del Tótem 1 de abajo [Capa '12_Bomba_TriClamp']
  #     Succiona del Tótem 1 (pichancha media altura) e IMPULSA AL PERGOLADO SOLAR.
  #   - NO SE CREA NINGUNA TERCERA BOMBA NI PULMONES FALSOS.

  existing_t1 = entities.grep(Sketchup::Group).find { |g| g.valid? && g.name =~ /2_IBC_Inferior|IBC_Inferior|Totem.*1|Totem.*Abajo/i }
  existing_t2 = entities.grep(Sketchup::Group).find { |g| g.valid? && g.name =~ /3_IBC_Superior|IBC_Superior|Totem.*2|Totem.*Arriba/i }
  existing_p2 = entities.grep(Sketchup::Group).find { |g| g.valid? && g.name =~ /12_Bomba_TriClamp|Bomba.*TriClamp|Bomba.*Evans|Bomba.*Totem/i }
  existing_rack = entities.grep(Sketchup::Group).find { |g| g.valid? && g.name =~ /8_Modulo_Osmosis_Inversa|Modulo.*RO|Rack/i }

  # Coordenadas maestras nativas de 'aquaresilencia 1.skp'
  t1_cx = existing_t1 ? (existing_t1.bounds.min.x + existing_t1.bounds.max.x)/2.0 : 2539.mm
  t1_cy = existing_t1 ? (existing_t1.bounds.min.y + existing_t1.bounds.max.y)/2.0 : 0.mm
  t1_cz = existing_t1 ? existing_t1.bounds.min.z : 0.mm

  t2_cx = existing_t2 ? (existing_t2.bounds.min.x + existing_t2.bounds.max.x)/2.0 : 2539.mm
  t2_cy = existing_t2 ? (existing_t2.bounds.min.y + existing_t2.bounds.max.y)/2.0 : 0.mm
  t2_top_z = existing_t2 ? existing_t2.bounds.max.z : 2350.mm

  # Ubicación de Bomba 2 (Bomba Evans 24V en el lateral de Tótem 1)
  if existing_p2
    p2_box = existing_p2.bounds
    p2_cx = (p2_box.min.x + p2_box.max.x) / 2.0
    p2_cy = (p2_box.min.y + p2_box.max.y) / 2.0
    p2_cz = p2_box.max.z
    p2_discharge_pt = Geom::Point3d.new(p2_cx, p2_cy, p2_cz)
  else
    p2_cx = t1_cx - 574.mm
    p2_cy = t1_cy
    p2_cz = 225.mm
    p2_discharge_pt = Geom::Point3d.new(p2_cx, p2_cy, p2_cz + 100.mm)
  end

  # Ubicación del Rack de Filtración y Manifold
  rack_x = existing_rack ? (existing_rack.bounds.min.x + existing_rack.bounds.max.x)/2.0 : 1800.mm
  rack_y = existing_rack ? existing_rack.bounds.min.y : -780.mm
  rack_z = 950.mm

  # Contenedor raíz unificado
  root_grp = entities.add_group
  root_grp.name = "Diagrama_Mejorado_AquaResiliencia_v2"
  r_ents = root_grp.entities

  # Si la Bomba 2 no existía en el archivo, se modela únicamente ella como BOMBA 2 (Evans 24V)
  unless existing_p2
    add_solid_box(r_ents, p2_cx - 80.mm, p2_cy - 60.mm, 0.mm, 160.mm, 120.mm, p2_cz - 50.mm, mat_galv, "Bancada_Bomba_2_Evans", layer_hidr)
    add_cylinder_tube(r_ents, p2_cx, p2_cy, p2_cz - 50.mm, 95.mm, 160.mm, mat_bomba, 0.mm, "Motor_Bomba_2_Evans_24V_1A", layer_hidr)
    add_cylinder_tube(r_ents, p2_cx, p2_cy, p2_cz + 110.mm, 85.mm, 65.mm, mat_cabeza, 0.mm, "Cabezal_Diafragma_145PSI", layer_hidr)
  end

  # ==============================================================================
  # ETAPA 1 (ELECTROQUÍMICA / BAJA PRESIÓN / GRAVEDAD):
  # ==============================================================================
  # Conservamos las celdas electrolíticas en 'U' y el banco modular de 6 celdas
  abs_od     = 1.900.inch
  abs_id     = 1.500.inch
  col_h      = 3000.mm
  col_sep    = 220.mm
  u_bottom_z = 350.mm

  c1_x = 450.mm
  c2_x = c1_x + col_sep
  c_y  = -150.mm

  add_solid_box(r_ents, c1_x - 90.mm, c_y - 90.mm, 0.mm, col_sep + 180.mm, 180.mm, 15.mm, mat_galv, "Placa_Base_Anclaje_Celdas", layer_galv)
  add_solid_box(r_ents, c1_x - 75.mm, c_y + 35.mm, 15.mm, 30.mm, 30.mm, 3150.mm, mat_galv, "Poste_Arriostre_Izq", layer_galv)
  add_solid_box(r_ents, c2_x + 45.mm, c_y + 35.mm, 15.mm, 30.mm, 30.mm, 3150.mm, mat_galv, "Poste_Arriostre_Der", layer_galv)

  [400.mm, 1300.mm, 2200.mm, 3100.mm].each_with_index do |brace_z, idx|
    add_solid_box(r_ents, c1_x - 75.mm, c_y + 20.mm, brace_z, col_sep + 150.mm, 15.mm, 30.mm, mat_galv, "Travesano_Abrazadera_#{idx+1}", layer_galv)
  end

  add_cylinder_tube(r_ents, c1_x, c_y, u_bottom_z, abs_od, col_h, mat_abs_vis, abs_id, "Columna_1_ABS_3m_Anodo", layer_galv)
  add_cylinder_tube(r_ents, c2_x, c_y, u_bottom_z, abs_od, col_h, mat_abs_vis, abs_id, "Columna_2_ABS_3m_Catodo", layer_galv)
  add_cylinder_tube(r_ents, c1_x, c_y, u_bottom_z, abs_id - 2.mm, col_h - 80.mm, mat_agua, 0.mm, "Electrolito_Col1", layer_galv)
  add_cylinder_tube(r_ents, c2_x, c_y, u_bottom_z, abs_id - 2.mm, col_h - 80.mm, mat_agua, 0.mm, "Electrolito_Col2", layer_galv)

  u_pts = generate_u_bend_points_xz(c1_x, c2_x, c_y, u_bottom_z + 20.mm, u_bottom_z, 14)
  add_pipe_along_path(r_ents, u_pts, abs_od + 4.mm, mat_abs_solid, "Puente_Inferior_U_ABS_1_5", layer_galv, 24)

  mid_u_x = (c1_x + c2_x) / 2.0
  lowest_u_z = u_bottom_z - (col_sep / 2.0)
  add_cylinder_tube(r_ents, mid_u_x, c_y, lowest_u_z - 90.mm, 28.mm, 90.mm, mat_abs_solid, 0.mm, "Tubo_Purga_Inferior", layer_galv)
  add_solid_box(r_ents, mid_u_x - 25.mm, c_y - 25.mm, lowest_u_z - 70.mm, 50.mm, 50.mm, 35.mm, mat_valv_roja, "Valvula_Purga_Lodos", layer_hidr)

  add_cylinder_tube(r_ents, c1_x, c_y, u_bottom_z + 40.mm, 18.mm, col_h - 60.mm, mat_zinc, 0.mm, "Anodo_Sacrificial_Zinc_3m", layer_galv)
  add_cylinder_tube(r_ents, c2_x, c_y, u_bottom_z + 100.mm, 32.mm, 1200.mm, mat_cobre, 28.mm, "Malla_Auxiliar_Cobre", layer_galv)

  inox_od = 0.375.inch
  inox_pts = generate_u_bend_points_xz(c2_x - 10.mm, c2_x + 10.mm, c_y, u_bottom_z + col_h + 120.mm, u_bottom_z + 60.mm, 10)
  add_pipe_along_path(r_ents, inox_pts, inox_od, mat_inox, "Serpentin_Catodo_Inox_3_8_U", layer_galv, 16)
  add_cylinder_tube(r_ents, c1_x, c_y, u_bottom_z + col_h + 60.mm, 45.mm, 90.mm, mat_agua, 38.mm, "Camara_Aire_Goteo_Aislado", layer_hidr)

  # Banco Modular de 6 Celdas
  mod_base_x = c1_x - 520.mm
  mod_base_y = c_y - 60.mm
  mod_z      = 400.mm
  mod_sp     = 85.mm

  add_solid_box(r_ents, mod_base_x - 30.mm, mod_base_y - 30.mm, 0.mm, (mod_sp * 3) + 20.mm, (mod_sp * 2) + 20.mm, mod_z, mat_galv, "Mesa_Soporte_6_Celdas", layer_galv)

  cell_tops = []
  2.times do |r|
    3.times do |c|
      idx = (r * 3) + c + 1
      cx = mod_base_x + (c * mod_sp)
      cy = mod_base_y + (r * mod_sp)
      add_cylinder_tube(r_ents, cx, cy, mod_z, abs_od, 1000.mm, mat_abs_vis, abs_id, "Celda_Independiente_#{idx}", layer_galv)
      add_cylinder_tube(r_ents, cx, cy, mod_z + 40.mm, 14.mm, 900.mm, mat_zinc, 0.mm, "Anodo_Zinc_Celda_#{idx}", layer_galv)
      add_cylinder_tube(r_ents, cx, cy, mod_z + 60.mm, 30.mm, 650.mm, mat_cobre, 26.mm, "Catodo_Malla_Cobre_#{idx}", layer_galv)
      cell_tops << Geom::Point3d.new(cx, cy, mod_z + 1000.mm)
    end
  end

  (0...5).each do |i|
    p1 = cell_tops[i]
    p2 = cell_tops[i+1]
    mid = Geom::Point3d.new((p1.x + p2.x)/2.0, (p1.y + p2.y)/2.0, p1.z + 35.mm)
    add_pipe_along_path(r_ents, [p1, mid, p2], 5.mm, mat_cable_pos, "Puente_Serie_Celda_#{i+1}_a_#{i+2}", layer_elec, 8)
  end

  # ==============================================================================
  # ETAPA 2 (SUCCIÓN DE TÓTEM 1 -> BOMBA 2 -> PERGOLADO COLECTOR SOLAR):
  # ==============================================================================
  # 1. Succión desde Tótem 1 (pichancha a media altura Z = 450mm para dejar sedimentos abajo)
  suction_origin = Geom::Point3d.new(t1_cx - 520.mm, t1_cy, t1_cz + 450.mm)
  suction_pts = [
    suction_origin,
    Geom::Point3d.new(p2_cx + 80.mm, t1_cy, t1_cz + 450.mm),
    Geom::Point3d.new(p2_cx + 80.mm, p2_cy, p2_cz + 20.mm),
    Geom::Point3d.new(p2_cx + 20.mm, p2_cy, p2_cz + 20.mm)
  ]
  add_pipe_along_path(r_ents, suction_pts, 20.mm, mat_azul, "Succion_Bomba2_Pichancha_MediaAltura", layer_hidr, 12)
  add_cylinder_tube(r_ents, suction_origin.x, suction_origin.y, suction_origin.z - 25.mm, 38.mm, 50.mm, mat_laton, 0.mm, "Pichancha_Retencion_MediaAltura", layer_hidr)

  # 2. Pérgola en el Techo (Estructura y Colector)
  perg_x = 1350.mm
  perg_y = -1850.mm
  perg_z = 2650.mm
  perg_w = 1750.mm
  perg_d = 2400.mm

  [
    [perg_x, perg_y],
    [perg_x + perg_w, perg_y],
    [perg_x, perg_y + perg_d],
    [perg_x + perg_w, perg_y + perg_d]
  ].each_with_index do |pos, i|
    add_solid_box(r_ents, pos[0], pos[1], 0.mm, 45.mm, 45.mm, perg_z, mat_galv, "Columna_Pergola_#{i+1}", layer_perg)
  end

  add_solid_box(r_ents, perg_x, perg_y, perg_z, perg_w + 45.mm, 45.mm, 45.mm, mat_galv, "Viga_Pergola_Sur", layer_perg)
  add_solid_box(r_ents, perg_x, perg_y + perg_d, perg_z, perg_w + 45.mm, 45.mm, 45.mm, mat_galv, "Viga_Pergola_Norte", layer_perg)
  add_solid_box(r_ents, perg_x, perg_y, perg_z, 45.mm, perg_d + 45.mm, 45.mm, mat_galv, "Viga_Pergola_Oeste", layer_perg)
  add_solid_box(r_ents, perg_x + perg_w, perg_y, perg_z, 45.mm, perg_d + 45.mm, 45.mm, mat_galv, "Viga_Pergola_Este", layer_perg)

  # 3. Línea de Impulsión de Bomba 2 hacia el Pergolado Solar en el Techo
  pump_to_perg_pts = [
    p2_discharge_pt,
    Geom::Point3d.new(p2_cx, p2_cy, p2_cz + 140.mm),
    Geom::Point3d.new(perg_x + 60.mm, p2_cy, p2_cz + 140.mm),
    Geom::Point3d.new(perg_x + 60.mm, perg_y + 60.mm, p2_cz + 140.mm),
    Geom::Point3d.new(perg_x + 60.mm, perg_y + 60.mm, perg_z + 55.mm)
  ]
  add_pipe_along_path(r_ents, pump_to_perg_pts, 16.mm, mat_azul, "Impulsion_Bomba2_Evans_Hacia_Pergola", layer_hidr, 14)

  # 4. Serpentín Colector Solar en Pérgola (~35m manguera PE 3/8")
  coil_pts = []
  coil_pts << Geom::Point3d.new(perg_x + 60.mm, perg_y + 60.mm, perg_z + 55.mm)

  num_loops = 24
  step_y    = (perg_d - 140.mm) / num_loops.to_f
  x_left    = perg_x + 80.mm
  x_right   = perg_x + perg_w - 60.mm
  z_coil    = perg_z + 55.mm

  num_loops.times do |i|
    y_curr = perg_y + 70.mm + (i * step_y)
    y_next = y_curr + (step_y * 0.5)
    if i.even?
      coil_pts << Geom::Point3d.new(x_left,  y_curr, z_coil)
      coil_pts << Geom::Point3d.new(x_right, y_curr, z_coil)
      coil_pts << Geom::Point3d.new(x_right + 12.mm, y_next, z_coil)
    else
      coil_pts << Geom::Point3d.new(x_right, y_curr, z_coil)
      coil_pts << Geom::Point3d.new(x_left,  y_curr, z_coil)
      coil_pts << Geom::Point3d.new(x_left - 12.mm,  y_next, z_coil)
    end
  end

  add_pipe_along_path(r_ents, coil_pts, 12.mm, mat_manguera, "Colector_Solar_Pergola_Manguera_3_8_35m", layer_perg, 12)

  # Válvula de alivio térmico de sobrecalentamiento en verano (>45°C)
  out_x = x_right
  out_y = perg_y + perg_d - 60.mm
  add_cylinder_tube(r_ents, out_x, out_y, z_coil + 10.mm, 38.mm, 75.mm, mat_laton, 0.mm, "Valvula_Alivio_Termico_Verano_45C", layer_hidr)

  # ==============================================================================
  # ETAPA 3 (BAJADA DE AGUA PRECALENTADA Y MANIFOLD REDISEÑADO):
  # ==============================================================================
  # Bajada de agua precalentada (30°C - 35°C) desde la pérgola hacia el Manifold en el bastidor
  mani_cx = rack_x
  mani_cy = rack_y
  mani_cz = rack_z

  hot_down_pts = [
    Geom::Point3d.new(out_x, out_y, z_coil),
    Geom::Point3d.new(out_x, out_y, mani_cz + 180.mm),
    Geom::Point3d.new(mani_cx - 180.mm, out_y, mani_cz + 180.mm),
    Geom::Point3d.new(mani_cx - 180.mm, mani_cy, mani_cz + 60.mm),
    Geom::Point3d.new(mani_cx - 180.mm, mani_cy, mani_cz)
  ]
  add_pipe_along_path(r_ents, hot_down_pts, 16.mm, mat_roja, "Bajada_Agua_Precalentada_35C_a_Manifold", layer_hidr, 14)

  # --- CUERPO PRINCIPAL DEL MANIFOLD REDISEÑADO ---
  # Colector horizontal de 1" de acero inoxidable / latón (450mm de longitud)
  mani_len = 420.mm
  add_solid_box(r_ents, mani_cx - 200.mm, mani_cy - 20.mm, mani_cz - 20.mm, mani_len, 40.mm, 40.mm, mat_laton, "Manifold_Distribuidor_Maestro", layer_hidr)

  # Manómetro de glicerina principal de entrada (0-150 PSI) montado en el cuerpo del manifold
  add_cylinder_tube(r_ents, mani_cx - 130.mm, mani_cy, mani_cz + 20.mm, 15.mm, 35.mm, mat_laton, 0.mm, "Cuello_Manometro_Entrada", layer_hidr)
  add_cylinder_tube(r_ents, mani_cx - 130.mm, mani_cy - 10.mm, mani_cz + 55.mm, 52.mm, 22.mm, mat_inox, 0.mm, "Manometro_Entrada_Presion_Bomba2", layer_hidr)

  # PUERTO 1: LÍNEA DOMÉSTICA (80% DEL CAUDAL) CON VÁLVULA DE ESFERA AZUL
  # Para uso en la casa: cisterna, inodoros, lavadoras y riego de plantas
  port1_x = mani_cx - 40.mm
  add_cylinder_tube(r_ents, port1_x, mani_cy, mani_cz - 55.mm, 20.mm, 35.mm, mat_laton, 0.mm, "Cople_Puerto1_Domestico", layer_hidr)
  add_solid_box(r_ents, port1_x - 22.mm, mani_cy - 22.mm, mani_cz - 95.mm, 44.mm, 44.mm, 40.mm, mat_valv_azul, "Valvula_Esfera_Azul_Domestica_80pct", layer_hidr)
  # Palanca azul ergonómica
  add_solid_box(r_ents, port1_x - 6.mm, mani_cy + 15.mm, mani_cz - 85.mm, 12.mm, 70.mm, 14.mm, mat_valv_azul, "Maneta_Azul_Linea_Domestica", layer_hidr)

  # Tubería de bajada a cisterna del hogar
  dom_pts = [
    Geom::Point3d.new(port1_x, mani_cy, mani_cz - 95.mm),
    Geom::Point3d.new(port1_x, mani_cy, 180.mm),
    Geom::Point3d.new(port1_x - 300.mm, mani_cy, 180.mm)
  ]
  add_pipe_along_path(r_ents, dom_pts, 18.mm, mat_azul, "Linea_Uso_Domestico_80pct_Cisterna", layer_hidr, 14)

  # PUERTO 2: LÍNEA HACIA ÓSMOSIS INVERSA (20% DEL CAUDAL @ 85-90 PSI)
  # Con regulador de aguja micrométrico calibrado para 85-90 PSI exactos
  port2_x = mani_cx + 100.mm
  add_cylinder_tube(r_ents, port2_x, mani_cy, mani_cz - 45.mm, 18.mm, 25.mm, mat_laton, 0.mm, "Cople_Puerto2_RO", layer_hidr)
  add_cylinder_tube(r_ents, port2_x, mani_cy, mani_cz - 105.mm, 42.mm, 60.mm, mat_valv_roja, 0.mm, "Regulador_Aguja_Calibrado_90_PSI", layer_hidr)
  add_cylinder_tube(r_ents, port2_x + 35.mm, mani_cy - 10.mm, mani_cz - 75.mm, 45.mm, 18.mm, mat_inox, 0.mm, "Manometro_Trabajo_RO_90PSI", layer_hidr)

  # Tubería hacia prefiltros de cartucho
  to_prefilters_pts = [
    Geom::Point3d.new(port2_x, mani_cy, mani_cz - 105.mm),
    Geom::Point3d.new(port2_x, mani_cy, mani_cz - 170.mm),
    Geom::Point3d.new(mani_cx + 220.mm, mani_cy, mani_cz - 170.mm)
  ]
  add_pipe_along_path(r_ents, to_prefilters_pts, 14.mm, mat_azul, "Alimentacion_Prefiltros_RO_90PSI", layer_hidr, 12)

  # Prefiltro 1: Sedimentos PP 5 micras (Vaso Portafiltro Azul 10")
  f1_x = mani_cx + 220.mm
  add_cylinder_tube(r_ents, f1_x, mani_cy, mani_cz - 460.mm, 110.mm, 270.mm, mat_filtro_az, 0.mm, "Vaso_Filtro_Sedimentos_5_Micras", layer_hidr)
  add_cylinder_tube(r_ents, f1_x, mani_cy, mani_cz - 190.mm, 118.mm, 40.mm, mat_cabeza, 0.mm, "Cabezal_Filtro_Sedimentos", layer_hidr)

  # Prefiltro 2: Bloque de Carbón Activado CTO (Vaso Portafiltro Azul 10")
  f2_x = f1_x + 140.mm
  add_cylinder_tube(r_ents, f2_x, mani_cy, mani_cz - 460.mm, 110.mm, 270.mm, mat_filtro_az, 0.mm, "Vaso_Filtro_Carbon_Activado_CTO", layer_hidr)
  add_cylinder_tube(r_ents, f2_x, mani_cy, mani_cz - 190.mm, 118.mm, 40.mm, mat_cabeza, 0.mm, "Cabezal_Filtro_Carbon", layer_hidr)

  # Niple de interconexión entre filtros
  add_pipe_along_path(r_ents, [Geom::Point3d.new(f1_x, mani_cy, mani_cz - 170.mm), Geom::Point3d.new(f2_x, mani_cy, mani_cz - 170.mm)], 14.mm, mat_inox, "Puente_Inter_Filtros", layer_hidr, 12)

  # Electroválvula Solenoide 24V DC normalmente cerrada (Relé 2, gobernada por presostato de 40 PSI)
  sol_x = f2_x + 90.mm
  add_solid_box(r_ents, sol_x - 20.mm, mani_cy - 20.mm, mani_cz - 190.mm, 40.mm, 40.mm, 50.mm, mat_laton, "Cuerpo_Solenoide_24V", layer_hidr)
  add_solid_box(r_ents, sol_x - 16.mm, mani_cy - 16.mm, mani_cz - 140.mm, 32.mm, 32.mm, 45.mm, mat_bat, "Bobina_Solenoide_24V_NC", layer_elec)

  add_pipe_along_path(r_ents, [Geom::Point3d.new(f2_x, mani_cy, mani_cz - 170.mm), Geom::Point3d.new(sol_x, mani_cy, mani_cz - 170.mm)], 14.mm, mat_inox, "Conexion_Filtro_a_Solenoide", layer_hidr, 12)

  # Portamembrana de Ósmosis Inversa (RO 4040 / Residencial Inox)
  ro_x = sol_x + 120.mm
  add_cylinder_tube(r_ents, ro_x, mani_cy, mani_cz - 520.mm, 105.mm, 520.mm, mat_inox, 0.mm, "Portamembrana_Osmosis_Inversa_RO", layer_hidr)
  add_cylinder_tube(r_ents, ro_x, mani_cy, mani_cz, 112.mm, 35.mm, mat_cabeza, 0.mm, "Tapa_Superior_Portamembrana", layer_hidr)

  # Conexión solenoide a entrada de membrana
  ro_in_pts = [
    Geom::Point3d.new(sol_x, mani_cy, mani_cz - 170.mm),
    Geom::Point3d.new(ro_x, mani_cy, mani_cz - 170.mm),
    Geom::Point3d.new(ro_x, mani_cy, mani_cz + 20.mm)
  ]
  add_pipe_along_path(r_ents, ro_in_pts, 12.mm, mat_azul, "Entrada_AltaPresion_a_Membrana_RO", layer_hidr, 12)

  # LÍNEA DE PERMEADO (AGUA PURIFICADA 20%) HACIA BARRIL SANKE D 40 PSI
  # Sale por el centro inferior de la membrana, pasa por sensor Hall [P-08] y va al barril
  barril_cx = 1000.mm
  barril_cy = -1600.mm
  barril_cz = 250.mm

  permeate_pts = [
    Geom::Point3d.new(ro_x, mani_cy, mani_cz - 520.mm),
    Geom::Point3d.new(ro_x, mani_cy, 120.mm),
    Geom::Point3d.new(barril_cx + 400.mm, mani_cy, 120.mm),
    Geom::Point3d.new(barril_cx + 400.mm, barril_cy, 120.mm),
    Geom::Point3d.new(barril_cx, barril_cy, 120.mm),
    Geom::Point3d.new(barril_cx, barril_cy, barril_cz + 280.mm)
  ]
  add_pipe_along_path(r_ents, permeate_pts, 8.mm, mat_agua, "Linea_Permeado_AguaPura_a_Barril_Sanke", layer_hidr, 10)

  # Sensor de Flujo Digital Efecto Hall [P-08]
  add_solid_box(r_ents, ro_x - 18.mm, mani_cy - 18.mm, 200.mm, 36.mm, 36.mm, 45.mm, mat_cabeza, "Sensor_Flujo_Hall_P08", layer_elec)

  # RECIRCULACIÓN COMPLETA DE SALMUERA DE RO HACIA TÓTEM 2 SUPERIOR (CERO DESPERDICIO)
  # El rechazo sale con contrapresión de 85-90 PSI y sube directo a la tapa del Tótem 2
  reject_pts = [
    Geom::Point3d.new(ro_x + 35.mm, mani_cy, mani_cz - 500.mm),
    Geom::Point3d.new(ro_x + 70.mm, mani_cy, mani_cz - 500.mm),
    Geom::Point3d.new(ro_x + 70.mm, mani_cy, t2_top_z + 80.mm),
    Geom::Point3d.new(t2_cx, t2_cy, t2_top_z + 80.mm),
    Geom::Point3d.new(t2_cx, t2_cy, t2_top_z + 20.mm)
  ]
  add_pipe_along_path(r_ents, reject_pts, 10.mm, mat_salmuera, "Retorno_Recirculacion_Salmuera_a_Totem2_Superior", layer_hidr, 12)
  # Válvula de contrapresión / restrictor de flujo regulable
  add_cylinder_tube(r_ents, ro_x + 70.mm, mani_cy, mani_cz - 350.mm, 28.mm, 45.mm, mat_valv_roja, 0.mm, "Valvula_Contrapresion_Rechazo_90PSI", layer_hidr)

  # ==============================================================================
  # SUBSISTEMA DE ENERGÍA Y CONTROL:
  # ==============================================================================
  # 6 Paneles solares de patio de 5W en serie (24V nom / 30W pico) sobre la pérgola
  elec_x = perg_x + 120.mm
  elec_y = perg_y + 100.mm
  roof_z = perg_z + 90.mm

  6.times do |p|
    px = elec_x + (p * 240.mm)
    add_solid_box(r_ents, px, elec_y, roof_z, 210.mm, 320.mm, 15.mm, mat_panel, "Panel_Solar_Patio_5W_Serie_#{p+1}", layer_elec)
    add_solid_box(r_ents, px - 5.mm, elec_y - 5.mm, roof_z - 4.mm, 220.mm, 330.mm, 10.mm, mat_galv, "Marco_Aluminio_Panel_#{p+1}", layer_elec)
  end

  # Gabinete de Control IoT IP65 y Baterías LiFePO4 EVE BF1024
  gab_x = rack_x - 320.mm
  gab_y = rack_y - 60.mm
  gab_z = 850.mm

  add_solid_box(r_ents, gab_x, gab_y, gab_z, 280.mm, 160.mm, 360.mm, mat_gabinete, "Gabinete_Control_Timer_y_Baterias", layer_elec)

  3.times do |b|
    bx = gab_x + 25.mm + (b * 80.mm)
    by = gab_y - 35.mm
    bz = gab_z + 50.mm
    add_solid_box(r_ents, bx, by, bz, 65.mm, 38.mm, 130.mm, mat_bat, "Bateria_POS_EVE_BF1024_7_2V_2600mAh_#{b+1}", layer_elec)
  end

  # Cableado eléctrico
  solar_cable_pts = [
    Geom::Point3d.new(elec_x, elec_y, roof_z),
    Geom::Point3d.new(gab_x + 140.mm, elec_y, roof_z),
    Geom::Point3d.new(gab_x + 140.mm, gab_y + 80.mm, gab_z + 360.mm)
  ]
  add_pipe_along_path(r_ents, solar_cable_pts, 8.mm, mat_cable_pos, "Bajada_DC_6_Paneles_Serie_24V_30W", layer_elec, 8)

  pump_cable_pts = [
    Geom::Point3d.new(gab_x + 140.mm, gab_y + 80.mm, gab_z),
    Geom::Point3d.new(gab_x + 140.mm, p2_cy, p2_cz + 50.mm),
    Geom::Point3d.new(p2_cx, p2_cy, p2_cz + 50.mm)
  ]
  add_pipe_along_path(r_ents, pump_cable_pts, 8.mm, mat_cable_neg, "Alimentacion_Bomba_2_Evans_24V_1A", layer_elec, 8)

  # ==============================================================================
  # ETIQUETAS TÉCNICAS 3D Y COTAS MAESTRAS
  # ==============================================================================
  notes_grp = r_ents.add_group
  notes_grp.name = "Cotas_y_Especificaciones_Tecnicas"
  notes_grp.layer = layer_notes
  n_ents = notes_grp.entities

  annotations = [
    [
      suction_origin,
      Geom::Vector3d.new(-220.mm, -180.mm, 200.mm),
      "2 BOMBAS TOTALES EN EL SISTEMA:\n1. Bomba Pozo Somero (-7.5m -> Tótem 2 Superior)\n2. Bomba Impulsora Evans 24V (Tótem 1 Inferior)\n-> Succión con pichancha a media altura (evita sedimentos)\n-> Impulsa directo al Pergolado Solar en techo"
    ],
    [
      Geom::Point3d.new(perg_x + (perg_w / 2.0), perg_y + (perg_d / 2.0), z_coil + 20.mm),
      Geom::Vector3d.new(0.mm, -250.mm, 350.mm),
      "PERGOLADO COLECTOR SOLAR TÉRMICO\n~35m Manguera Polietileno Negra 3/8\"\nPrecalienta agua a 30°C - 35°C con el sol\nTriplica el flujo de la membrana RO\nVálvula Alivio Térmico Verano (>45°C)"
    ],
    [
      Geom::Point3d.new(mani_cx, mani_cy, mani_cz + 50.mm),
      Geom::Vector3d.new(180.mm, -220.mm, 240.mm),
      "MANIFOLD DISTRIBUIDOR CENTRAL REDISEÑADO\nEntrada: Agua precalentada 30-35°C de pérgola\nSalida 1: 80% Doméstico (Cisterna/Casa) con Válvula Azul\nSalida 2: 20% RO con regulador aguja a 85-90 PSI exactos\nFiltros 5µm + Carbón CTO + Solenoide 24V NC"
    ],
    [
      Geom::Point3d.new(ro_x + 70.mm, mani_cy, t2_top_z),
      Geom::Vector3d.new(200.mm, 150.mm, 150.mm),
      "CIRCUITO CERRADO CERO DESPERDICIO:\nRetorno del 100% de la Salmuera de Rechazo de RO\nRecircula directo hacia Tótem 2 Superior\nVuelve al ciclo electrofloculador -> decantador"
    ],
    [
      Geom::Point3d.new(c1_x, c_y, u_bottom_z + col_h + 100.mm),
      Geom::Vector3d.new(-240.mm, -180.mm, 200.mm),
      "ETAPA 1 (ELECTROQUÍMICA / GRAVEDAD)\nColumnas ABS 1.5\" x 3.0m en 'U'\nÁnodo: Zinc 3m | Cátodo/Termosifón: Inox 3/8\"\nPurga inferior de lodos y cámara de goteo aislado"
    ],
    [
      Geom::Point3d.new(gab_x + 120.mm, gab_y - 35.mm, gab_z + 80.mm),
      Geom::Vector3d.new(-180.mm, -240.mm, -80.mm),
      "RESPALDO ENERGÉTICO HÍBRIDO 24V\n6 Paneles Patio 5W en Serie (30W pico)\n3 Celdas POS EVE BF1024 en Serie (21.6V / 56 Wh)\nControl automático por ciclos / nivel / presostato"
    ]
  ]

  annotations.each do |pt, vec, label_text|
    n_ents.add_text(label_text, pt, vec)
  end

  dim_pt1 = Geom::Point3d.new(c1_x, c_y, u_bottom_z)
  dim_pt2 = Geom::Point3d.new(c1_x, c_y, u_bottom_z + col_h)
  n_ents.add_dimension_linear(dim_pt1, dim_pt2, Geom::Vector3d.new(-160.mm, 0.mm, 0.mm))

  model.commit_operation
  Sketchup.active_model.active_view.zoom_extents
  puts ">>> [OK] Diagrama AquaResiliencia v2.2 actualizado exitosamente:"
  puts "    - EXACTAMENTE 2 BOMBAS (Pozo Somero + Impulsión Tótem 1 a Pérgola)."
  puts "    - Bomba 2 conectada al Pergolado Colector Solar en el techo."
  puts "    - Manifold rediseñado: 80% Doméstico (Válvula Azul) / 20% RO (85-90 PSI)."
  puts "    - Recirculación total de salmuera hacia Tótem 2 Superior."
  UI.messagebox("¡Diagrama AquaResiliencia v2.2 Actualizado!\n\n✔ EXACTAMENTE 2 BOMBAS en el sistema (Pozo + Tótem 1).\n✔ Bomba de Tótem de abajo conectada al Pergolado Solar en techo.\n✔ Manifold Rediseñado (80% Doméstico con válvula azul / 20% RO a 90 PSI).\n✔ Recirculación 100% de salmuera a Tótem 2 Superior.\n✔ Celdas electrolíticas, paneles y cotas 3D perfectamente integradas.")

rescue => e
  model.abort_operation
  puts ">>> [ERROR EN EJECUCIÓN SKETCHUP]: #{e.class}: #{e.message}"
  puts e.backtrace.join("\n")
  UI.messagebox("Error al actualizar diagrama:\n#{e.class}: #{e.message}")
end
