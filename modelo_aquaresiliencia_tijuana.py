#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
================================================================================
  MODELO HIDROGEOLOGICO DE LADERA URBANA -- ACUIFERO TIJUANA 0201
  AquaResiliencia: Micropozos de Alivio vs. Saturacion por Fugas de Red
================================================================================

Autores    : Equipo AquaResiliencia / GIA Tijuana
Version    : 2.0 -- Septiembre 2026
Convocatoria: Ventures Hack 2026 -- CDT / FIDEM / Tijuana Innovadora

ESCENARIO 1 (BASE):
  Fugas cronicas de la red potable CESPT (~24% perdidas) infiltran sobre
  las arcillas impermeables de la Fm. Otay (K < 1e-7 m/s), elevando la
  presion de poro progresivamente hasta que el Factor de Seguridad (FS)
  cae por debajo de 1.0 -> FALLA CATASTROFICA DE LADERA.

ESCENARIO 2 (AQUARESILIENCIA):
  Red distribuida de micropozos someros de bajo caudal (0.3-0.8 L/s c/u)
  abate el nivel freatico, reduce las presiones de poro y restaura FS > 1.5.

FISICA IMPLEMENTADA:
  - Ley de Darcy + balance volumetrico transitorio en la capa aluvial.
  - Metodo de Bishop Simplificado para talud circular (20 dovelas).
  - Criterio de Mohr-Coulomb.
  - Presion de poro: u = gamma_w * h_w.

PARAMETROS: CONAGUA DR_0201 (2023), SGM/INEGI, Gudino Elizondo (CICESE 2018).
DEPENDENCIAS: pip install numpy matplotlib
EJECUCION: python modelo_aquaresiliencia_tijuana.py
================================================================================
"""

import numpy as np
import matplotlib
matplotlib.use("Agg")   # sin GUI; cambiar a "TkAgg" si se quiere ventana
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
from matplotlib.gridspec import GridSpec
import warnings
warnings.filterwarnings("ignore")

plt.rcParams.update({
    "figure.dpi": 120, "savefig.dpi": 300,
    "font.family": "DejaVu Sans",
    "axes.titlesize": 12, "axes.labelsize": 10,
    "xtick.labelsize": 9, "ytick.labelsize": 9,
    "legend.fontsize": 9, "axes.grid": True, "grid.alpha": 0.30,
})


# ==============================================================================
#  BLOQUE 1 -- PARAMETROS FISICOS DOCUMENTADOS (ACUIFERO TIJUANA 0201)
# ==============================================================================

class ParametrosAcuifero:
    """
    Parametros calibrados del Acuifero Tijuana 0201.

    Fuentes primarias:
      [1] CONAGUA (2023). Actualizacion Disponibilidad Media Anual Acuifero
          Tijuana (0201). DOF. [DR_0201]
      [2] SGM/INEGI (2020). Cartografia Geologico-Minera, Hoja Tijuana H11-1.
      [3] Gudino Elizondo, N. (2018). Tesis Doctoral, CICESE, Ensenada BC.
      [4] Wakida, F.T. et al. (2008). Hydrological Processes, 22(14):2441-2448.

    Estratigrafia representativa en canones urbanos de Tijuana:
      Capa 1: Aluv. Cuaternario (Qal) + Rellenos Urbanos -- 0.0 a 6.0 m
      Capa 2: Fm. San Diego (Tsd) / Fm. Rosarito Beach (Trm) -- 6.0 a 35.0 m
      Capa 3: Fm. Otay (To) -- ACUICLUDO IMPERMEABLE -- > 35.0 m
              (aflora en laderas de canones a ~6-8 m de profundidad)
    """

    # ---- Capa Superior: Aluv. Cuaternario (Qal) -- 0.0 a 6.0 m -------------
    H_aluvion    = 6.0           # [m]    Espesor capa aluvial
    # NOTA CALIBRACION: En posiciones de ladera, el aluv. es mas fino (limo-arenoso)
    # que en el fondo del valle. K representativa de relleno urbano compactado
    # sobre la interfaz Qal/Otay: extremo inferior del rango DR_0201 (1.5e-5 m/s).
    K_aluvion    = 1.5e-5        # [m/s]  K ladera (limite inferior DR_0201 p.18)
    n_aluvion    = 0.32          # [-]    Porosidad total (rango 0.28-0.38)
    Sy_aluvion   = 0.15          # [-]    Rendimiento especifico (rango 0.12-0.16)
    gamma_d      = 17.0e3        # [N/m3] Peso especifico seco
    gamma_sat    = 19.5e3        # [N/m3] Peso especifico saturado
    phi_aluvion  = np.radians(28.0)   # [rad] Angulo friccion interna efectiva
    c_aluvion    = 12.0e3        # [Pa]   Cohesion efectiva (12 kPa)

    # ---- Capa Intermedia: Fm. San Diego / Rosarito Beach -- 6.0 a 35.0 m ---
    H_sandiego   = 29.0          # [m]
    K_sandiego   = 5.0e-6        # [m/s]  (rango 1e-6 a 1e-5)
    n_sandiego   = 0.22
    Sy_sandiego  = 0.08

    # ---- Capa Inferior: Fm. Otay (To) -- ACUICLUDO -------------------------
    K_otay       = 5.0e-8        # [m/s]  < 1e-7 m/s (impermeable practico)
    phi_otay     = np.radians(16.0)
    c_otay       = 45.0e3        # [Pa]   Arcilla sobreconsolidada (Otay Hardpan)

    # ---- Geometria de la ladera --------------------------------------------
    H_talud      = 8.0           # [m]    Altura total del talud representativo
    beta_deg     = 38.0          # [deg]  Angulo de inclinacion de la ladera
    beta         = np.radians(38.0)
    L_talud      = 8.0 / np.tan(np.radians(38.0))   # [m] Longitud horizontal

    # ---- Condiciones iniciales ---------------------------------------------
    h_freatic_ini = 0.5          # [m]    NAF inicial (condicion seca)
    gamma_w       = 9810.0       # [N/m3] Peso especifico del agua

    # ---- Fugas de red CESPT (Infiltracion continua) ------------------------
    # Perdidas fisicas continuas en red potable (~24% perdidas CESPT).
    # En laderas con arcilla Otay impermeable, las fugas saturan el aluvion
    # silenciosamente durante todo el ano sin drenaje vertical.
    q_fuga       = 0.025 / 86400.0    # [m/s] fuga continua concentrada (25 mm/dia)

    # ---- Recarga Pluvial / Lluvias (CONAGUA DR_0201) -----------------------
    # Precipitacion media anual en Tijuana: 230 a 300 mm/ano (regimen mediterraneo).
    # Lluvias concentradas en invierno (nov-mar) con tormentas de 25-60 mm/dia
    # que actuan como el detonador hidraulico final sobre el suelo pre-saturado.
    p_anual_mm        = 260.0         # [mm/ano] media anual oficial CONAGUA
    coef_infil_lluvia = 0.40          # fraccion que infiltra en ladera coluvial permeable

    # ---- Micropozos AquaResiliencia ----------------------------------------
    # Bombeo frugal de bajo caudal calibrado por ensayos en prototipo experimental
    n_pozos       = 5             # numero de micropozos en la seccion analizada
    Q_pozo        = 0.5e-3        # [m3/s] caudal por pozo (0.5 L/s)
    r_influencia  = 8.0           # [m]   radio de influencia de cada pozo
    prof_pozo     = 5.0           # [m]   profundidad del micropozo (somero)

    # ---- Tiempo de simulacion ----------------------------------------------
    dias_total          = 365
    dt_dias             = 1.0
    dias_inicio_mitiga  = 120      # dia en que se activan los micropozos


# ==============================================================================
#  BLOQUE 2 -- MODELO TRANSITORIO DE NIVEL FREATICO (BALANCE VOLUMETRICO)
# ==============================================================================

def generar_serie_lluvia(t_dias, p):
    """
    Genera serie diaria de precipitacion pluvial en Tijuana (mm/dia)
    y tasa de infiltracion efectiva en la ladera q_lluvia (m/s).
    Clima mediterraneo: lluvias concentradas en invierno (noviembre a marzo)
    con tormentas de 20 a 55 mm/dia que actuan como detonador hidraulico.
    """
    n = len(t_dias)
    precip_mm = np.zeros(n)
    # Eventos de tormenta invernal tipicos de Tijuana (centro, pico mm/d, duracion dias)
    tormentas = [
        (15,  24.0, 2.5),
        (35,  48.0, 3.5),   # Tormenta invernal intensa (detonador)
        (65,  35.0, 3.0),
        (85,  22.0, 2.0),
        (315, 30.0, 2.5),
        (345, 52.0, 4.0)    # Tormenta severa de fin de ano
    ]
    for dia_c, p_pico, dur in tormentas:
        pulso = p_pico * np.exp(-0.5 * ((t_dias - dia_c) / (dur / 2.0))**2)
        precip_mm += pulso

    # Infiltracion efectiva [m/s] sobre la ladera coluvial
    q_infil_lluvia = (precip_mm * p.coef_infil_lluvia / 1000.0) / 86400.0
    return precip_mm, q_infil_lluvia

def simular_nivel_freatico(params):
    """
    Evolucion temporal del NAF mediante balance volumetrico acoplado:
    Recarga Total = Fugas de Red CESPT + Infiltracion Pluvial (Lluvias).

      Sy * dh/dt = (Q_fugas + Q_lluvia - Q_drenaje_lateral [- Q_bombeo]) / A_cuenca
    """
    p = params
    t_dias    = np.arange(0, p.dias_total + p.dt_dias, p.dt_dias)
    n_t       = len(t_dias)
    h_base    = np.zeros(n_t)
    h_mitiga  = np.zeros(n_t)
    h_base[0]    = p.h_freatic_ini
    h_mitiga[0]  = p.h_freatic_ini

    precip_mm, q_infil_lluvia = generar_serie_lluvia(t_dias, p)

    W         = 10.0                             # ancho de la seccion [m]
    A_cuenca  = W * p.L_talud                    # area tributaria [m2]
    factor_bloqueo = 0.05                        # acuicludo Fm. Otay impide drenaje libre
    T_aluv    = p.K_aluvion * p.H_aluvion * factor_bloqueo  # [m2/s]
    dt_s      = p.dt_dias * 86400.0              # dt en segundos
    Q_fugas   = p.q_fuga * A_cuenca              # caudal continuo fugas CESPT [m3/s]
    Q_bomb    = p.Q_pozo * p.n_pozos             # caudal bombeo total [m3/s]

    for i in range(1, n_t):
        # Caudal de recarga combinado: Fuga CESPT + Infiltracion de lluvia del dia
        Q_recarga_dia = Q_fugas + q_infil_lluvia[i] * A_cuenca

        # -- Escenario base (sin mitigacion) --
        hb        = max(h_base[i-1], 0.0)
        Qdb       = T_aluv * hb / p.L_talud * W
        dh_b      = (Q_recarga_dia - Qdb) * dt_s / (A_cuenca * p.Sy_aluvion)
        h_base[i] = np.clip(hb + dh_b, 0.0, p.H_aluvion)

        # -- Escenario AquaResiliencia (con micropozos) --
        hm        = max(h_mitiga[i-1], 0.0)
        Qdm       = T_aluv * hm / p.L_talud * W
        if t_dias[i] >= p.dias_inicio_mitiga:
            Qbeff = Q_bomb * min(hm / p.prof_pozo, 1.0)
        else:
            Qbeff = 0.0
        dh_m         = (Q_recarga_dia - Qdm - Qbeff) * dt_s / (A_cuenca * p.Sy_aluvion)
        h_mitiga[i]  = np.clip(hm + dh_m, 0.0, p.H_aluvion)

    return t_dias, h_base, h_mitiga, precip_mm


# ==============================================================================
#  BLOQUE 3 -- FACTOR DE SEGURIDAD: METODO DE BISHOP SIMPLIFICADO
# ==============================================================================

def factor_seguridad_bishop(h_freatic, params, n_dovelas=20):
    """
    Factor de Seguridad (FS) por Bishop Simplificado para superficie de falla
    circular en la ladera con nivel freatico variable.

    Formulacion (Bishop 1955, Geotechnique 5(1):7-17):

      FS = Sum[(c_prima*b + (W*cos(a) - u*b)*tan(phi_prima)) / m_alpha]
           / Sum[W * sin(a)]

    Donde:
      m_alpha = cos(a) + sin(a)*tan(phi_prima) / FS   (convergencia iterativa)
      W  = peso de la dovela (parte seca + parte saturada)
      u  = presion de poro en la base = gamma_w * h_sat
      b  = ancho de la dovela
      a  = angulo de la base de la dovela respecto a la horizontal

    Geometria: superficie circular tangente a la base de la capa aluvial;
    el angulo alpha varia linealmente de beta (pie) a 0 (cresta).

    Parametros
    ----------
    h_freatic : float -- NAF sobre la base aluvial [m]
    params    : ParametrosAcuifero
    n_dovelas : int   -- numero de dovelas (default 20)

    Retorna
    -------
    FS : float
    """
    p = params
    H = p.H_talud

    # Discretizacion de dovelas
    x_div  = np.linspace(0.0, p.L_talud, n_dovelas + 1)
    x_mid  = 0.5 * (x_div[:-1] + x_div[1:])
    b_dov  = p.L_talud / n_dovelas

    # Altura del terreno en cada x (perfil lineal del talud)
    z_sup  = np.maximum(H - x_mid * np.tan(p.beta), 0.0)

    # Angulo alpha: lineal de beta en el pie a 0 en la cresta
    alpha  = p.beta * (1.0 - x_mid / p.L_talud)
    alpha  = np.clip(alpha, -np.pi / 4, np.pi / 4)

    # Alturas saturada y seca
    h_sat  = np.minimum(h_freatic * np.ones(n_dovelas), z_sup)
    h_dry  = np.maximum(z_sup - h_freatic, 0.0)

    # Peso de la dovela por unidad de ancho [N/m]
    W      = (h_dry * p.gamma_d + h_sat * p.gamma_sat) * b_dov

    # Presion de poro en la base [Pa]
    u      = p.gamma_w * h_sat

    # Iteracion de Bishop
    FS = 1.5
    for _ in range(80):
        m_a = np.cos(alpha) + np.sin(alpha) * np.tan(p.phi_aluvion) / FS
        m_a = np.where(np.abs(m_a) < 1e-6, 1e-6, m_a)
        resistencia = (p.c_aluvion * b_dov
                       + (W * np.cos(alpha) - u * b_dov) * np.tan(p.phi_aluvion))
        num         = np.sum(resistencia / m_a)
        den         = np.sum(W * np.sin(alpha))
        if abs(den) < 1e-3:
            return 99.9
        FS_new = num / den
        if abs(FS_new - FS) < 1e-7:
            break
        FS = FS_new
    return max(FS, 0.01)


def calcular_series_fs(t_dias, h_base, h_mitiga, params):
    """Arrays de FS para ambos escenarios."""
    print("     Calculando FS escenario base (Bishop)...")
    fs_b = np.array([factor_seguridad_bishop(h, params) for h in h_base])
    print("     Calculando FS escenario mitigacion...")
    fs_m = np.array([factor_seguridad_bishop(h, params) for h in h_mitiga])
    return fs_b, fs_m


# ==============================================================================
#  BLOQUE 4 -- REPORTE NUMERICO EN CONSOLA
# ==============================================================================

def imprimir_reporte(t_dias, h_base, h_mitiga, fs_base, fs_mitiga, params):
    """Imprime tabla de resultados y resumen final."""
    p   = params
    sep = "=" * 74
    print("\n" + sep)
    print("  REPORTE NUMERICO -- AQUARESILIENCIA TIJUANA (ACUIFERO 0201)")
    print(sep)
    print(f"  Simulacion: {p.dias_total} dias | dt={p.dt_dias} dia | Bishop 20 dovelas")
    print(f"  H_talud={p.H_talud}m | beta={p.beta_deg}deg | L_base={p.L_talud:.1f}m")
    print(f"  K_aluv={p.K_aluvion:.2e}m/s | Sy={p.Sy_aluvion} | n={p.n_aluvion}")
    print(f"  phi_prima={np.degrees(p.phi_aluvion):.0f}deg | c_prima={p.c_aluvion/1000:.1f}kPa")
    print(f"  K_otay={p.K_otay:.2e}m/s (acuicludo Fm. Otay)")
    print(f"  q_fuga={p.q_fuga*86400*1000:.3f}mm/dia | {p.n_pozos}pozos x {p.Q_pozo*1000:.1f}L/s activan dia{p.dias_inicio_mitiga}")
    print(sep)
    print(f"  {'DIA':>5} | {'h_base':>9} | {'h_mitiga':>10} | {'FS_base':>9} | {'FS_mitiga':>11} | Estado")
    print("  " + "-" * 68)
    for d in [0, 30, 60, 90, 120, 150, 180, 210, 240, 270, 300, 330, 365]:
        idx = min(int(d), len(t_dias) - 1)
        fb = fs_base[idx]
        fm = fs_mitiga[idx]
        eb = "FALLA"   if fb < 1.0 else ("CRIT" if fb < 1.3 else "OK")
        em = "ESTABLE" if fm >= 1.5 else ("CRIT" if fm >= 1.0 else "RISC")
        print(f"  {d:>5} | {h_base[idx]:>8.3f}m | {h_mitiga[idx]:>9.3f}m | "
              f"{fb:>9.4f} | {fm:>11.4f} | Base:{eb} / Mitiga:{em}")

    hb = h_base[-1];  hm = h_mitiga[-1]
    fb = fs_base[-1]; fm = fs_mitiga[-1]
    ub = p.gamma_w * hb / 1000.0
    um = p.gamma_w * hm / 1000.0
    vb = p.Q_pozo * p.n_pozos * (p.dias_total - p.dias_inicio_mitiga) * 86400.0
    red_u = (ub - um) / max(ub, 0.01) * 100.0

    print("\n" + sep)
    print("  RESUMEN FINAL (Dia 365)")
    print(sep)
    print(f"  NAF base:     {hb:.3f}m | u={ub:.2f}kPa | FS={fb:.4f} "
          f"({'FALLA CATASTROFICA' if fb < 1.0 else 'Inestable'})")
    print(f"  NAF mitigado: {hm:.3f}m | u={um:.2f}kPa | FS={fm:.4f} "
          f"({'ESTABLE' if fm >= 1.5 else 'Marginal'})")
    print(f"  Abatimiento:  {hb - hm:.3f}m | Reduccion presion poro: {red_u:.1f}%")
    print(f"  Vol. bombeado acumulado: {vb:.0f}m3 ({vb/1000:.3f} hm3/anio)")

    idx_f = np.where(fs_base < 1.0)[0]
    if len(idx_f) > 0:
        print(f"\n  ADVERTENCIA: Falla geotecnica en escenario base: DIA {t_dias[idx_f[0]]:.0f}")
    else:
        print(f"\n  INFO: FS no cae por debajo de 1.0 en los {p.dias_total} dias simulados.")

    idx_e = np.where((fs_mitiga >= 1.5) & (t_dias >= p.dias_inicio_mitiga))[0]
    if len(idx_e) > 0:
        print(f"  OK: FS >= 1.5 restaurado en AquaResiliencia: DIA {t_dias[idx_e[0]]:.0f}")
    print(sep + "\n")


# ==============================================================================
#  BLOQUE 5 -- FIGURA 1: SERIES TEMPORALES (4 paneles)
# ==============================================================================

def _estilo_ax_oscuro(ax):
    ax.set_facecolor("#161B22")
    for sp in ax.spines.values():
        sp.set_edgecolor("#30363D")
    ax.tick_params(colors="#C9D1D9")
    ax.xaxis.label.set_color("#C9D1D9")
    ax.yaxis.label.set_color("#C9D1D9")
    ax.title.set_color("#E6EDF3")
    ax.grid(True, color="#21262D", linewidth=0.6)


def figura_series_temporales(t_dias, h_base, h_mitiga, fs_base, fs_mitiga, params, precip_mm=None):
    """
    Figura 1: 4 paneles con series temporales.
      (a) NAF(t) -- nivel freatico
      (b) u(t)   -- presion de poro
      (c) FS(t)  -- Factor de Seguridad con zonas de alerta
      (d) Volumen bombeado acumulado y reduccion de presion de poro
    """
    p   = params
    CB  = "#FF6B6B"   # color escenario base
    CM  = "#4DD0E1"   # color mitigacion
    CD  = "#FFD54F"   # color dia activacion

    fig = plt.figure(figsize=(16, 12))
    fig.patch.set_facecolor("#0D1117")
    gs  = GridSpec(2, 2, figure=fig, hspace=0.42, wspace=0.35,
                   left=0.08, right=0.96, top=0.92, bottom=0.07)
    axes = [fig.add_subplot(gs[r, c]) for r in range(2) for c in range(2)]
    for ax in axes:
        _estilo_ax_oscuro(ax)

    u_base   = params.gamma_w * h_base   / 1000.0   # [kPa]
    u_mitiga = params.gamma_w * h_mitiga / 1000.0

    # (a) NAF e Infiltracion Acoplada (Lluvias + Fugas CESPT)
    ax = axes[0]
    ax.plot(t_dias, h_base,   color=CB, lw=2.5, label="Base (Fuga CESPT + Lluvia)", zorder=4)
    ax.plot(t_dias, h_mitiga, color=CM, lw=2.5, label="AquaResiliencia (Alivio)", zorder=4)
    ax.fill_between(t_dias, h_base, h_mitiga,
                    where=h_base > h_mitiga, alpha=0.18, color=CM, label="Abatimiento", zorder=3)
    ax.axvline(p.dias_inicio_mitiga, color=CD, lw=1.5, ls="--", alpha=0.85, zorder=5)
    ax.axhline(p.H_aluvion, color=CB, lw=1.2, ls=":", alpha=0.55,
               label=f"Saturacion total ({p.H_aluvion}m)")
    ax.text(p.dias_inicio_mitiga + 4, p.H_aluvion * 0.42,
            f"Dia {p.dias_inicio_mitiga}\nMicropozos\nACTIVOS",
            color=CD, fontsize=8.5, fontweight="bold", zorder=6)
    ax.set_xlabel("Tiempo (dias)")
    ax.set_ylabel("Nivel freatico NAF (m)")
    ax.set_title("(a) Nivel Freatico: Fugas CESPT + Tormentas Pluviales", fontweight="bold")
    ax.set_ylim(-0.1, p.H_aluvion + 0.6)
    ax.legend(loc="upper left", framealpha=0.3, facecolor="#0D1117",
              edgecolor="#30363D", labelcolor="#C9D1D9")

    # Eje secundario para hietograma de lluvia invertido (estandar hidrologico)
    if precip_mm is not None:
        ax_rain = ax.twinx()
        ax_rain.bar(t_dias, precip_mm, width=1.0, color="#38BDF8", alpha=0.32, label="Precipitacion (mm/d)")
        max_p = max(precip_mm.max() * 3.5, 60.0)
        ax_rain.set_ylim(max_p, 0.0) # Invertido
        ax_rain.set_ylabel("Lluvia (mm/dia)", color="#38BDF8", fontsize=8.5)
        ax_rain.tick_params(axis="y", colors="#38BDF8", labelsize=8)
        ax_rain.grid(False)

    # (b) Presion de poro
    ax = axes[1]
    ax.plot(t_dias, u_base,   color=CB, lw=2.5, label="Base")
    ax.plot(t_dias, u_mitiga, color=CM, lw=2.5, label="AquaResiliencia")
    ax.fill_between(t_dias, u_base, u_mitiga,
                    where=u_base > u_mitiga, alpha=0.18, color="#66BB6A")
    ax.axvline(p.dias_inicio_mitiga, color=CD, lw=1.5, ls="--", alpha=0.85)
    ax.text(5, u_base.max() * 0.97, f"u_max = {u_base.max():.1f} kPa", color=CB, fontsize=9)
    ax.text(5, u_mitiga[-1] * 0.75 + 1.0, f"u_mit = {u_mitiga[-1]:.1f} kPa", color=CM, fontsize=9)
    ax.set_xlabel("Tiempo (dias)")
    ax.set_ylabel("Presion de poro  u = gw * h  (kPa)")
    ax.set_title("(b) Presion de Poro en Sup. de Falla", fontweight="bold")
    ax.legend(loc="upper left", framealpha=0.3, facecolor="#0D1117",
              edgecolor="#30363D", labelcolor="#C9D1D9")

    # (c) Factor de Seguridad
    ax = axes[2]
    ax.axhspan(0,   1.0, alpha=0.18, color="#E53935", zorder=0)
    ax.axhspan(1.0, 1.3, alpha=0.12, color="#FF9800", zorder=0)
    ax.axhspan(1.3, 1.5, alpha=0.09, color="#FFEB3B", zorder=0)
    ax.axhspan(1.5, 4.0, alpha=0.07, color="#4CAF50", zorder=0)

    ax.plot(t_dias, fs_base,   color=CB, lw=2.8, label="Base (fuga cronica)", zorder=5)
    ax.plot(t_dias, fs_mitiga, color=CM, lw=2.8, label="AquaResiliencia",     zorder=5)
    ax.axhline(1.0, color="#E53935", lw=1.8, ls="--", alpha=0.9)
    ax.axhline(1.3, color="#FF9800", lw=1.3, ls="-.", alpha=0.7)
    ax.axhline(1.5, color="#66BB6A", lw=1.3, ls=":",  alpha=0.7)
    ax.axvline(p.dias_inicio_mitiga, color=CD, lw=1.5, ls="--", alpha=0.85)

    idx_f = np.where(fs_base < 1.0)[0]
    if len(idx_f) > 0:
        tf = t_dias[idx_f[0]]
        ax.annotate(f"FALLA\nDia {tf:.0f}",
                    xy=(tf, 1.0), xytext=(max(tf - 55, 10), 0.55),
                    color=CB, fontsize=9, fontweight="bold",
                    arrowprops=dict(arrowstyle="->", color=CB, lw=1.5))

    idx_r = np.where((fs_mitiga > 1.5) & (t_dias > p.dias_inicio_mitiga))[0]
    if len(idx_r) > 0:
        tr = t_dias[idx_r[0]]
        ax.annotate(f"FS>=1.5\nDia {tr:.0f}",
                    xy=(tr, 1.5), xytext=(min(tr + 12, p.dias_total - 55), 1.85),
                    color=CM, fontsize=9, fontweight="bold",
                    arrowprops=dict(arrowstyle="->", color=CM, lw=1.5))

    for y_t, etq, col in [(0.62, "FALLA", "#E53935"), (1.15, "CRITICO", "#FF9800"),
                           (1.38, "MARGINAL", "#FFEB3B"), (1.78, "ESTABLE", "#66BB6A")]:
        ax.text(p.dias_total * 0.98, y_t, etq, color=col, fontsize=8.5,
                ha="right", fontweight="bold", alpha=0.85)

    ax.set_xlabel("Tiempo (dias)")
    ax.set_ylabel("Factor de Seguridad FS")
    ax.set_title("(c) FS -- Metodo Bishop Simplificado (Mohr-Coulomb)", fontweight="bold")
    ax.set_ylim(0.3, 3.2)
    ax.set_xlim(0, p.dias_total)
    ax.legend(loc="upper right", framealpha=0.3, facecolor="#0D1117",
              edgecolor="#30363D", labelcolor="#C9D1D9")

    # (d) Volumen bombeado y reduccion de presion
    ax  = axes[3]
    Q_dia   = p.Q_pozo * p.n_pozos * 86400.0
    mask_m  = t_dias >= p.dias_inicio_mitiga
    vol_acum = np.zeros(len(t_dias))
    vol_acum[mask_m] = np.cumsum(np.ones(mask_m.sum()) * Q_dia)
    dh      = np.maximum(h_base - h_mitiga, 0.0)
    red_pr  = dh * p.gamma_w / 1000.0

    ax.fill_between(t_dias, vol_acum / 1000.0, alpha=0.35, color=CM)
    ax.plot(t_dias, vol_acum / 1000.0, color=CM, lw=2.5,
            label=f"Vol. bombeado ({p.n_pozos} pozos)")
    ax.axvline(p.dias_inicio_mitiga, color=CD, lw=1.5, ls="--", alpha=0.85)
    ax.set_xlabel("Tiempo (dias)")
    ax.set_ylabel("Volumen bombeado acumulado (x10^3 m3)", color=CM)
    ax.tick_params(axis="y", colors=CM)
    ax.set_title("(d) Volumen Rescatado y Reduccion de Presion", fontweight="bold")

    ax2 = ax.twinx()
    ax2.plot(t_dias, red_pr, color="#FFD54F", lw=2.3, ls="--",
             label="Reduccion presion poro (kPa)")
    ax2.set_ylabel("Reduccion presion de poro (kPa)", color="#FFD54F")
    ax2.tick_params(axis="y", colors="#FFD54F")
    ax2.set_facecolor("#161B22")
    for sp in ax2.spines.values():
        sp.set_edgecolor("#30363D")

    l1, lb1 = ax.get_legend_handles_labels()
    l2, lb2 = ax2.get_legend_handles_labels()
    ax.legend(l1 + l2, lb1 + lb2, loc="upper left", framealpha=0.3,
              facecolor="#0D1117", edgecolor="#30363D", labelcolor="#C9D1D9")

    fig.suptitle(
        "MODELO HIDROGEOLOGICO DE LADERA URBANA -- ACUIFERO TIJUANA 0201\n"
        "AquaResiliencia: Micropozos vs. Saturacion Progresiva (Fugas CESPT ~24%)",
        fontsize=13.5, fontweight="bold", color="#E6EDF3", y=0.97
    )
    plt.savefig("fig1_series_temporales.png", dpi=300, bbox_inches="tight",
                facecolor="#0D1117")
    print("  -> fig1_series_temporales.png guardada")
    plt.clf(); plt.close()


# ==============================================================================
#  BLOQUE 6 -- FIGURA 2: PERFIL GEOLOGICO 2D DE LA LADERA
# ==============================================================================

def figura_perfil_geologico(params, h_base_365, h_mitiga_365):
    """
    Perfil geologico 2D comparando los dos escenarios al Dia 365.
    Incluye estratigrafia, NAF, superficie de falla circular,
    flechas de fugas, micropozos y valor de FS en cada panel.
    """
    p = params
    H = p.H_talud
    L = p.L_talud

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(18, 8), sharey=True)
    fig.patch.set_facecolor("#0D1117")

    casos = [
        (ax1, "ESCENARIO BASE (Dia 365) -- Sin Intervencion",
         h_base_365, "#FF6B6B", False),
        (ax2, "ESCENARIO AQUARESILIENCIA (Dia 365) -- Micropozos Activos",
         h_mitiga_365, "#4DD0E1", True),
    ]

    for ax, titulo, h_f, col_agua, es_mitig in casos:
        ax.set_facecolor("#0D1117")
        ax.set_title(titulo, color="#E6EDF3", fontsize=11, fontweight="bold", pad=8)
        ax.tick_params(colors="#C9D1D9")
        ax.xaxis.label.set_color("#C9D1D9")
        ax.yaxis.label.set_color("#C9D1D9")
        for sp in ax.spines.values():
            sp.set_edgecolor("#30363D")
        ax.grid(True, color="#21262D", lw=0.6)

        x_perf = np.linspace(0, L * 1.3, 300)
        z_perf = np.where(x_perf <= L,
                          np.maximum(H - x_perf * np.tan(p.beta), 0.0), 0.0)

        # Fm. Otay (base impermeable)
        ax.fill_between(x_perf, -3.0, 0.0,
                        color="#3E2723", alpha=0.95, zorder=1,
                        label="Fm. Otay -- Acuicludo (K<1e-7 m/s)")

        # Fm. San Diego / Rosarito Beach (capa intermedia)
        z_aluv_piso = np.maximum(z_perf - p.H_aluvion, 0.0)
        ax.fill_between(x_perf, 0.0, z_aluv_piso,
                        color="#6D4C41", alpha=0.90, zorder=2,
                        label="Fm. San Diego/Rosarito (K~5e-6 m/s)")

        # Aluv. Cuaternario (capa permeable superior)
        ax.fill_between(x_perf, z_aluv_piso, z_perf,
                        color="#D4A96A", alpha=0.90, zorder=3,
                        label="Aluv. Qal (K=1.5e-4 m/s)")

        # Nivel freatico
        z_fre = np.where(x_perf <= L,
                         np.minimum(h_f, np.maximum(z_perf - 0.02, 0.0)),
                         np.nan)
        ax.plot(x_perf, z_fre, color=col_agua, lw=2.8, ls="--",
                zorder=8, label=f"NAF = {h_f:.2f} m")
        ax.fill_between(x_perf, 0.0, z_fre,
                        where=~np.isnan(z_fre),
                        color=col_agua, alpha=0.22, zorder=7)

        # Perfil del terreno
        ax.plot(x_perf, z_perf, color="#8D6E63", lw=3.0, zorder=10)
        ax.fill_between(x_perf, z_perf, H * 1.08, color="#0D1117", zorder=9)

        # Superficie de falla circular (Bishop)
        R_f   = H / np.sin(p.beta) * 1.2
        xc    = L * 0.5
        zc    = h_f + R_f
        th    = np.linspace(np.pi * 0.93, np.pi * 1.55, 150)
        xc_a  = xc + R_f * np.cos(th)
        zc_a  = zc + R_f * np.sin(th)
        mk    = (xc_a >= 0) & (xc_a <= L * 1.18) & (zc_a >= -0.3) & (zc_a <= H)
        if mk.sum() > 2:
            cf = "#FF5252" if not es_mitig else "#69F0AE"
            ax.plot(xc_a[mk], zc_a[mk], color=cf, lw=2.3, ls="-.",
                    zorder=11, label="Superficie de falla (Bishop)")

        # Flechas de fugas CESPT
        for xf in np.linspace(0.12 * L, 0.72 * L, 4):
            zf = H - xf * np.tan(p.beta)
            if zf > 0.8:
                ax.annotate("", xy=(xf, zf - 1.0), xytext=(xf, zf + 0.25),
                            zorder=12,
                            arrowprops=dict(arrowstyle="->", color="#FF8A65",
                                           lw=2.2, mutation_scale=16))
        ax.text(0.10 * L, H * 1.03, "Fugas CESPT\n(~24% perdidas)",
                color="#FF8A65", fontsize=8.5, fontweight="bold", zorder=13)

        # Micropozos AquaResiliencia (solo escenario mitigacion)
        if es_mitig:
            pos_pz = np.linspace(0.08 * L, 0.65 * L, p.n_pozos)
            for xpz in pos_pz:
                zpz_s = max(H - xpz * np.tan(p.beta), 0.0)
                zpz_f = max(zpz_s - p.prof_pozo, 0.0)
                ax.plot([xpz, xpz], [zpz_f, zpz_s],
                        color="#00E5FF", lw=5, zorder=14, solid_capstyle="round")
                ax.annotate("", xy=(xpz - 0.35, zpz_f + 0.7),
                            xytext=(xpz, zpz_f + 0.35),
                            arrowprops=dict(arrowstyle="->", color="#00E5FF",
                                           lw=1.8, mutation_scale=12), zorder=15)
            ax.text(pos_pz[0] - 0.4, h_f * 0.5 + 0.3,
                    f"{p.n_pozos} micropozos\n{p.Q_pozo*1000:.1f} L/s c/u",
                    color="#00E5FF", fontsize=8.5, fontweight="bold", zorder=16)

        # FS en esquina inferior derecha
        fs_v = factor_seguridad_bishop(h_f, p)
        cfs  = ("#4CAF50" if fs_v >= 1.5 else
                "#FF9800" if fs_v >= 1.0 else "#E53935")
        ax.text(0.97, 0.04, f"FS = {fs_v:.3f}",
                transform=ax.transAxes, color=cfs, fontsize=17,
                fontweight="bold", ha="right", va="bottom",
                bbox=dict(boxstyle="round,pad=0.45", facecolor="#161B22",
                          edgecolor=cfs, alpha=0.92))

        ax.set_xlabel("Distancia horizontal (m)")
        ax.set_ylabel("Cota (m s.n.r.)")
        ax.set_ylim(-1.2, H * 1.1)
        ax.set_xlim(-0.3, L * 1.3)
        ax.legend(loc="upper right", framealpha=0.35, facecolor="#0D1117",
                  edgecolor="#30363D", labelcolor="#C9D1D9", fontsize=8)

    fig.suptitle(
        "PERFIL GEOLOGICO 2D -- LADERA URBANA TIPO CANON TIJUANA\n"
        "Comparacion de Escenarios al Dia 365 del Ano Hidrologico",
        fontsize=13, fontweight="bold", color="#E6EDF3", y=0.995
    )
    plt.tight_layout(rect=[0, 0, 1, 0.96])
    plt.savefig("fig2_perfil_geologico.png", dpi=300, bbox_inches="tight",
                facecolor="#0D1117")
    print("  -> fig2_perfil_geologico.png guardada")
    plt.clf(); plt.close()


# ==============================================================================
#  BLOQUE 7 -- FIGURA 3: ANALISIS DE SENSIBILIDAD DEL FS
# ==============================================================================

def figura_sensibilidad(params):
    """
    3 paneles de sensibilidad:
      (a) FS vs. NAF (h)   -- para los parametros calibrados del Acuifero 0201
      (b) FS vs. Angulo    -- para distintos NAF
      (c) FS vs. Cohesion  -- para distintos NAF
    """
    p   = params
    fig, axes = plt.subplots(1, 3, figsize=(18, 7))
    fig.patch.set_facecolor("#0D1117")

    for ax in axes:
        _estilo_ax_oscuro(ax)
        ax.axhline(1.0, color="#E53935", lw=1.8, ls="--", alpha=0.9)
        ax.axhline(1.5, color="#4CAF50", lw=1.3, ls=":",  alpha=0.7)
        ax.axhspan(0,   1.0, alpha=0.13, color="#E53935", zorder=0)
        ax.axhspan(1.5, 4.0, alpha=0.07, color="#4CAF50", zorder=0)

    # (a) FS vs. h
    h_rng = np.linspace(0.05, p.H_aluvion, 60)
    fs_h  = [factor_seguridad_bishop(hh, p) for hh in h_rng]

    axes[0].plot(h_rng, fs_h, color="#4DD0E1", lw=3.0, label="Param. calibrados")
    axes[0].fill_between(h_rng, fs_h, 1.0, where=np.array(fs_h) < 1.0,
                         alpha=0.25, color="#E53935")
    axes[0].fill_between(h_rng, fs_h, 1.5, where=np.array(fs_h) > 1.5,
                         alpha=0.15, color="#4CAF50")

    h_b365 = p.H_aluvion * 0.95
    h_m365 = p.H_aluvion * 0.22
    fs_b365 = factor_seguridad_bishop(h_b365, p)
    fs_m365 = factor_seguridad_bishop(h_m365, p)
    axes[0].scatter([h_b365], [fs_b365], s=130, color="#FF6B6B", zorder=10,
                    label=f"Base D365 (FS={fs_b365:.2f})")
    axes[0].scatter([h_m365], [fs_m365], s=130, color="#69F0AE", zorder=10,
                    label=f"AquaRes D365 (FS={fs_m365:.2f})")
    axes[0].set_xlabel("Nivel freatico h (m sobre base aluvial)")
    axes[0].set_ylabel("Factor de Seguridad FS")
    axes[0].set_title("(a) FS vs. Nivel Freatico (NAF)\nParam. calibrados Acuifero 0201",
                      fontweight="bold")
    axes[0].set_ylim(0.3, 3.5)
    axes[0].legend(framealpha=0.3, facecolor="#0D1117", edgecolor="#30363D",
                   labelcolor="#C9D1D9")

    # (b) FS vs. angulo del talud
    beta_rng = np.linspace(15, 55, 50)
    h_vals   = [0.5, 1.5, 2.5, 4.0, 5.5]
    cols_b   = ["#69F0AE", "#4DD0E1", "#FFCA28", "#FF8A65", "#FF5252"]
    for hv, col in zip(h_vals, cols_b):
        pt = ParametrosAcuifero()
        fs_bv = []
        for bd in beta_rng:
            pt.beta    = np.radians(bd)
            pt.beta_deg = bd
            pt.L_talud = pt.H_talud / np.tan(pt.beta)
            fs_bv.append(factor_seguridad_bishop(hv, pt))
        axes[1].plot(beta_rng, fs_bv, color=col, lw=2.2, label=f"h = {hv} m")
    axes[1].axvline(p.beta_deg, color="#FFD54F", lw=1.5, ls="--", alpha=0.85)
    axes[1].text(p.beta_deg + 0.5, 3.15, f"beta={p.beta_deg:.0f}", color="#FFD54F", fontsize=8)
    axes[1].set_xlabel("Angulo del talud (grados)")
    axes[1].set_ylabel("Factor de Seguridad FS")
    axes[1].set_title("(b) FS vs. Angulo de la Ladera\nSensibilidad a beta (distintos NAF)",
                      fontweight="bold")
    axes[1].set_ylim(0.3, 3.5)
    axes[1].legend(framealpha=0.3, facecolor="#0D1117", edgecolor="#30363D",
                   labelcolor="#C9D1D9")

    # (c) FS vs. cohesion c'
    c_rng  = np.linspace(2.0, 45.0, 60) * 1000.0
    h_vals2 = [1.0, 3.0, 5.5]
    cols_c  = ["#69F0AE", "#4DD0E1", "#FF8A65"]
    for hv, col in zip(h_vals2, cols_c):
        pt2 = ParametrosAcuifero()
        fs_cv = []
        for cv in c_rng:
            pt2.c_aluvion = cv
            fs_cv.append(factor_seguridad_bishop(hv, pt2))
        axes[2].plot(c_rng / 1000.0, fs_cv, color=col, lw=2.2, label=f"h = {hv} m")
    axes[2].axvline(p.c_aluvion / 1000.0, color="#FFD54F", lw=1.5, ls="--", alpha=0.85)
    axes[2].text(p.c_aluvion / 1000.0 + 0.5, 3.15,
                 f"c_prima={p.c_aluvion/1000:.0f}kPa", color="#FFD54F", fontsize=8)
    axes[2].set_xlabel("Cohesion efectiva c_prima (kPa)")
    axes[2].set_ylabel("Factor de Seguridad FS")
    axes[2].set_title("(c) FS vs. Cohesion Efectiva c_prima\nSensibilidad (distintos NAF)",
                      fontweight="bold")
    axes[2].set_ylim(0.3, 3.5)
    axes[2].legend(framealpha=0.3, facecolor="#0D1117", edgecolor="#30363D",
                   labelcolor="#C9D1D9")

    fig.suptitle(
        "ANALISIS DE SENSIBILIDAD -- FACTOR DE SEGURIDAD GEOTECNICO\n"
        "Ladera Urbana Tijuana | Aluv. Qal sobre Fm. Otay (Acuicludo)",
        fontsize=13, fontweight="bold", color="#E6EDF3", y=1.01
    )
    plt.tight_layout()
    plt.savefig("fig3_sensibilidad_FS.png", dpi=300, bbox_inches="tight",
                facecolor="#0D1117")
    print("  -> fig3_sensibilidad_FS.png guardada")
    plt.clf(); plt.close()


# ==============================================================================
#  BLOQUE 8 -- FIGURA 4: DASHBOARD EJECUTIVO (VENTURES HACK 2026)
# ==============================================================================

def figura_dashboard_ejecutivo(t_dias, h_base, h_mitiga, fs_base, fs_mitiga, params):
    """
    Panel ejecutivo con 6 zonas:
      - FS comparativo (panel grande)
      - NAF compacto
      - Metricas clave
      - Indicadores circulares de estabilidad (x2)
      - Analisis costo-beneficio
    """
    p   = params
    CB  = "#F87171"
    CM  = "#34D399"
    CY  = "#FCD34D"

    fig = plt.figure(figsize=(18, 13))
    fig.patch.set_facecolor("#0A0E1A")
    gs  = GridSpec(3, 3, figure=fig, hspace=0.50, wspace=0.38,
                   left=0.06, right=0.97, top=0.91, bottom=0.05)

    ax_fs   = fig.add_subplot(gs[0, :2])
    ax_naf  = fig.add_subplot(gs[0, 2])
    ax_met  = fig.add_subplot(gs[1, :])
    ax_p1   = fig.add_subplot(gs[2, 0])
    ax_p2   = fig.add_subplot(gs[2, 1])
    ax_roi  = fig.add_subplot(gs[2, 2])

    def estilizar_panel(ax):
        ax.set_facecolor("#0F1629")
        for sp in ax.spines.values():
            sp.set_edgecolor("#1E2A4A")
        ax.tick_params(colors="#94A3B8")
        ax.xaxis.label.set_color("#94A3B8")
        ax.yaxis.label.set_color("#94A3B8")
        ax.title.set_color("#E2E8F0")
        ax.grid(True, color="#1E2A4A", lw=0.55)

    for ax in [ax_fs, ax_naf]:
        estilizar_panel(ax)

    # FS principal
    ax_fs.axhspan(0,   1.0, alpha=0.20, color="#DC2626", zorder=0)
    ax_fs.axhspan(1.0, 1.3, alpha=0.13, color="#EA580C", zorder=0)
    ax_fs.axhspan(1.3, 1.5, alpha=0.09, color="#CA8A04", zorder=0)
    ax_fs.axhspan(1.5, 4.0, alpha=0.08, color="#16A34A", zorder=0)
    ax_fs.plot(t_dias, fs_base,   color=CB, lw=3.2, label="Escenario Base",    zorder=5)
    ax_fs.plot(t_dias, fs_mitiga, color=CM, lw=3.2, label="AquaResiliencia",   zorder=5)
    ax_fs.axhline(1.0, color="#DC2626", lw=2.0, ls="--", alpha=0.9)
    ax_fs.axhline(1.5, color="#4ADE80", lw=1.5, ls=":",  alpha=0.7)
    ax_fs.axvline(p.dias_inicio_mitiga, color=CY, lw=2.0, ls="--", alpha=0.85)

    idx_f = np.where(fs_base < 1.0)[0]
    if len(idx_f) > 0:
        tf = t_dias[idx_f[0]]
        ax_fs.annotate(f"FALLA\nDia {tf:.0f}",
                       xy=(tf, 1.0), xytext=(max(tf - 60, 10), 0.50),
                       color=CB, fontsize=11, fontweight="bold",
                       arrowprops=dict(arrowstyle="->", color=CB, lw=2.0))

    ax_fs.text(p.dias_inicio_mitiga + 3, 0.36,
               f"Dia {p.dias_inicio_mitiga}: Micropozos ACTIVOS",
               color=CY, fontsize=9, fontweight="bold")
    for y_t, etq, col in [(0.60, "FALLA", "#DC2626"), (1.15, "CRITICO", "#EA580C"),
                           (1.38, "MARGINAL", "#CA8A04"), (1.8, "ESTABLE", "#16A34A")]:
        ax_fs.text(p.dias_total * 0.99, y_t, etq, color=col, fontsize=9,
                   ha="right", fontweight="bold")
    ax_fs.set_xlabel("Tiempo (dias del ano hidrologico)")
    ax_fs.set_ylabel("Factor de Seguridad FS")
    ax_fs.set_title("FACTOR DE SEGURIDAD GEOTECNICO -- Bishop Simplificado (Mohr-Coulomb)",
                    fontweight="bold", fontsize=12)
    ax_fs.set_ylim(0.3, 3.2)
    ax_fs.set_xlim(0, p.dias_total)
    ax_fs.legend(loc="upper right", framealpha=0.4, facecolor="#0A0E1A",
                 edgecolor="#1E2A4A", labelcolor="#E2E8F0", fontsize=10)

    # NAF compacto
    ax_naf.plot(t_dias, h_base,   color=CB, lw=2.5)
    ax_naf.plot(t_dias, h_mitiga, color=CM, lw=2.5)
    ax_naf.fill_between(t_dias, h_base, h_mitiga,
                        where=h_base > h_mitiga, alpha=0.2, color=CM)
    ax_naf.axhline(p.H_aluvion, color=CB, lw=1.2, ls=":", alpha=0.6)
    ax_naf.axvline(p.dias_inicio_mitiga, color=CY, lw=1.5, ls="--", alpha=0.8)
    ax_naf.set_xlabel("Tiempo (dias)")
    ax_naf.set_ylabel("NAF (m)")
    ax_naf.set_title("Nivel Freatico\n(NAF)", fontweight="bold")
    abt_max = (h_base - h_mitiga).max()
    ax_naf.text(0.95, 0.97, f"Abt. max:\n{abt_max:.2f} m",
                transform=ax_naf.transAxes, color=CM, fontsize=9,
                ha="right", va="top", fontweight="bold")

    # Metricas clave
    ax_met.set_facecolor("#0F1629")
    ax_met.axis("off")
    for sp in ax_met.spines.values():
        sp.set_edgecolor("#1E2A4A")
    ax_met.set_title("METRICAS CLAVE DEL MODELO -- AquaResiliencia vs. Escenario Base",
                     color="#E2E8F0", fontsize=11.5, fontweight="bold", pad=6)

    hb = h_base[-1];  hm = h_mitiga[-1]
    fb = fs_base[-1]; fm = fs_mitiga[-1]
    ub = p.gamma_w * hb / 1000.0
    um = p.gamma_w * hm / 1000.0
    abt365 = hb - hm
    red_u  = (ub - um) / max(ub, 0.01) * 100.0
    vb     = p.Q_pozo * p.n_pozos * (p.dias_total - p.dias_inicio_mitiga) * 86400.0

    metricas = [
        ("FS Base\n(Dia 365)",      f"{fb:.3f}",   "#DC2626",
         "FALLA" if fb < 1.0 else "Inestable", "#DC2626"),
        ("FS Mitigado\n(Dia 365)",  f"{fm:.3f}",   "#22C55E",
         "ESTABLE" if fm >= 1.5 else "Marginal", "#22C55E"),
        ("NAF Base",                f"{hb:.2f} m",  "#F87171",
         f"{hb/p.H_aluvion*100:.0f}% saturado", "#F87171"),
        ("NAF Mitigado",            f"{hm:.2f} m",  "#34D399",
         f"Abt: {abt365:.2f} m", "#34D399"),
        ("Presion Poro\nBase",      f"{ub:.1f} kPa","#FB923C",
         f"Mit: {um:.1f} kPa", "#34D399"),
        ("Reduccion\nPresion Poro", f"{red_u:.0f}%","#34D399",
         f"Vol: {vb:.0f} m3", "#60A5FA"),
    ]

    n_m  = len(metricas)
    x_ms = np.linspace(0.04, 0.96, n_m)
    for i, (lbl, val, cv, sub, cs) in enumerate(metricas):
        xp = x_ms[i]
        ax_met.text(xp, 0.82, lbl, transform=ax_met.transAxes,
                    color="#94A3B8", fontsize=9, ha="center", va="top")
        ax_met.text(xp, 0.52, val, transform=ax_met.transAxes,
                    color=cv, fontsize=17, ha="center", va="top", fontweight="bold")
        ax_met.text(xp, 0.16, sub, transform=ax_met.transAxes,
                    color=cs, fontsize=8.8, ha="center", va="top", fontstyle="italic")
        if i < n_m - 1:
            ax_met.axvline((x_ms[i] + x_ms[i + 1]) / 2,
                           color="#1E2A4A", lw=1, alpha=0.8)
    ax_met.set_xlim(0, 1)
    ax_met.set_ylim(0, 1)

    # Indicadores circulares de estabilidad
    for ax_pie, lbl_p, fs_v in [
        (ax_p1, "ESCENARIO BASE\n(Dia 365)", fb),
        (ax_p2, "AQUARESILIENCIA\n(Dia 365)", fm),
    ]:
        ax_pie.set_facecolor("#0F1629")
        frac   = min(fs_v / 2.0, 1.0)
        cfs    = ("#22C55E" if fs_v >= 1.5 else
                  "#F87171" if fs_v < 1.0 else "#FB923C")
        ax_pie.pie([frac, max(1.0 - frac, 0.0)],
                   colors=[cfs, "#2D3748"],
                   wedgeprops=dict(width=0.55, edgecolor="#0A0E1A", linewidth=2),
                   startangle=90, counterclock=False)
        ax_pie.text(0, 0, f"FS\n{fs_v:.2f}", ha="center", va="center",
                    fontsize=16, fontweight="bold", color=cfs)
        ax_pie.set_title(lbl_p, color="#E2E8F0", fontsize=9.5, fontweight="bold")

    # Costo-Beneficio
    ax_roi.set_facecolor("#0F1629")
    ax_roi.axis("off")
    ax_roi.set_title("Costo-Beneficio Estimado", color="#E2E8F0",
                     fontsize=9.5, fontweight="bold")
    costo_pz  = 800
    costo_tot = costo_pz * p.n_pozos
    op_anual  = 200 * p.n_pozos
    beneficio = 2_500_000
    roi_neto  = beneficio - (costo_tot + op_anual)
    bc        = beneficio / (costo_tot + op_anual)
    lineas = [
        ("INVERSION", "", "#94A3B8"),
        (f"  {p.n_pozos} micropozos @ USD ${costo_pz} c/u",
         f"USD ${costo_tot:,.0f}", "#60A5FA"),
        ("  Operacion anual estimada",
         f"USD ${op_anual:,.0f}/ano", "#60A5FA"),
        ("", "", "#94A3B8"),
        ("BENEFICIO EVITADO", "", "#94A3B8"),
        ("  Costo prom. deslizamiento\n  urbano (Tijuana, CENAPRED)",
         f"USD ${beneficio:,.0f}", "#34D399"),
        ("", "", "#94A3B8"),
        ("ROI NETO ESTIMADO",  f"USD ${roi_neto:,.0f}", "#FCD34D"),
        ("RELACION B/C",       f"{bc:.0f}x retorno",     "#FCD34D"),
    ]
    yp = 0.95
    for lin, val, col in lineas:
        ax_roi.text(0.02, yp, lin, transform=ax_roi.transAxes,
                    color=col, fontsize=8.0, va="top")
        if val:
            ax_roi.text(0.98, yp, val, transform=ax_roi.transAxes,
                        color=col, fontsize=9.0, va="top", ha="right", fontweight="bold")
        yp -= 0.10 if "\n" not in lin else 0.13
    ax_roi.set_xlim(0, 1)
    ax_roi.set_ylim(0, 1)

    fig.suptitle(
        "AQUARESILIENCIA TIJUANA -- TABLERO EJECUTIVO\n"
        "Modelo Hidro-Geotecnico Computacional | Acuifero 0201 | Ventures Hack 2026",
        fontsize=14, fontweight="bold", color="#E2E8F0", y=0.973
    )
    plt.savefig("fig4_dashboard_ejecutivo.png", dpi=300, bbox_inches="tight",
                facecolor="#0A0E1A")
    print("  -> fig4_dashboard_ejecutivo.png guardada")
    plt.clf(); plt.close()


# ==============================================================================
#  BLOQUE 9 -- PUNTO DE ENTRADA PRINCIPAL
# ==============================================================================

def main():
    sep = "=" * 74
    print("\n" + sep)
    print("  AquaResiliencia Tijuana -- Modelo Hidrogeologico Computacional")
    print("  Acuifero CONAGUA 0201 | Canones Urbanos | Ventures Hack 2026")
    print(sep)

    print("\n  [1/5] Inicializando parametros fisicos (DR_0201 / SGM / CICESE)...")
    p = ParametrosAcuifero()
    print(f"        H_talud  = {p.H_talud} m | beta = {p.beta_deg} deg | L_base = {p.L_talud:.1f} m")
    print(f"        K_aluv   = {p.K_aluvion:.2e} m/s | Sy = {p.Sy_aluvion} | n = {p.n_aluvion}")
    print(f"        phi_prima = {np.degrees(p.phi_aluvion):.0f} deg | c_prima = {p.c_aluvion/1000:.1f} kPa")
    print(f"        K_otay   = {p.K_otay:.2e} m/s (acuicludo Fm. Otay)")

    print("\n  [2/5] Simulando evolucion del NAF (365 dias, dt=1 dia)...")
    t_dias, h_base, h_mitiga, precip_mm = simular_nivel_freatico(p)
    print(f"        NAF base (Dia 365):     {h_base[-1]:.3f} m")
    print(f"        NAF mitigado (Dia 365): {h_mitiga[-1]:.3f} m")
    print(f"        Abatimiento logrado:    {h_base[-1] - h_mitiga[-1]:.3f} m")

    print("\n  [3/5] Calculando FS -- Metodo de Bishop Simplificado (20 dovelas)...")
    fs_base, fs_mitiga = calcular_series_fs(t_dias, h_base, h_mitiga, p)
    print(f"        FS base (Dia 365):     {fs_base[-1]:.4f} "
          f"({'FALLA' if fs_base[-1] < 1.0 else 'Inestable'})")
    print(f"        FS mitigado (Dia 365): {fs_mitiga[-1]:.4f} "
          f"({'ESTABLE' if fs_mitiga[-1] >= 1.5 else 'Marginal'})")

    print("\n  [4/5] Reporte numerico detallado:")
    imprimir_reporte(t_dias, h_base, h_mitiga, fs_base, fs_mitiga, p)

    print("  [5/5] Generando 4 figuras (PNG 300 dpi)...")
    print("\n  -> Figura 1: Series temporales (NAF, presion poro, FS, volumen)...")
    figura_series_temporales(t_dias, h_base, h_mitiga, fs_base, fs_mitiga, p, precip_mm)

    print("\n  -> Figura 2: Perfil geologico 2D de la ladera...")
    figura_perfil_geologico(p, h_base[-1], h_mitiga[-1])

    print("\n  -> Figura 3: Analisis de sensibilidad del FS...")
    figura_sensibilidad(p)

    print("\n  -> Figura 4: Dashboard ejecutivo (Ventures Hack 2026)...")
    figura_dashboard_ejecutivo(t_dias, h_base, h_mitiga, fs_base, fs_mitiga, p)

    print("\n  Simulacion completa. Archivos generados:")
    for fn in ["fig1_series_temporales.png", "fig2_perfil_geologico.png",
               "fig3_sensibilidad_FS.png", "fig4_dashboard_ejecutivo.png"]:
        print(f"    - {fn}")

    print("\n  Referencias cientificas:")
    print("  [1] CONAGUA (2023). Actualizacion Disponibilidad Media Anual")
    print("      Acuifero Tijuana (0201). DOF Mexico.")
    print("  [2] SGM/INEGI (2020). Cartografia Geologico-Minera Hoja Tijuana H11-1.")
    print("  [3] Bishop, A.W. (1955). The use of the slip circle in the stability")
    print("      analysis of slopes. Geotechnique, 5(1), 7-17.")
    print("  [4] Mohr, O. (1900). Welche Umstaende bedingen die Elastizitaetsgrenze.")
    print("  [5] Gudino Elizondo, N. (2018). Tesis Doctoral CICESE, Ensenada BC.")
    print("  [6] Wakida, F.T. et al. (2008). Nitrate contamination in groundwater")
    print("      in the alluvial aquifer of the Tijuana River. Hydrol. Proc. 22(14).")
    print()


if __name__ == "__main__":
    main()
