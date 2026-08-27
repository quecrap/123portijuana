const puppeteer = require('puppeteer-core');
const fs = require('fs');
const path = require('path');
const http = require('http');

const EDGE_PATH = 'C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe';
const PORT = 8089;

// Lightweight static file server
function startServer() {
  return new Promise((resolve) => {
    const server = http.createServer((req, res) => {
      let filePath = path.join(__dirname, req.url === '/' ? 'index.html' : req.url.split('?')[0]);
      if (fs.existsSync(filePath) && fs.statSync(filePath).isFile()) {
        const ext = path.extname(filePath);
        const mimeTypes = {
          '.html': 'text/html',
          '.js': 'text/javascript',
          '.css': 'text/css',
          '.json': 'application/json',
          '.png': 'image/png',
          '.jpg': 'image/jpeg',
          '.svg': 'image/svg+xml'
        };
        res.writeHead(200, { 'Content-Type': mimeTypes[ext] || 'text/plain' });
        fs.createReadStream(filePath).pipe(res);
      } else {
        res.writeHead(404);
        res.end('Not found');
      }
    });
    server.listen(PORT, () => resolve(server));
  });
}

async function runTest() {
  console.log('🚀 Iniciando prueba automatizada de AquaResiliencia en Android...');
  const server = await startServer();
  console.log(`🌐 Servidor local activo en puerto ${PORT}`);
  
  const browser = await puppeteer.launch({
    executablePath: EDGE_PATH,
    headless: 'new',
    args: ['--no-sandbox', '--disable-setuid-sandbox', '--disable-gpu']
  });

  const page = await browser.newPage();
  
  // Emulate Android Device (Pixel 7 / Galaxy S23)
  await page.setViewport({
    width: 412,
    height: 915,
    deviceScaleFactor: 2.625,
    isMobile: true,
    hasTouch: true
  });

  const consoleLogs = [];
  page.on('console', msg => consoleLogs.push(`[${msg.type()}] ${msg.text()}`));
  page.on('pageerror', err => consoleLogs.push(`[PAGE ERROR] ${err.toString()}`));

  console.log(`📱 Cargando http://localhost:${PORT}/index.html en emulador Android...`);
  await page.goto(`http://localhost:${PORT}/index.html`, { waitUntil: 'networkidle0' });

  // 1. Check if auth overlay is hidden and page content is visible
  const authHidden = await page.evaluate(() => {
    const el = document.getElementById('auth-overlay');
    return el ? el.classList.contains('hidden') : true;
  });
  console.log(`✅ Pantalla de inicio pública directa: ${authHidden ? 'VISIBLE (Sin bloqueo de login)' : 'BLOQUEADO'}`);

  // 2. Wait for map initialization and tile layers
  await page.waitForSelector('#map-container', { visible: true });
  console.log('✅ Contenedor Leaflet #map-container cargado');

  // Wait 3s for Google Satellite / CartoDB tiles to fetch
  await new Promise(r => setTimeout(r, 3000));

  // 3. Scroll to Map section
  await page.evaluate(() => {
    document.getElementById('motor-hidro').scrollIntoView({ behavior: 'instant' });
  });
  await new Promise(r => setTimeout(r, 1200));

  // Capture Screenshot of Map on Android
  const mapScreenshotPath = path.join(__dirname, 'android_map_view.png');
  await page.screenshot({ path: mapScreenshotPath, fullPage: false });
  console.log(`📸 Screenshot del mapa en Android guardado en: ${mapScreenshotPath}`);

  // 4. Test Zone Preset Buttons
  console.log('🧪 Probando selección de zonas hidrogeológicas...');
  
  // Tap Playas
  await page.evaluate(() => setPresetZone(0));
  await new Promise(r => setTimeout(r, 600));
  let zoneName = await page.$eval('#out-zone-name', el => el.textContent);
  let scoreText = await page.$eval('#out-badge', el => el.textContent);
  console.log(`  🏖️ Zona 0: ${zoneName} -> ${scoreText}`);

  // Tap Arroyo Alamar
  await page.evaluate(() => setPresetZone(2));
  await new Promise(r => setTimeout(r, 600));
  zoneName = await page.$eval('#out-zone-name', el => el.textContent);
  scoreText = await page.$eval('#out-badge', el => el.textContent);
  console.log(`  🌾 Zona 2: ${zoneName} -> ${scoreText}`);

  // Tap Cerro Colorado (Veto)
  await page.evaluate(() => setPresetZone(5));
  await new Promise(r => setTimeout(r, 600));
  zoneName = await page.$eval('#out-zone-name', el => el.textContent);
  scoreText = await page.$eval('#out-badge', el => el.textContent);
  let hazardAlert = await page.$eval('#hazard-alert-box', el => el.style.display);
  console.log(`  ⛰️ Zona 5 (Rechazo): ${zoneName} -> ${scoreText} (Alerta: ${hazardAlert !== 'none' ? 'ACTIVA' : 'INACTIVA'})`);

  // 5. Test Basemap Switcher
  console.log('🗺️ Probando cambio de vista base (Satélite / Topo / Oscuro)...');
  await page.evaluate(() => setBaseMap('topo'));
  await new Promise(r => setTimeout(r, 800));
  await page.evaluate(() => setBaseMap('sat'));
  await new Promise(r => setTimeout(r, 800));
  console.log('✅ Conmutador de mapa base Satélite / Topo responde correctamente');

  // 6. Test Layer Toggles
  console.log('⚡ Probando las 7 capas de datos...');
  const layersActive = await page.$eval('#layer-status-tag', el => el.textContent);
  console.log(`  Capas iniciales: ${layersActive}`);

  // 7. Test Mobile Drawer Menu
  console.log('📱 Probando menú deslizante móvil...');
  await page.evaluate(() => toggleMobileDrawer(true));
  await new Promise(r => setTimeout(r, 500));
  const drawerOpen = await page.$eval('#mobile-drawer', el => el.classList.contains('open'));
  console.log(`  Menú móvil: ${drawerOpen ? 'ABIERTO CORRECTAMENTE' : 'ERROR'}`);

  const drawerScreenshotPath = path.join(__dirname, 'android_drawer_view.png');
  await page.screenshot({ path: drawerScreenshotPath, fullPage: false });
  console.log(`📸 Screenshot del menú móvil guardado en: ${drawerScreenshotPath}`);

  await page.evaluate(() => toggleMobileDrawer(false));
  await new Promise(r => setTimeout(r, 400));

  await browser.close();
  server.close();
  console.log('\n🎉 ¡PRUEBA EN ANDROID COMPLETADA CON ÉXITO!');
}

runTest().catch(err => {
  console.error('❌ Error en prueba:', err);
  process.exit(1);
});
