// Agua de Gía — Service Worker v6 (Network-First + Bypass Range/Video)
const CACHE_NAME = 'agua-de-gia-cache-v6';

self.addEventListener('install', (event) => {
  self.skipWaiting();
});

self.addEventListener('activate', (event) => {
  event.waitUntil(
    caches.keys().then((cacheNames) => {
      return Promise.all(
        cacheNames.map((name) => {
          if (name !== CACHE_NAME) {
            return caches.delete(name);
          }
        })
      );
    }).then(() => self.clients.claim())
  );
});

self.addEventListener('fetch', (event) => {
  if (event.request.method !== 'GET') return;

  // NEVER intercept Range requests or video/media files (prevents HTML5 video loading hang)
  if (
    event.request.headers.get('range') ||
    event.request.url.includes('.mp4') ||
    event.request.url.includes('sketchup_') ||
    event.request.url.includes('render_arquitectonico')
  ) {
    return;
  }

  // Network-First strategy so updates are always visible immediately
  event.respondWith(
    fetch(event.request)
      .then((networkResponse) => {
        if (networkResponse && networkResponse.status === 200 && networkResponse.type === 'basic') {
          const copy = networkResponse.clone();
          caches.open(CACHE_NAME).then((cache) => cache.put(event.request, copy));
        }
        return networkResponse;
      })
      .catch(() => caches.match(event.request))
  );
});
