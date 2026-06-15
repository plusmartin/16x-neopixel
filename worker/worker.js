// Cloudflare Worker — 32x32display.martin-garwil.workers.dev
// Secrets (set via `wrangler secret put`):
//   GIPHY_KEY  — GIPHY API key (never in source)
//
// Routes:
//   GET /gif     — returns a random trending GIF binary (proxied from GIPHY)
//   GET /tiktok  — returns {"followers":N,"likes":N} for @odile.juarez

const TIKTOK_USER = 'odile.juarez';

// ─────────────────────────────────────────────────────────────────────────────
export default {
  async fetch(request, env) {
    const { pathname } = new URL(request.url);
    if (pathname === '/gif')    return handleGif(env);
    if (pathname === '/tiktok') return handleTikTok();
    return new Response('not found', { status: 404 });
  }
};

// ── /gif ─────────────────────────────────────────────────────────────────────
async function handleGif(env) {
  if (!env.GIPHY_KEY) return new Response('missing GIPHY_KEY', { status: 500 });

  // Fetch trending GIFs
  const meta = await fetch(
    `https://api.giphy.com/v1/gifs/trending?api_key=${env.GIPHY_KEY}&limit=50&rating=g`
  );
  if (!meta.ok) return new Response('giphy error', { status: meta.status });

  const { data } = await meta.json();
  if (!data || data.length === 0) return new Response('no gifs', { status: 502 });

  // Pick a random result, prefer fixed_height_small (smallest file)
  const item = data[Math.floor(Math.random() * data.length)];
  const url  = item.images?.fixed_height_small?.url
             || item.images?.downsized?.url
             || item.images?.original?.url;
  if (!url) return new Response('no url', { status: 502 });

  const gif = await fetch(url);
  return new Response(gif.body, {
    headers: { 'Content-Type': 'image/gif', 'Cache-Control': 'no-store' }
  });
}

// ── /tiktok ───────────────────────────────────────────────────────────────────
// TikTok has no public API. We try two approaches in order:
//   1. Internal JSON API (lighter — JSON response, no HTML parsing)
//   2. Profile HTML scrape (fallback — regex on raw HTML)
// TikTok may block Cloudflare datacenter IPs. If both fail, returns HTTP 502
// so the ESP32 keeps the last known value and doesn't crash.

async function handleTikTok() {
  // Attempt 1 — internal JSON API
  try {
    const res = await fetch(
      `https://www.tiktok.com/api/user/detail/?uniqueId=${TIKTOK_USER}` +
      `&aid=1988&cookie_enabled=1&screen_width=1920&screen_height=1080` +
      `&browser_language=en-US&browser_platform=Win32&browser_name=Mozilla&browser_version=5.0`,
      {
        headers: {
          'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/124.0.0.0 Safari/537.36',
          'Accept': 'application/json, */*',
          'Accept-Language': 'en-US,en;q=0.9',
          'Referer': `https://www.tiktok.com/@${TIKTOK_USER}`,
        }
      }
    );
    if (res.ok) {
      const data = await res.json();
      if (data?.statusCode === 0 && data.userInfo?.stats) {
        const { followerCount, heartCount } = data.userInfo.stats;
        return json({ followers: followerCount, likes: heartCount });
      }
    }
  } catch (_) { /* fall through */ }

  // Attempt 2 — HTML scrape
  try {
    const res = await fetch(`https://www.tiktok.com/@${TIKTOK_USER}`, {
      headers: {
        'User-Agent': 'Mozilla/5.0 (iPhone; CPU iPhone OS 17_0 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.0 Mobile/15E148 Safari/604.1',
        'Accept': 'text/html,application/xhtml+xml',
        'Accept-Language': 'en-US,en;q=0.9',
      }
    });
    if (!res.ok)
      return json({ error: `tiktok_${res.status}` }, 502);

    const html = await res.text();
    const fm = html.match(/"followerCount"\s*:\s*(\d+)/);
    const lm = html.match(/"heartCount"\s*:\s*(\d+)/);
    if (!fm || !lm)
      return json({ error: 'no_data_in_html' }, 502);

    return json({ followers: parseInt(fm[1]), likes: parseInt(lm[1]) });
  } catch (e) {
    return json({ error: e.message }, 500);
  }
}

function json(obj, status = 200) {
  return new Response(JSON.stringify(obj), {
    status,
    headers: { 'Content-Type': 'application/json' }
  });
}
