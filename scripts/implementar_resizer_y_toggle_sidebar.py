import re

with open('visor_satelital_tijuana.html', 'r', encoding='utf-8') as f:
    html = f.read()

# 1. Modificar CSS de #sidebar y agregar estilos de resizer y toggle button
css_sidebar_nuevo = """
        #sidebar {
            width: 440px;
            background: var(--bg-card);
            backdrop-filter: blur(14px);
            border-right: none;
            display: flex;
            flex-direction: column;
            z-index: 1000;
            overflow-y: auto !important;
            overflow-x: hidden !important;
            box-shadow: 4px 0 24px rgba(0,0,0,0.6);
            box-sizing: border-box;
            flex-shrink: 0;
            transition: width 0.25s cubic-bezier(0.16, 1, 0.3, 1), padding 0.25s ease;
            scrollbar-width: thin;
            scrollbar-color: rgba(56, 189, 248, 0.3) transparent;
        }

        #sidebar::-webkit-scrollbar {
            width: 5px;
        }
        #sidebar::-webkit-scrollbar-track {
            background: transparent;
        }
        #sidebar::-webkit-scrollbar-thumb {
            background: rgba(56, 189, 248, 0.3);
            border-radius: 4px;
        }
        #sidebar::-webkit-scrollbar-thumb:hover {
            background: rgba(56, 189, 248, 0.6);
        }

        #sidebar.collapsed {
            width: 0 !important;
            min-width: 0 !important;
            padding: 0 !important;
            overflow: hidden !important;
            box-shadow: none !important;
        }

        /* Divisor interactivo y arrastrable */
        #sidebar-resizer {
            width: 7px;
            background: rgba(15, 23, 42, 0.9);
            border-left: 1px solid rgba(56, 189, 248, 0.25);
            border-right: 1px solid rgba(0, 0, 0, 0.8);
            cursor: col-resize;
            z-index: 1001;
            display: flex;
            align-items: center;
            justify-content: center;
            flex-shrink: 0;
            transition: background 0.15s ease;
            user-select: none;
        }

        #sidebar-resizer:hover, #sidebar-resizer.active {
            background: rgba(0, 229, 255, 0.4);
            border-left-color: var(--accent-cyan);
        }

        .resizer-handle {
            width: 3px;
            height: 36px;
            border-radius: 2px;
            background: rgba(255, 255, 255, 0.25);
            transition: background 0.15s ease;
        }

        #sidebar-resizer:hover .resizer-handle, #sidebar-resizer.active .resizer-handle {
            background: #ffffff;
            box-shadow: 0 0 8px rgba(0, 229, 255, 0.8);
        }

        /* Botón flotante para Ocultar / Mostrar panel lateral */
        #sidebar-toggle-btn {
            position: absolute;
            top: 14px;
            left: 450px;
            z-index: 1002;
            background: rgba(15, 23, 42, 0.94);
            border: 1.5px solid var(--accent-cyan);
            color: var(--accent-cyan);
            border-radius: 8px;
            width: 34px;
            height: 34px;
            display: flex;
            align-items: center;
            justify-content: center;
            cursor: pointer;
            box-shadow: 0 4px 16px rgba(0, 0, 0, 0.6);
            backdrop-filter: blur(10px);
            font-size: 0.88rem;
            font-weight: 800;
            transition: left 0.25s cubic-bezier(0.16, 1, 0.3, 1), transform 0.2s ease, background 0.2s ease;
        }

        #sidebar-toggle-btn:hover {
            background: var(--accent-cyan);
            color: #040814;
            transform: scale(1.08);
            box-shadow: 0 0 18px rgba(0, 229, 255, 0.8);
        }
"""

# Reemplazar estilo antiguo de #sidebar
css_sidebar_antiguo = """        #sidebar {
            width: 440px;
            background: var(--bg-card);
            backdrop-filter: blur(14px);
            border-right: 1px solid var(--border);
            display: flex;
            flex-direction: column;
            z-index: 1000;
            overflow-y: auto;
            box-shadow: 4px 0 24px rgba(0,0,0,0.6);
        }"""

if css_sidebar_antiguo in html:
    html = html.replace(css_sidebar_antiguo, css_sidebar_nuevo, 1)
    print("CSS de #sidebar y resizer actualizado.")
else:
    print("ALERTA: no se encontró css_sidebar_antiguo exacto.")

# 2. Insertar markup de #sidebar-resizer y #sidebar-toggle-btn
markup_antiguo = """    </div>

    <!-- Map Container -->
    <div id="map"></div>"""

markup_nuevo = """    </div>

    <!-- Divisor Interactivo Arrastrable -->
    <div id="sidebar-resizer" title="Arrastra para cambiar el ancho del panel o haz doble clic para ocultarlo">
        <div class="resizer-handle"></div>
    </div>

    <!-- Botón Flotante para Ocultar / Mostrar Panel -->
    <button id="sidebar-toggle-btn" title="Ocultar panel lateral">
        <span id="sidebar-toggle-icon">◀</span>
    </button>

    <!-- Map Container -->
    <div id="map"></div>"""

if markup_antiguo in html:
    html = html.replace(markup_antiguo, markup_nuevo, 1)
    print("Markup de resizer y toggle button insertado.")
else:
    print("ALERTA: no se encontró markup_antiguo exacto.")

# 3. Insertar script de interacción para arrastrar y ocultar el sidebar
script_resizer = """
        // Interacción del divisor arrastrable y botón de ocultar panel
        const sidebarElem = document.getElementById('sidebar');
        const resizerElem = document.getElementById('sidebar-resizer');
        const toggleBtnElem = document.getElementById('sidebar-toggle-btn');
        const toggleIconElem = document.getElementById('sidebar-toggle-icon');
        const timelineElem = document.querySelector('.timeline-container');

        let isResizing = false;
        let sidebarWidth = 440;

        function applySidebarWidth(w) {
            sidebarWidth = Math.max(260, Math.min(window.innerWidth * 0.75, w));
            sidebarElem.style.width = sidebarWidth + 'px';
            if (!sidebarElem.classList.contains('collapsed')) {
                if (toggleBtnElem) toggleBtnElem.style.left = (sidebarWidth + 8) + 'px';
                if (timelineElem) timelineElem.style.left = (sidebarWidth + 24) + 'px';
            }
            if (typeof map !== 'undefined') map.invalidateSize();
        }

        function toggleSidebarCollapse() {
            const isCollapsed = sidebarElem.classList.toggle('collapsed');
            if (isCollapsed) {
                if (toggleBtnElem) {
                    toggleBtnElem.style.left = '12px';
                    toggleIconElem.innerText = '▶';
                    toggleBtnElem.title = 'Mostrar panel lateral';
                }
                if (timelineElem) timelineElem.style.left = '24px';
                if (resizerElem) resizerElem.style.display = 'none';
            } else {
                sidebarElem.style.width = sidebarWidth + 'px';
                if (toggleBtnElem) {
                    toggleBtnElem.style.left = (sidebarWidth + 8) + 'px';
                    toggleIconElem.innerText = '◀';
                    toggleBtnElem.title = 'Ocultar panel lateral';
                }
                if (timelineElem) timelineElem.style.left = (sidebarWidth + 24) + 'px';
                if (resizerElem) resizerElem.style.display = 'flex';
            }
            setTimeout(() => {
                if (typeof map !== 'undefined') map.invalidateSize();
            }, 260);
        }

        if (toggleBtnElem) {
            toggleBtnElem.addEventListener('click', toggleSidebarCollapse);
        }

        if (resizerElem) {
            resizerElem.addEventListener('dblclick', toggleSidebarCollapse);

            resizerElem.addEventListener('mousedown', (e) => {
                isResizing = true;
                resizerElem.classList.add('active');
                document.body.style.cursor = 'col-resize';
                document.body.style.userSelect = 'none';
            });
        }

        window.addEventListener('mousemove', (e) => {
            if (!isResizing) return;
            applySidebarWidth(e.clientX);
        });

        window.addEventListener('mouseup', () => {
            if (isResizing) {
                isResizing = false;
                if (resizerElem) resizerElem.classList.remove('active');
                document.body.style.cursor = '';
                document.body.style.userSelect = '';
                if (typeof map !== 'undefined') map.invalidateSize();
            }
        });
"""

# Insertar antes del cierre de script </script>
html = html.replace('    </script>\n</body>', script_resizer + '\n    </script>\n</body>', 1)
print("Script de interacción de resizer insertado.")

with open('visor_satelital_tijuana.html', 'w', encoding='utf-8') as f:
    f.write(html)

print("visor_satelital_tijuana.html actualizado con exito.")
