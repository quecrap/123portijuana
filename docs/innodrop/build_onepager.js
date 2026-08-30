const {
  Document, Packer, Paragraph, TextRun, HeadingLevel, AlignmentType,
  Table, TableRow, TableCell, WidthType, ShadingType, BorderStyle,
  Spacing, ExternalHyperlink,
} = require("docx");
const fs = require("fs");

const CYAN = "0E7C86";
const DARKINK = "0E1B26";
const MUTED = "5B7079";
const LINE = "DCE7EC";

function tag(text, color) {
  return new TextRun({ text, bold: true, color: color || CYAN, size: 16 });
}

function body(text, opts = {}) {
  return new TextRun({ text, size: 18, color: DARKINK, ...opts });
}

function bullet(runs) {
  return new Paragraph({
    bullet: { level: 0 },
    spacing: { after: 60 },
    children: Array.isArray(runs) ? runs : [runs],
  });
}

const doc = new Document({
  sections: [
    {
      properties: {
        page: {
          size: { width: 12240, height: 15840 }, // US Letter
          margin: { top: 620, bottom: 500, left: 620, right: 620 },
        },
      },
      children: [
        // Header
        new Paragraph({
          spacing: { after: 20 },
          children: [
            new TextRun({ text: "💧 AquaResiliencia Tijuana", bold: true, size: 34, color: CYAN }),
          ],
        }),
        new Paragraph({
          spacing: { after: 140 },
          children: [
            new TextRun({ text: "Red Centinela de Seguridad Hídrica — Postulación INNODROP 2026", size: 20, color: MUTED, italics: true }),
          ],
          border: { bottom: { color: LINE, space: 4, style: BorderStyle.SINGLE, size: 6 } },
        }),

        // Category line
        new Paragraph({
          spacing: { after: 160 },
          children: [
            tag("CATEGORÍA:  "),
            body("Emprendedores mayores de edad, operaciones en México", { bold: true }),
            body("      |      "),
            tag("REGIÓN:  "),
            body("Tijuana / Baja California — Acuífero CONAGUA 0201"),
          ],
        }),

        // Problem
        new Paragraph({
          spacing: { before: 60, after: 60 },
          children: [tag("EL PROBLEMA", CYAN)],
        }),
        new Paragraph({
          spacing: { after: 160 },
          children: [
            body(
              "Tijuana depende en más del 90% del Acueducto Río Colorado–Tijuana, que cruza La Rumorosa con un consumo energético de ~7.5 kWh/m³ y está sujeto a la crisis del Lago Mead. El acuífero local (CONAGUA 0201) tiene un déficit oficial de "
            ),
            body("-1,664,020 m³/año", { bold: true }),
            body(
              " (DR_0201, 2024). En las colonias periféricas, miles de familias no tienen forma de saber, en tiempo real, si el agua subterránea bajo su predio es segura o si extraerla arriesga sobreexplotar el acuífero — y hoy pagan $300–$1,200 MXN/mes por agua de pipa de calidad incierta."
            ),
          ],
        }),

        // Solution
        new Paragraph({
          spacing: { before: 60, after: 60 },
          children: [tag("LA SOLUCIÓN — Plataforma de prevención, no solo extracción", CYAN)],
        }),
        bullet([
          body("AquaEngine: ", { bold: true }),
          body("modelo de viabilidad geoespacial que cruza altimetría satelital con datos oficiales CONAGUA/REPDA/SGM para clasificar un punto como seguro, condicionado o no viable antes de tocar tierra — replicable en cualquier cuenca de México."),
        ]),
        bullet([
          body("Nodo Centinela IoT (ESP32): ", { bold: true }),
          body("sensores de conductividad (TDS), nivel freático y caudal con corte automático de bomba ante abatimiento o contaminación. Firmware funcionando, no solo concepto."),
        ]),
        bullet([
          body("Caso de aplicación real: ", { bold: true }),
          body("acuífero Tijuana 0201, con matriz de 25+ fuentes documentales oficiales (CONAGUA, SGM, USGS, UABC, CICESE, El Colef) y 8 sitios piloto mapeados con coordenadas y clasificación de riesgo."),
        ]),

        // Traction
        new Paragraph({
          spacing: { before: 140, after: 60 },
          children: [tag("EVIDENCIA Y AVANCE ACTUAL", CYAN)],
        }),
        bullet(body("Prototipo funcional en producción: mapa interactivo, corte 3D hidrogeológico, calculadora de costos, generador de solicitud CONAGUA.")),
        bullet(body("Firmware ESP32 real con lógica de corte de seguridad y API de telemetría JSON.")),
        bullet(body("Equipo preseleccionado como candidato destacado por Ventures Hack 2026 (CDT) en etapa previa de validación.")),

        // Why now / impact
        new Paragraph({
          spacing: { before: 140, after: 60 },
          children: [tag("POR QUÉ AHORA", CYAN)],
        }),
        new Paragraph({
          spacing: { after: 160 },
          children: [
            body(
              "La plataforma no depende de resolver primero el litigio de la extracción: el AquaEngine y la Red Centinela ya generan valor como herramienta de datos, monitoreo y prevención de riesgo hídrico — deployable en cualquier organismo operador, municipio o cuenca del país, no solo en Tijuana."
            ),
          ],
        }),

        // Footer / contact
        new Paragraph({
          spacing: { before: 100, after: 40 },
          border: { top: { color: LINE, space: 6, style: BorderStyle.SINGLE, size: 6 } },
          children: [
            tag("CONTACTO:  ", MUTED),
            body("Pablo — archiduquecampos@gmail.com", { bold: true }),
          ],
        }),
        new Paragraph({
          children: [
            tag("REPOSITORIO / DEMO:  ", MUTED),
            body("https://github.com/quecrap/123portijuana   |   https://quecrap.github.io/123portijuana/"),
          ],
        }),
      ],
    },
  ],
});

Packer.toBuffer(doc).then((buf) => {
  fs.writeFileSync(__dirname + "/AquaResiliencia_INNODROP_2026_OnePager.docx", buf);
  console.log("written");
});
