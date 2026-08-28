Necesito que cambies la navegación del sitio de scroll continuo a vista por
secciones, con menú hamburguesa en móvil. Objetivo: que un juez pueda saltar
directo a "Modelo de negocio" sin tener que hacer scroll por todo el sitio.

## Estructura de navegación

1. Convierte cada sección actual (`#hero`, `#problema`, `#motor-hidro`,
   `#humedal`, `#modelo-negocio`, `#equipo`, etc. — usa los ids que ya existen
   en el HTML) en un "panel" independiente. Solo un panel visible a la vez.

2. Barra de navegación fija arriba con:
   - Logo/nombre del proyecto a la izquierda
   - En escritorio (>768px): lista horizontal de las secciones, la activa
     resaltada con un subrayado o color distinto
   - En móvil (<768px): ícono de hamburguesa (☰) que abre un menú lateral
     deslizante (drawer) con la misma lista de secciones

3. Al hacer clic en una sección del menú:
   - Oculta el panel actual, muestra el nuevo con una transición simple
     (fade o slide, 200-300ms, nada exagerado)
   - Actualiza la URL con el hash correspondiente (para que los links
     `#motor-hidro` sigan funcionando como enlace directo)
   - En móvil, cierra el drawer automáticamente al seleccionar

4. Agrega flechas o botones "Anterior / Siguiente" al fondo de cada panel
   para navegar secuencialmente sin tener que abrir el menú cada vez —
   útil para cuando alguien SÍ quiere ver todo en orden.

5. Indicador de progreso (puntos o barra) mostrando en qué sección está
   dentro del total — ayuda a orientarse sin scroll.

## Implementación técnica sugerida

- Todo en JS vanilla, sin librerías nuevas (el sitio ya es HTML/CSS/JS puro)
- Usa `display: none` / `display: block` o clases CSS con `opacity` +
  `visibility` para el cambio de panel — evita reflow pesado
- El menú hamburguesa en móvil: un `<div>` que se desliza desde la izquierda
  o derecha, con overlay semitransparente detrás que cierra el menú al
  tocarlo
- Mantén el diseño visual actual (colores, tipografía, tarjetas) — el
  cambio es solo de navegación, no de estilo

## Importante

- Que cada sección siga siendo accesible por su hash directo
  (ej. compartir `index.html#motor-hidro` debe abrir esa sección
  directamente al cargar, no la portada)
- Prueba que funcione tanto en escritorio como en móvil antes de darlo
  por terminado — este sitio se va a mostrar en el pitch del 2-3 de
  septiembre, posiblemente desde un celular o proyector
