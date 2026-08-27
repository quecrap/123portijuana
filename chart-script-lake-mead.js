// Inicialización del Widget y Gráfica
document.addEventListener('DOMContentLoaded', async () => {
  const pillBtn = document.getElementById('day-zero-pill');
  const popover = document.getElementById('day-zero-popover');
  
  pillBtn.addEventListener('click', () => popover.classList.toggle('hidden'));

  // Cargar datos históricos y renderizar gráfica
  renderLakeMeadChart();
});

async function renderLakeMeadChart() {
  const ctx = document.getElementById('lakeMeadChart').getContext('2d');
  
  // Datos simulados de los últimos 12 meses (reemplazables por fetch a USGS/USBR)
  const labels = ['Sep 25', 'Nov 25', 'Ene 26', 'Mar 26', 'May 26', 'Jul 26', 'Ago 26'];
  const elevationData = [1062, 1058, 1052, 1048, 1045, 1041, 1039.16];
  const deadPoolLine = Array(labels.length).fill(895);
  const tier1Line = Array(labels.length).fill(1075);

  document.getElementById('day-zero-text').innerText = '🔴 Día Cero Río Colorado: ~1,150 días (1,039 ft)';

  new Chart(ctx, {
    type: 'line',
    data: {
      labels: labels,
      datasets: [
        {
          label: 'Elevación Lake Mead (ft)',
          data: elevationData,
          borderColor: '#38bdf8',
          backgroundColor: 'rgba(56, 189, 248, 0.1)',
          fill: true,
          tension: 0.3,
          borderWidth: 2
        },
        {
          label: 'Tier 1 Recortes (1,075 ft)',
          data: tier1Line,
          borderColor: '#f59e0b',
          borderDash: [5, 5],
          borderWidth: 1,
          pointRadius: 0
        },
        {
          label: 'Día Cero / Dead Pool (895 ft)',
          data: deadPoolLine,
          borderColor: '#ef4444',
          borderWidth: 2,
          pointRadius: 0
        }
      ]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      scales: {
        y: { min: 850, max: 1100, grid: { color: '#1e293b' }, ticks: { color: '#94a3b8', font: { size: 10 } } },
        x: { grid: { display: false }, ticks: { color: '#94a3b8', font: { size: 10 } } }
      },
      plugins: { legend: { display: false } }
    }
  });
}
