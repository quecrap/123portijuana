import re

with open('visor_satelital_tijuana.html', 'r', encoding='utf-8') as f:
    html = f.read()

# 1. Agregar CSS estilizado para .leaflet-control-layers
css_selector_capas = """
        /* Estilo Glassmorphism Compacto para Selector de Capas */
        .leaflet-control-layers {
            background: rgba(15, 23, 42, 0.94) !important;
            border: 1px solid rgba(56, 189, 248, 0.4) !important;
            border-radius: 12px !important;
            box-shadow: 0 12px 32px rgba(0, 0, 0, 0.75) !important;
            backdrop-filter: blur(16px) !important;
            color: #f8fafc !important;
            font-family: 'Inter', sans-serif !important;
            font-size: 0.74rem !important;
            padding: 10px 14px !important;
            max-height: 60vh !important;
            max-width: 320px !important;
            overflow-y: auto !important;
            z-index: 1000 !important;
        }

        .leaflet-control-layers-expanded {
            padding: 12px 14px !important;
            animation: fadeInLayers 0.2s cubic-bezier(0.16, 1, 0.3, 1);
        }

        @keyframes fadeInLayers {
            from { opacity: 0; transform: translateY(-6px) scale(0.98); }
            to { opacity: 1; transform: translateY(0) scale(1); }
        }

        .leaflet-control-layers-toggle {
            background-color: rgba(15, 23, 42, 0.92) !important;
            border: 1.5px solid var(--accent-cyan) !important;
            border-radius: 10px !important;
            width: 44px !important;
            height: 44px !important;
            box-shadow: 0 0 16px rgba(0, 229, 255, 0.45) !important;
            transition: all 0.2s ease !important;
            background-size: 26px 26px !important;
        }

        .leaflet-control-layers-toggle:hover {
            transform: scale(1.08) !important;
            box-shadow: 0 0 24px rgba(0, 229, 255, 0.8) !important;
            background-color: rgba(15, 23, 42, 1) !important;
        }

        .leaflet-control-layers label {
            margin-bottom: 5px !important;
            display: flex !important;
            align-items: center !important;
            cursor: pointer !important;
            color: #e2e8f0 !important;
            line-height: 1.35 !important;
            user-select: none;
        }

        .leaflet-control-layers label:hover {
            color: var(--accent-cyan) !important;
        }

        .leaflet-control-layers-separator {
            border-top: 1px solid rgba(255, 255, 255, 0.15) !important;
            margin: 8px 0 !important;
        }

        .leaflet-control-layers::-webkit-scrollbar {
            width: 5px;
        }
        .leaflet-control-layers::-webkit-scrollbar-thumb {
            background: rgba(56, 189, 248, 0.4);
            border-radius: 4px;
        }
"""

# Insertar antes de </style>
html = html.replace('    </style>', css_selector_capas + '\n    </style>', 1)
print("CSS del selector de capas insertado.")

# 2. Modificar overlayMaps con nombres más compactos y limpios
bloque_overlay_antiguo = """        const overlayMaps = {
            "🟣 Red Piezométrica Oficial CONAGUA (Acuífero 0201 - 48 Pozos)": capaPiezometriaOficial,
            "🟣 Red Piezométrica CONAGUA (Acuífero 0245 Rosarito - 3 Pozos)": capaPiezometriaRosarito,
            "🟢 Planta Demostrativa Agua de Gía (Reactor POU 1 BBL)": capaMonitoreoPropio,
            "🔵 Aprovechamientos y Concesiones (REPDA - 20 Títulos)": capaRepda,
            "🟡 Referencias Indirectas, Norias y Cárcamos de Achique": capaReferenciasIndirectas,
            "💧 Umbrales de Proximidad Visual 1 km - Campaña 2009 (48 Estaciones)": capaProximidad2009,
            "💧 Umbrales de Proximidad Visual 1 km - Campaña 2013 (27 Estaciones)": capaProximidad2013,
            "🔴 Focos InSAR (Deformación y Pronóstico Sentinel-1)": zonasInSARGroup,
            "🔥 Índice Exploratorio de Humedad y Referencias (Heurístico)": heatLayerGroup,
            "⚡ Sismicidad Reciente (USGS)": sismosGroup,
            "📡 Estación Telemetría USGS": usgsGroup
        };

        L.control.layers(baseMaps, overlayMaps, { position: 'topright', collapsed: false }).addTo(map);"""

bloque_overlay_nuevo = """        const overlayMaps = {
            "🟣 Piezometría Oficial CONAGUA (Acuífero 0201 - 48 Pozos)": capaPiezometriaOficial,
            "🟣 Piezometría CONAGUA (Acuífero 0245 Rosarito - 3 Pozos)": capaPiezometriaRosarito,
            "🟢 Planta Demostrativa Agua de Gía (Reactor 1 BBL)": capaMonitoreoPropio,
            "🔵 Concesiones REPDA (20 Títulos)": capaRepda,
            "🟡 Referencias Urbanas y Cárcamos": capaReferenciasIndirectas,
            "💧 Proximidad Visual 1 km - 2009 (48 Pozos)": capaProximidad2009,
            "💧 Proximidad Visual 1 km - 2013 (27 Pozos)": capaProximidad2013,
            "🔴 Deformación InSAR (Sentinel-1)": zonasInSARGroup,
            "🔥 Índice Heurístico de Humedad": heatLayerGroup,
            "⚡ Sismicidad Reciente (USGS)": sismosGroup,
            "📡 Testigo Telemetría USGS Nestor": usgsGroup
        };

        // Selector plegable con botón de cierre explícito (no tapa el mapa)
        const layerControl = L.control.layers(baseMaps, overlayMaps, {
            position: 'topright',
            collapsed: true
        }).addTo(map);

        // Inyectar botón de cierre y encabezado dentro del selector
        setTimeout(() => {
            const container = layerControl.getContainer();
            if (container) {
                const form = container.querySelector('form');
                if (form) {
                    const header = document.createElement('div');
                    header.style.cssText = 'display:flex; justify-content:space-between; align-items:center; margin-bottom:8px; padding-bottom:6px; border-bottom:1px solid rgba(255,255,255,0.15); font-weight:800; color:#00E5FF; font-size:0.75rem;';
                    header.innerHTML = '<span>🗺️ CAPAS CARTOGRÁFICAS</span><button id=\"btnCerrarCapas\" type=\"button\" style=\"background:#ef4444; color:#fff; border:none; border-radius:4px; padding:2px 7px; font-size:0.68rem; font-weight:800; cursor:pointer;\">✕ Cerrar</button>';
                    form.insertBefore(header, form.firstChild);

                    const btnCerrar = header.querySelector('#btnCerrarCapas');
                    if (btnCerrar) {
                        btnCerrar.addEventListener('click', (e) => {
                            e.preventDefault();
                            e.stopPropagation();
                            layerControl.collapse();
                        });
                    }
                }
            }
        }, 150);

        // Al dar clic en el mapa, colapsar el selector automáticamente
        map.on('click', () => {
            layerControl.collapse();
        });"""

if bloque_overlay_antiguo in html:
    html = html.replace(bloque_overlay_antiguo, bloque_overlay_nuevo, 1)
    print("Selector de capas convertido a plegable (collapsed: true) con botón de cierre.")
else:
    print("ALERTA: no se encontró bloque_overlay_antiguo exacto.")

with open('visor_satelital_tijuana.html', 'w', encoding='utf-8') as f:
    f.write(html)

print("visor_satelital_tijuana.html actualizado con éxito.")
