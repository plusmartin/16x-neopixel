#ifndef CONFIGS_H
#define CONFIGS_H

// ----- Hardware — 4 parallel data pins (one per panel row) -----
// Each pin drives 256 LEDs (panels 0-3, 4-7, 8-11, 12-15 respectively).
// ESP32 assigns one RMT channel per pin and transmits all in parallel.
#define DATA_PIN_0     5      // top row of panels
#define DATA_PIN_1     18
#define DATA_PIN_2     19
#define DATA_PIN_3     21     // bottom row of panels

#define LEDS_PER_PIN   256    // NUM_LEDS / 4

// ----- Panel layout -----
#define PANEL_W        8      // pixels per panel (width)
#define PANEL_H        8      // pixels per panel (height)
#define PANELS_X       4      // panels across
#define PANELS_Y       4      // panels down

// ----- Derived -----
#define MATRIX_W       (PANEL_W * PANELS_X)   // 32
#define MATRIX_H       (PANEL_H * PANELS_Y)   // 32
#define NUM_LEDS       (MATRIX_W * MATRIX_H)  // 1024

// ----- FastLED -----
#define LED_TYPE       WS2812B
#define COLOR_ORDER    GRB
#define BRIGHTNESS     80

// ── Version ──────────────────────────────────────────────────────────────────
// Increment before each OTA release. Device prints this on serial at boot and
// every 10 s. Check serial or /api/mode response to confirm the version running.
#define FW_VERSION "1.3.0"

// ── OTA ──────────────────────────────────────────────────────────────────────
// GitHub Releases asset. Upload firmware.bin to a new release to deploy OTA.
// Trigger via web UI or GET /api/ota. Device pulls HTTPS, verifies, reboots.
#define OTA_URL "https://github.com/plusmartin/16x-neopixel/releases/latest/download/firmware.bin"

// ── Time ─────────────────────────────────────────────────────────────────────
// NTP server: pool.ntp.org (public, no key needed).
// UTC offset in seconds. Mexico City = UTC-6 = -21600.
#define TIMEZONE_OFFSET  (-6L * 3600L)

// ── GIF worker ───────────────────────────────────────────────────────────────
// Cloudflare Worker proxies GIPHY trending GIFs.
// Worker source: worker/worker.js — deploy to your own Worker and update URL.
// Secret GIPHY_KEY must be set in the Worker (wrangler secret put GIPHY_KEY).
// GIPHY free tier: https://developers.giphy.com
#define GIF_WORKER_URL     "https://32x32display.martin-garwil.workers.dev/gif"
#define GIF_BOOT_FETCH     4                          // GIFs downloaded at boot
#define GIF_FETCH_INTERVAL (3600UL * 1000UL)          // refresh one slot per hour

// ── TikTok stats ─────────────────────────────────────────────────────────────
// Same Cloudflare Worker, /tiktok endpoint scrapes @odile.juarez profile.
// No API key needed — scrapes public page. May break if TikTok blocks scraping.
// On failure the device keeps last known values and retries next interval.
#define TIKTOK_WORKER_URL     "https://32x32display.martin-garwil.workers.dev/tiktok"
#define TIKTOK_CHECK_INTERVAL (20UL * 60UL * 1000UL)  // 20 minutes

#endif
