#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Generador e Impresor ESC/POS (80mm) - Versión Alta Legibilidad (Sin Fondo Negro)
+ Diagramas Laterales Izquierdo y Derecho de los 38 Pines del NodeMCU ESP32S.
Gabinete "Agua de Gia" - Nodo 2 (NodeMCU ESP32S 38 Pines)
"""

import ctypes
from ctypes import wintypes
import subprocess
import sys

# ==============================================================================
# COMANDOS NATIVOS ESC/POS (MODO CLARO SIN FONDO NEGRO INVERTIDO)
# ==============================================================================
ESC_INIT        = b"\x1B\x40"          # Inicializar impresora
ALIGN_LEFT      = b"\x1B\x61\x00"      # Alineación izquierda
ALIGN_CENTER    = b"\x1B\x61\x01"      # Alineación centrada
BOLD_ON         = b"\x1B\x45\x01"      # Negrita ON
BOLD_OFF        = b"\x1B\x45\x00"      # Negrita OFF
INVERT_OFF      = b"\x1D\x42\x00"      # Asegurar Blanco sobre Negro APAGADO
UNDERLINE_ON    = b"\x1B\x2D\x02"      # Subrayado grueso ON
UNDERLINE_OFF   = b"\x1B\x2D\x00"      # Subrayado OFF
SIZE_NORMAL     = b"\x1D\x21\x00"      # Tamaño normal (1x1)
SIZE_TALL       = b"\x1D\x21\x01"      # Doble alto (1x2) - Muy legible para títulos
SIZE_WIDE       = b"\x1D\x21\x10"      # Doble ancho (2x1)
SIZE_DOUBLE     = b"\x1D\x21\x11"      # Doble ancho y doble alto (2x2) para ID y GPIO
PARTIAL_CUT     = b"\x1D\x56\x42\x00"  # Corte parcial de guillotina con avance

CUT_LINE_STR    = "--- [ CORTAR AQUI ] ------------------------\n\n\n"
BOX_LINE_STR    = "+------------------------------------------+\n"
EQ_LINE_STR     = "============================================\n"


def b_text(txt: str) -> bytes:
    """Codifica texto ASCII/CP437 limpio para impresora térmica POS."""
    return txt.encode("cp437", errors="replace")


def format_block_banner(title: str) -> bytes:
    """Encabezado principal de cada bloque (Claro, en Doble Alto y Negrita, sin fondo negro)."""
    out = bytearray()
    out += INVERT_OFF
    out += ALIGN_CENTER
    out += BOLD_ON
    out += b_text(EQ_LINE_STR)
    out += SIZE_TALL
    out += b_text(f"{title}\n")
    out += SIZE_NORMAL
    out += b_text(EQ_LINE_STR + "\n")
    out += BOLD_OFF
    out += ALIGN_LEFT
    return bytes(out)


def format_label(header: str, big_id_gpio: str, details: list[str]) -> bytes:
    """
    Formatea una etiqueta individual con máxima claridad sobre fondo blanco:
    - Marco superior + Encabezado en Doble Alto (\x1D\x21\x01) y Negrita (SIN fondo negro)
    - ID y GPIO en Doble Tamaño (\x1D\x21\x11)
    - Detalles técnicos bien espaciados
    - Línea de corte con tijera y 2 líneas de avance
    """
    out = bytearray()
    out += INVERT_OFF
    out += ALIGN_CENTER
    out += BOLD_ON
    out += SIZE_NORMAL
    out += b_text(BOX_LINE_STR)
    out += SIZE_TALL
    out += b_text(f"{header}\n")
    out += SIZE_NORMAL
    out += b_text(BOX_LINE_STR)

    # ID y GPIO en Doble Tamaño (\x1D\x21\x11) sobre fondo blanco
    if big_id_gpio:
        out += SIZE_DOUBLE
        out += b_text(f"{big_id_gpio}\n")
        out += SIZE_NORMAL
        out += b_text("--------------------------------------------\n")

    # Detalles técnicos a la izquierda
    out += ALIGN_LEFT
    out += BOLD_ON
    for line in details:
        out += b_text(f" * {line}\n")
    out += BOLD_OFF

    # Línea de corte con tijera + 2 líneas de avance
    out += b_text("\n" + CUT_LINE_STR)
    return bytes(out)


def format_cable_flag(flag_text: str) -> bytes:
    """
    Banderilla compacta de 2 líneas (doble impresión espejada para doblar sobre el cable)
    SIN fondo negro para lectura perfecta.
    """
    out = bytearray()
    out += INVERT_OFF
    out += ALIGN_CENTER
    out += BOLD_ON
    out += SIZE_TALL
    out += b_text(f"{flag_text}\n")
    out += b_text(f"{flag_text}\n")
    out += SIZE_NORMAL
    out += BOLD_OFF
    out += ALIGN_LEFT
    out += b_text(CUT_LINE_STR)
    return bytes(out)


def build_esp32_pinout_diagrams() -> bytes:
    """
    Construye el BLOQUE 0 / ESPECIAL:
    1) Etiqueta vertical para pegar al LADO IZQUIERDO del ESP32 (Pines 1 al 19)
    2) Etiqueta vertical para pegar al LADO DERECHO del ESP32 (Pines 38 al 20)
    3) Diagrama completo cara a cara de los 38 pines del NodeMCU ESP32S
    """
    out = bytearray()
    out += ESC_INIT
    out += INVERT_OFF

    # --------------------------------------------------------------------------
    # 1. TIRA PARA PEGAR AL LADO IZQUIERDO DEL ESP32 (PINES 01 AL 19)
    # --------------------------------------------------------------------------
    out += format_block_banner("DIAGRAMA LADO IZQUIERDO ESP32 (PIN 1-19)")
    out += ALIGN_CENTER
    out += BOLD_ON
    out += SIZE_TALL
    out += b_text("<< LADO IZQUIERDO (Pines 1 al 19) <<\n")
    out += SIZE_NORMAL
    out += b_text("   [ ARRIBA: ANTENA WIFI ESP32S ]   \n")
    out += b_text(EQ_LINE_STR)
    out += ALIGN_LEFT

    left_pins = [
        ("01", "3V3 ", "Salida 3.3V (Pullup DS18B20)"),
        ("02", "EN  ", "Reset / Enable"),
        ("03", "P36 ", "VP  | DIVISOR 4:1 (3x10k+10k) VOLT/AMP"),
        ("04", "P39 ", "VN  | Libre (Solo Entrada)"),
        ("05", "P34 ", "ADC | ACS712 AC 5A (Primaria ATX)"),
        ("06", "P35 ", "ADC | CORRIENTE BOMBAS DC"),
        ("07", "P32 ", "ADC0| PRESOSTATO 100PSI (Puente P22)"),
        ("08", "P33 ", "ID3 | SSR 1 FUENTE ATX FLOC"),
        ("09", "P25 ", "ID5 | BOMBA TRASVASE (Bix#2)"),
        ("10", "P26 ", "ID4 | SSR 3 BOMBA PECERA H2"),
        ("11", "P27 ", "ID2 | SSR 2 SOL. CONO 120V"),
        ("12", "P14 ", "ID1 | BOMBA EVANS 24V (Bix#2)"),
        ("13", "P12 ", "Libre (Boot Strapping)"),
        ("14", "GND ", "STAR-GND (Tierra Comun)"),
        ("15", "P13 ", "ID9 | SOL. BYPASS RO (Bix#1)"),
        ("16", "SD2 ", "GPIO 9  (Flash - NO USAR)"),
        ("17", "SD3 ", "GPIO 10 (Flash - NO USAR)"),
        ("18", "CMD ", "GPIO 11 (Flash - NO USAR)"),
        ("19", "5V  ", "VIN | Entrada +5VSB Fuente ATX"),
    ]

    for num, pin, desc in left_pins:
        out += b_text(f"Pin {num} [{pin}] -> {desc}\n")

    out += ALIGN_CENTER
    out += b_text(EQ_LINE_STR)
    out += b_text("   [ ABAJO: CONECTOR USB TIPO-C ]   \n")
    out += BOLD_OFF
    out += ALIGN_LEFT
    out += b_text("\n" + CUT_LINE_STR)
    out += PARTIAL_CUT

    # --------------------------------------------------------------------------
    # 2. TIRA PARA PEGAR AL LADO DERECHO DEL ESP32 (PINES 38 AL 20)
    # --------------------------------------------------------------------------
    out += format_block_banner("DIAGRAMA LADO DERECHO ESP32 (PIN 38-20)")
    out += ALIGN_CENTER
    out += BOLD_ON
    out += SIZE_TALL
    out += b_text(">> LADO DERECHO (Pines 38 al 20) >>\n")
    out += SIZE_NORMAL
    out += b_text("   [ ARRIBA: ANTENA WIFI ESP32S ]   \n")
    out += b_text(EQ_LINE_STR)
    out += ALIGN_LEFT

    right_pins = [
        ("38", "GND ", "STAR-GND (Tierra Comun)"),
        ("37", "P23 ", "ID6 | BOMBA MANIFOLD + UV"),
        ("36", "P22 ", "Hi-Z| DATOS PRESOSTATO (Puente->P32)"),
        ("35", "TX0 ", "GPIO 1 (Serial USB TX)"),
        ("34", "RX0 ", "GPIO 3 (Serial USB RX)"),
        ("33", "P21 ", "IN  | PARO EMERGENCIA REBOSE"),
        ("32", "GND ", "STAR-GND (Tierra Comun)"),
        ("31", "P19 ", "IN  | NIVEL ALTO TOTEM 1"),
        ("30", "P18 ", "ISR | CAUDALIMETRO HALL"),
        ("29", "P5  ", "ID8 | SOL. PRE-RO 12V (Bix#1)"),
        ("28", "P17 ", "ID10| BUZZER ALARMA SONORA"),
        ("27", "P16 ", "PWM | SERVO VALVULA AGUJA RO"),
        ("26", "P4  ", "1-W | TEMPERATURA DS18B20"),
        ("25", "P0  ", "GPIO 0 (Boton BOOT)"),
        ("24", "P2  ", "GPIO 2 (LED Azul Placa)"),
        ("23", "P15 ", "ID7 | SOLENOIDE VIA A CASA"),
        ("22", "SD1 ", "GPIO 8 (Flash - NO USAR)"),
        ("21", "SD0 ", "GPIO 7 (Flash - NO USAR)"),
        ("20", "CLK ", "GPIO 6 (Flash - NO USAR)"),
    ]

    for num, pin, desc in right_pins:
        out += b_text(f"Pin {num} [{pin}] <- {desc}\n")

    out += ALIGN_CENTER
    out += b_text(EQ_LINE_STR)
    out += b_text("   [ ABAJO: CONECTOR USB TIPO-C ]   \n")
    out += BOLD_OFF
    out += ALIGN_LEFT
    out += b_text("\n" + CUT_LINE_STR)
    out += PARTIAL_CUT

    # --------------------------------------------------------------------------
    # 3. DIAGRAMA COMPLETO CARA A CARA (38 PINES) PARA LA TAPA DEL GABINETE
    # --------------------------------------------------------------------------
    out += format_block_banner("MAPA COMPLETO ESP32S 38 PINES (1-38)")
    out += ALIGN_CENTER
    out += BOLD_ON
    out += b_text("      [ ANTENA WIFI ARRIBA ]      \n")
    out += b_text("+------------------------------------------+\n")
    out += b_text("| IZQ (1-19)       |       DER (38-20)     |\n")
    out += b_text("+------------------+-----------------------+\n")
    out += ALIGN_LEFT

    side_by_side = [
        ("01 3V3          ", "          GND 38"),
        ("02 EN           ", "    ID6   P23 37"),
        ("03 P36 VOLT/AMP ", "   PT1->32 P22 36"),
        ("04 P39 (VN)     ", "          TX0 35"),
        ("05 P34 I_AC 5A  ", "          RX0 34"),
        ("06 P35 I_DC     ", "   REBOSE P21 33"),
        ("07 P32 PT1_ADC0 ", "          GND 32"),
        ("08 P33 ID3 ATX  ", "    TOTEM P19 31"),
        ("09 P25 ID5 TRASV", "     HALL P18 30"),
        ("10 P26 ID4 PECER", "  ID8 RO   P5 29"),
        ("11 P27 ID2 CONO ", "  BUZZER  P17 28"),
        ("12 P14 ID1 EVANS", "   SERVO  P16 27"),
        ("13 P12 LIBRE    ", "    TEMP   P4 26"),
        ("14 GND STAR-GND ", "     BOOT  P0 25"),
        ("15 P13 ID9 BYPAS", "      LED  P2 24"),
        ("16 SD2 (NO USAR)", " ID7 CASA P15 23"),
        ("17 SD3 (NO USAR)", " (NO USAR)SD1 22"),
        ("18 CMD (NO USAR)", " (NO USAR)SD0 21"),
        ("19 5V  (+5VSB)  ", " (NO USAR)CLK 20"),
    ]

    for l_col, r_col in side_by_side:
        out += b_text(f"| {l_col} | {r_col} |\n")

    out += ALIGN_CENTER
    out += b_text("+------------------+-----------------------+\n")
    out += b_text("      [ PUERTO USB-C ABAJO ]      \n\n")
    out += BOLD_OFF
    out += ALIGN_LEFT
    out += b_text(CUT_LINE_STR)
    out += PARTIAL_CUT

    # --------------------------------------------------------------------------
    # 4. DIAGRAMA ESPECIAL: PRESOSTATO 100 PSI + TANQUE 1.10m (35cm + 75cm)
    # --------------------------------------------------------------------------
    out += format_block_banner("CONEXION PRESOSTATO 100 PSI (TANQUE 1.10m)")
    out += ALIGN_LEFT
    out += BOLD_ON
    out += b_text("TRANSDUCTOR DE PRESION (0-100 PSI / 0.5-4.5V):\n")
    out += b_text("  * cable ROJO (+VCC)  --> +5V DC (Pin 19 VIN)\n")
    out += b_text("  * cable NEGRO (GND)  --> STAR-GND (Pin 14/32/38)\n")
    out += b_text("  * cable DATOS (0.5V) --> P22 + PUENTE A P32\n")
    out += b_text(EQ_LINE_STR)
    out += b_text("POR QUE EL PUENTE P22 -> P32 (Pin 36 a Pin 07):\n")
    out += b_text("  * GPIO 22 NO tiene ADC interno en el ESP32.\n")
    out += b_text("  * P22 quedo en Alta Impedancia (INPUT sin pullup)\n")
    out += b_text("    para no alterar tu cable conectado en P22.\n")
    out += b_text("  * Pon un jumper de P22 -> P32 (ADC1_CH4).\n")
    out += b_text("  * P32 usa Ganancia ADC_0db (100-950 mV) + 256x\n")
    out += b_text("    oversampling (Resolucion: 0.05 mV = ~1 mm).\n")
    out += b_text(EQ_LINE_STR)
    out += b_text("GEOMETRIA DEL TANQUE (1 BBL = 117.3 L):\n")
    out += b_text("  * Altura Total   : 110.0 cm (1.10 m)\n")
    out += b_text("  * Cono Inferior  :  35.0 cm (0 a 15.8 L)\n")
    out += b_text("  * Cilindro Recto :  75.0 cm (15.8 a 117.3 L)\n")
    out += b_text("  * Pendiente 100PSI: 0.5689 mV/cm (Span 62.6 mV)\n")
    out += b_text("  * Vacio (0 cm)   : ~500.00 mV (Boton Tarar 0cm)\n")
    out += b_text("  * Union Cono 35cm: ~519.91 mV (15.8 L)\n")
    out += b_text("  * Lleno (110 cm) : ~562.58 mV (117.3 L)\n")
    out += BOLD_OFF
    out += b_text("\n" + CUT_LINE_STR)
    out += PARTIAL_CUT

    # --------------------------------------------------------------------------
    # 5. DIAGRAMA ESPECIAL: DIVISOR 4:1 RESISTIVO (3x10k + 1x10k) EN PIN 03 (P36)
    #    MEDICIÓN DE VOLTAJE REAL DE CELDA Y ESTIMACIÓN DE AMPERAJE SIN QUEMAR SENSOR
    # --------------------------------------------------------------------------
    out += format_block_banner("DIVISOR 4:1 VOLTAJE Y AMPERAJE (PIN 03 P36)")
    out += ALIGN_LEFT
    out += BOLD_ON
    out += b_text("MEDICION DE CELDA ELECTROFLOCULACION (PIN 03 / VP):\n")
    out += b_text("  * 3 Resistencias de 10k en serie = 30k hacia +12V\n")
    out += b_text("  * 1 Resistencia de 10k hacia STAR-GND (0V)\n")
    out += b_text("  * Punto medio al Pin 03 (P36 / VP / ADC1_CH0)\n")
    out += b_text(EQ_LINE_STR)
    out += b_text("ESQUEMA DE CONEXION FISICA:\n\n")
    out += b_text("  [+12V ELECTRODO / FUENTE ATX]\n")
    out += b_text("                |\n")
    out += b_text("             [ 10k ] R1\n")
    out += b_text("                |\n")
    out += b_text("             [ 10k ] R2  --> (Total R_arriba = 30k)\n")
    out += b_text("                |\n")
    out += b_text("             [ 10k ] R3\n")
    out += b_text("                |\n")
    out += b_text("  PUNTO MEDIO --+-----------------> PIN 03 [P36 / VP]\n")
    out += b_text("                |                   (ADC 0 a 3.5V max)\n")
    out += b_text("             [ 10k ] R4 (R_abajo = 10k)\n")
    out += b_text("                |\n")
    out += b_text("         [ STAR-GND / 0V ] (Pin 14 / 32 / 38)\n\n")
    out += b_text(EQ_LINE_STR)
    out += b_text("CALCULOS Y ESCALAS ELECTRICAS:\n")
    out += b_text("  * Relacion Division : 10k / (30k + 10k) = 1/4 (Ratio 4.0)\n")
    out += b_text("  * Con 12.0V Celda   : 3.00V en Pin 03 (100% Seguro)\n")
    out += b_text("  * Con 14.0V Maximo  : 3.50V en Pin 03 (Limite Seguro)\n")
    out += b_text("  * Formula Firmware  : V_celda = V_adc_pin36 * 4.0f\n")
    out += b_text("  * Amperaje Estimado : I = (V_vacio - V_celda) / R_fuente\n")
    out += b_text("  * ACS712-05B (Pin 05): Monitorea AC 120V (0.3 - 0.9A AC)\n")
    out += b_text("    de entrada primaria ATX para doble verificacion.\n")
    out += BOLD_OFF
    out += b_text("\n" + CUT_LINE_STR)
    out += PARTIAL_CUT

    return bytes(out)


def build_all_labels_payload() -> bytes:
    """Construye el flujo completo ESC/POS: Diagramas ESP32 + 5 Bloques Claros."""
    payload = bytearray()
    payload += build_esp32_pinout_diagrams()

    # ==========================================================================
    # BLOQUE 1: FASE 1 - POTENCIA AC Y 3 SSRs (Disparo 3.2V DC)
    # ==========================================================================
    payload += format_block_banner("BLOQUE 1: FASE 1 - POTENCIA AC Y 3 SSRs")

    payload += format_label(
        "[SSR 1 - ELECTROFLOCULACION]",
        "ID: 3 | GPIO 33 (P33)",
        [
            "PIN: GPIO 33 (P33)",
            "CTRL: 3.3V DC (Pin 3+ / Pin 4 GND)",
            "CARGA: Fuente ATX 12V/15A (120V AC Primaria)",
        ],
    )

    payload += format_label(
        "[SSR 2 - SOLENOIDE CONO 1 BBL]",
        "ID: 2 | GPIO 27 (P27)",
        [
            "PIN: GPIO 27 (P27)",
            "CTRL: 3.3V DC",
            'CARGA: Enchufe AC #1 -> Valvula 3/4" 120V AC',
        ],
    )

    payload += format_label(
        "[SSR 3 - BOMBA PECERA H2]",
        "ID: 4 | GPIO 26 (P26)",
        [
            "PIN: GPIO 26 (P26)",
            "CTRL: 3.3V DC",
            "CARGA: Enchufe AC #2 -> Bomba Whirlpool 120V AC",
        ],
    )

    payload += format_label(
        "[ENCHUFE AC #1 NANO BREWER]",
        "SSR 2 -> P27 (120V AC)",
        [
            'Conectar aqui: Solenoide Llenado Cono 3/4"',
            "(120V AC) -> Viene de SSR 2 (P27)",
        ],
    )

    payload += format_label(
        "[ENCHUFE AC #2 NANO BREWER]",
        "SSR 3 -> P26 (120V AC)",
        [
            "Conectar aqui: Bomba Pecera Pulmon H2",
            "(120V AC) -> Viene de SSR 3 (P26)",
        ],
    )

    payload += PARTIAL_CUT

    # ==========================================================================
    # BLOQUE 2: FASE 2 - MODULOS BIXOLON RJ12 Y CARGAS DC
    # ==========================================================================
    payload += format_block_banner("BLOQUE 2: FASE 2 - BIXOLON RJ12 Y DC")

    payload += format_label(
        "[MODULO BIXOLON #1 - VALVULAS RO 12V]",
        "P5 (ID8) / P13 (ID9)",
        [
            "Puerto CASH RJ12",
            "Pin 2 (Negro): Pre-RO (GPIO 5 / P5)",
            "Pin 5 (Amarillo): Bypass Lavado (GPIO 13 / P13)",
            "Pin 6 (Azul): STAR-GND",
        ],
    )

    payload += format_label(
        "[MODULO BIXOLON #2 - BOMBAS DC]",
        "P25 (ID5) / P14 (ID1)",
        [
            "Puerto CASH RJ12",
            "Pin 2 (Negro): Rele 30A Bomba Trasvase (GPIO 25 / P25)",
            "Pin 5 (Amarillo): Timer/Rele Bomba Evans 24V (GPIO 14 / P14)",
            "Pin 6 (Azul): STAR-GND",
        ],
    )

    payload += format_label(
        "[RELE AUTOMOTRIZ 30A - TRASVASE]",
        "ID: 5 | GPIO 25 (P25)",
        [
            "Pin 86: +12V Camaras",
            "Pin 85: Bixolon #2 Pin 2",
            "Pin 30: +12V Fuerza",
            "Pin 87: Positivo Bomba Trasvase",
        ],
    )

    payload += format_label(
        "[RELE / TIMER BOMBA POZO EVANS]",
        "ID: 1 | GPIO 14 (P14)",
        [
            "Disparo: Bixolon #2 Pin 5",
            "Fuerza: Fuente 24V Impresora -> Bomba Evans 24V DC",
        ],
    )

    payload += PARTIAL_CUT

    # ==========================================================================
    # BLOQUE 3: ACTUADORES COMPLEMENTARIOS, AUBER Y FUENTES
    # ==========================================================================
    payload += format_block_banner("BLOQUE 3: ACTUADORES, AUBER Y FUENTES")

    payload += format_label(
        "[ID 6: BOMBA MANIFOLD + UV]",
        "ID: 6 | GPIO 23 (P23)",
        [
            "PIN: GPIO 23 (P23)",
            "Presurizacion 50 PSI + Esterilizador UV",
        ],
    )

    payload += format_label(
        "[ID 7: SOLENOIDE VIA A CASA]",
        "ID: 7 | GPIO 15 (P15)",
        [
            "PIN: GPIO 15 (P15)",
            "Barril Presurizado 60L (12V DC NC)",
        ],
    )

    payload += format_label(
        "[SERVO VALVULA AGUJA RO]",
        "PWM | GPIO 16 (P16)",
        [
            "PIN: GPIO 16 (P16) | PWM 50Hz",
            "0=Cerrada / 90=100 PSI / 180=Purga",
        ],
    )

    payload += format_label(
        "[BUZZER ALARMA]",
        "ID: 10 | GPIO 17 (P17)",
        [
            "ID: 10 | PIN: GPIO 17 (P17)",
            "Activo en HIGH (3.3V)",
        ],
    )

    payload += format_label(
        "[CONTROLADOR AUBER INSTRUMENTS]",
        "AUBER PID (24V AC/DC)",
        [
            "Alim: Pin 9-10 (24V AC/DC)",
            "Salida: Pin 6-7-8",
            "Codigos: Init=0089 / PID=0036 / Temp=0001",
        ],
    )

    payload += format_label(
        "[FUENTE ATX 12V / 15A]",
        "SSR 1 | GPIO 33 (P33)",
        [
            "Controlada por SSR 1 (P33)",
            "+12V Amarillos -> Anodo Fe",
            "GND Negros -> Catodo Inox y STAR-GND",
            "+5VSB Morado -> VIN ESP32",
        ],
    )

    payload += format_label(
        "[FUENTE 12V CAMARAS]",
        "BUS +12V DC / STAR-GND",
        [
            "Positivo comun para Solenoides 12V,",
            "Rele 30A y Bombas 12V",
            "Negativo -> STAR-GND",
        ],
    )

    payload += format_label(
        "[FUENTE 24V IMPRESORA]",
        "BUS +24V DC (ID1 P14)",
        [
            "Exclusiva para Fuerza Bomba Pozo Evans 24V",
            "Negativo -> STAR-GND",
        ],
    )

    payload += PARTIAL_CUT

    # ==========================================================================
    # BLOQUE 4: SENSORES Y BARRA DE TIERRA
    # ==========================================================================
    payload += format_block_banner("BLOQUE 4: SENSORES Y BARRA DE TIERRA")

    payload += format_label(
        "[STAR-GND: BARRA COMUN DE TIERRA]",
        "STAR-GND (0V COMUN)",
        [
            "Unir aqui: GND ESP32 + Negativo ATX +",
            "Negativo 12V + Negativo 24V +",
            "Pin 6 RJ12 Bixolon",
        ],
    )

    payload += format_label(
        "[ADC P36: VOLTAJE CELDA Y AMPERAJE]",
        "DIVISOR 4:1 | GPIO 36 (P36/VP)",
        [
            "Pin 03 (P36 / VP / ADC1_CH0)",
            "3x10k en serie a +12V Electrodo",
            "1x10k a STAR-GND (Ratio 4:1)",
            "Mide 0-14V DC y estima Amperios DC",
        ],
    )

    payload += format_label(
        "[ADC P34: CORRIENTE AC PRIMARIA 5A]",
        "ACS712-05B | GPIO 34 (P34)",
        [
            "GPIO 34 (ADC1_CH6)",
            "Monitorea 120V AC Fuente ATX",
            "0.3A a 0.9A AC (Protege sensor 5A)",
        ],
    )

    payload += format_label(
        "[ADC P35: CORRIENTE BOMBAS DC]",
        "ADC1 | GPIO 35 (P35)",
        [
            "GPIO 35",
            "Proteccion Dead-Head > 7.5A",
        ],
    )

    payload += format_label(
        "[ADC P32: NIVEL HIDROSTATICO CONO]",
        "ADC1 | GPIO 32 (P32)",
        [
            "GPIO 32",
            "Campana 0-100% (1 BBL = 117.3L)",
        ],
    )

    payload += format_label(
        "[P21: PARO EMERGENCIA REBOSE]",
        "STOP | GPIO 21 (P21)",
        [
            "GPIO 21 (PULLUP a GND)",
            "2 Hilos Cobre en Tapa Cono",
        ],
    )

    payload += format_label(
        "[P19: NIVEL ALTO TOTEM 1]",
        "NIVEL | GPIO 19 (P19)",
        [
            "GPIO 19 (PULLUP a GND)",
            "Corta Bomba Evans al mojarse",
        ],
    )

    payload += format_label(
        "[P22: PRESOSTATO DEMANDA RO]",
        "RO SW | GPIO 22 (P22)",
        [
            "GPIO 22 (PULLUP a GND)",
            "20-40 PSI Barril 20L",
        ],
    )

    payload += format_label(
        "[P18: CAUDALIMETRO HALL]",
        "FLUJO | GPIO 18 (P18)",
        [
            "GPIO 18 (ISR Pulsos)",
            "L/min y Totalizador Litros",
        ],
    )

    payload += format_label(
        "[P4: TEMPERATURA DS18B20]",
        "TEMP | GPIO 4 (P4)",
        [
            "GPIO 4 (OneWire)",
            "Resistor 4.7k a 3.3V",
        ],
    )

    payload += PARTIAL_CUT

    # ==========================================================================
    # BLOQUE 5: BANDERILLAS COMPACTAS PARA CABLES (Doble impresion espejada)
    # ==========================================================================
    payload += format_block_banner("BLOQUE 5: BANDERILLAS COMPACTAS CABLES")

    cable_flags = [
        "[P14 -> ID1: EVANS 24V]",
        "[P27 -> ID2: SSR2 CONO 120V]",
        "[P33 -> ID3: SSR1 ATX FLOC]",
        "[P26 -> ID4: SSR3 PECERA 120V]",
        "[P25 -> ID5: BOMBA TRASVASE 12V]",
        "[P23 -> ID6: MANIFOLD + UV]",
        "[P15 -> ID7: SOL. VIA A CASA]",
        "[P5  -> ID8: SOL. PRE-RO 12V]",
        "[P13 -> ID9: SOL. BYPASS RO]",
        "[P16 -> SERVO AGUJA PWM]",
        "[P17 -> BUZZER ALARMA]",
        "[P36 -> DIVISOR 4:1 VOLT/AMP]",
        "[P34 -> ACS712 AC 5A PRIMARIA]",
        "[P35 -> ADC BOMBAS DC]",
        "[P32 -> ADC NIVEL CONO]",
        "[P21 -> PARO REBOSE]",
        "[P19 -> NIVEL TOTEM]",
        "[P22 -> PRESOSTATO RO]",
        "[P18 -> CAUDALIMETRO HALL]",
        "[P4  -> TEMP DS18B20]",
    ]

    for flag in cable_flags:
        payload += format_cable_flag(flag)

    # Avance final y corte parcial de guillotina del Bloque 5
    payload += b_text("\n\n")
    payload += PARTIAL_CUT

    return bytes(payload)


# ==============================================================================
# ENVÍO DIRECTO RAW A COLA DE IMPRESIÓN DE WINDOWS (WINSPOOL.DRV)
# ==============================================================================
class DOC_INFO_1W(ctypes.Structure):
    _fields_ = [
        ("pDocName", wintypes.LPWSTR),
        ("pOutputFile", wintypes.LPWSTR),
        ("pDatatype", wintypes.LPWSTR),
    ]


def send_raw_to_windows_printer(printer_name: str, raw_bytes: bytes) -> None:
    winspool = ctypes.WinDLL("winspool.drv", use_last_error=True)

    OpenPrinterW = winspool.OpenPrinterW
    OpenPrinterW.argtypes = [wintypes.LPWSTR, ctypes.POINTER(wintypes.HANDLE), wintypes.LPVOID]
    OpenPrinterW.restype = wintypes.BOOL

    StartDocPrinterW = winspool.StartDocPrinterW
    StartDocPrinterW.argtypes = [wintypes.HANDLE, wintypes.DWORD, ctypes.POINTER(DOC_INFO_1W)]
    StartDocPrinterW.restype = wintypes.DWORD

    EndDocPrinter = winspool.EndDocPrinter
    EndDocPrinter.argtypes = [wintypes.HANDLE]
    EndDocPrinter.restype = wintypes.BOOL

    StartPagePrinter = winspool.StartPagePrinter
    StartPagePrinter.argtypes = [wintypes.HANDLE]
    StartPagePrinter.restype = wintypes.BOOL

    EndPagePrinter = winspool.EndPagePrinter
    EndPagePrinter.argtypes = [wintypes.HANDLE]
    EndPagePrinter.restype = wintypes.BOOL

    WritePrinter = winspool.WritePrinter
    WritePrinter.argtypes = [
        wintypes.HANDLE,
        wintypes.LPVOID,
        wintypes.DWORD,
        ctypes.POINTER(wintypes.DWORD),
    ]
    WritePrinter.restype = wintypes.BOOL

    ClosePrinter = winspool.ClosePrinter
    ClosePrinter.argtypes = [wintypes.HANDLE]
    ClosePrinter.restype = wintypes.BOOL

    hPrinter = wintypes.HANDLE()
    if not OpenPrinterW(printer_name, ctypes.byref(hPrinter), None):
        err = ctypes.get_last_error()
        raise RuntimeError(f"No se pudo abrir la impresora '{printer_name}' (Error Win32: {err})")

    try:
        doc_info = DOC_INFO_1W()
        doc_info.pDocName = "Agua de Gia - Diagramas ESP32 + Etiquetas Claras"
        doc_info.pOutputFile = None
        doc_info.pDatatype = "RAW"

        job_id = StartDocPrinterW(hPrinter, 1, ctypes.byref(doc_info))
        if job_id == 0:
            err = ctypes.get_last_error()
            raise RuntimeError(f"StartDocPrinterW fallo (Error Win32: {err})")

        try:
            if not StartPagePrinter(hPrinter):
                err = ctypes.get_last_error()
                raise RuntimeError(f"StartPagePrinter fallo (Error Win32: {err})")

            try:
                written = wintypes.DWORD(0)
                buf = ctypes.create_string_buffer(raw_bytes)
                if not WritePrinter(hPrinter, buf, len(raw_bytes), ctypes.byref(written)):
                    err = ctypes.get_last_error()
                    raise RuntimeError(f"WritePrinter fallo (Error Win32: {err})")
                print(f"[OK] Se enviaron {written.value} / {len(raw_bytes)} bytes RAW ESC/POS a '{printer_name}' (Job ID: {job_id}).")
            finally:
                EndPagePrinter(hPrinter)
        finally:
            EndDocPrinter(hPrinter)
    finally:
        ClosePrinter(hPrinter)


def detect_pos80_printer() -> str:
    """Detecta automáticamente la impresora térmica POS de 80mm activa en USB."""
    try:
        cmd = [
            "powershell",
            "-NoProfile",
            "-Command",
            "Get-Printer | Where-Object { $_.PortName -like 'USB*' -and ($_.Name -match 'ZKP|POS|80|Thermal|Bixolon|Epson') } | Select-Object -ExpandProperty Name",
        ]
        res = subprocess.run(cmd, capture_output=True, text=True, check=False)
        candidates = [line.strip() for line in res.stdout.splitlines() if line.strip()]
        if candidates:
            return candidates[0]
    except Exception:
        pass
    return "ZKP8008 (2)"


if __name__ == "__main__":
    only_diagrams = ("--diagrams" in sys.argv)
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    printer = args[0] if args else detect_pos80_printer()
    print(f"[*] Impresora POS de 80mm detectada: '{printer}'")
    data = build_esp32_pinout_diagrams() if only_diagrams else build_all_labels_payload()

    bin_path = r"c:\Users\PEPITO\Downloads\123portijuana\firmware\AguaDeGia_ESP32S_38Pin\etiquetas_agua_de_gia_pos80.bin"
    with open(bin_path, "wb") as f:
        f.write(data)
    print(f"[*] Archivo binario ESC/POS guardado en: {bin_path} ({len(data)} bytes)")

    send_raw_to_windows_printer(printer, data)
    print("[*] Impresion enviada con exito (Sin fondo negro + Diagramas ESP32 Izq/Der).")
