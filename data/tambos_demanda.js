const demandaTambosGeoJSON = {
  "type": "FeatureCollection",
  "features": [
    {
      "type": "Feature",
      "properties": {
        "colonia": "Natura",
        "precio_tambo": 350,
        "precio_1000L": 1800,
        "demanda": "Muy Alta - 40 comentarios",
        "fuente": "FB 402330768853014/post/1116077894144961 2026-08-25",
        "entrega": "Sí",
        "cliente_ideal": "pozo somero 12-20m"
      },
      "geometry": {
        "type": "Point",
        "coordinates": [-116.9676, 32.4249]
      }
    },
    {
      "type": "Feature",
      "properties": {
        "colonia": "Delicias 2 y 3",
        "precio_tambo": 350,
        "demanda": "Alta",
        "fuente": "comentarios post $350"
      },
      "geometry": {
        "type": "Point",
        "coordinates": [-116.94269, 32.52549]
      }
    },
    {
      "type": "Feature",
      "properties": {
        "colonia": "El Florido 1ra-3ra",
        "precio_tambo": 350,
        "demanda": "Muy Alta",
        "nota_hidro": "Transición Profunda >7m pero con manto colgado por fugas CESPT 16-24% (Reporte CESPT 2024-2025)"
      },
      "geometry": {
        "type": "Point",
        "coordinates": [-116.8871, 32.4647]
      }
    },
    {
      "type": "Feature",
      "properties": {
        "colonia": "Urbi Villa del Prado 1 y 2",
        "precio_tambo": 350,
        "demanda": "Alta"
      },
      "geometry": {
        "type": "Point",
        "coordinates": [-116.96919, 32.40530]
      }
    },
    {
      "type": "Feature",
      "properties": {
        "colonia": "Mariano Matamoros",
        "precio_tambo": 200,
        "demanda": "Media - agotado",
        "fuente": "FB 540929850040992/post/1933602037440426 2026-09-14"
      },
      "geometry": {
        "type": "Point",
        "coordinates": [-116.97904, 32.49865]
      }
    },
    {
      "type": "Feature",
      "properties": {
        "colonia": "Villas del Campo / Venados / Amanecer",
        "precio_tambo": 350,
        "demanda": "Alta",
        "fuente": "comentarios Venados, Urbi, Amanecer"
      },
      "geometry": {
        "type": "Point",
        "coordinates": [-116.95, 32.43]
      }
    },
    {
      "type": "Feature",
      "properties": {
        "colonia": "Valle de las Palmas",
        "precio_1000L": 1800,
        "demanda": "Alta - ranchos",
        "cliente_ideal": "pozo somero 12-25m + bomba solar"
      },
      "geometry": {
        "type": "Point",
        "coordinates": [-117.0383, 32.5149]
      }
    }
  ]
};

if (typeof window !== 'undefined') {
  window.demandaTambosGeoJSON = demandaTambosGeoJSON;
}
if (typeof globalThis !== 'undefined') {
  globalThis.demandaTambosGeoJSON = demandaTambosGeoJSON;
}
if (typeof module !== 'undefined' && module.exports) {
  module.exports = demandaTambosGeoJSON;
}

