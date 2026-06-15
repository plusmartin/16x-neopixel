# 32×32 NeoPixel Display

ESP32-S3 driven 32×32 WS2812B LED matrix with WiFi, web UI, animated GIFs, and live panel calibration.

## Hardware

| Component | Detail |
|-----------|--------|
| MCU | ESP32-S3 DevKitC-1 |
| Display | 32×32 WS2812B (sixteen 8×8 panels arranged 4×4) |
| Data pins | GPIO 5, 18, 19, 21 — one per row of panels (parallel) |
| Flash | 16 MB, custom partition table |
| Power | 5 V, external supply with injection at each row recommended |

Panel wiring: each data pin drives a chain of 4 panels (256 LEDs). Panels are ordered right→left within each chain; pixels start at the bottom-right corner of each panel.

## Features

### Animations (Auto mode rotates through all)
- Fire, Matrix Rain, Starfield, Meteor Shower, Plasma, Rainbow, Color Wipe, Sparkle
- Game of Life, Breathe
- Animated GIFs downloaded from GIPHY via Cloudflare Worker

### Clocks
- 3-row static (time / temperature / day)
- Analog clock, Arc clock, Binary clock, Bar clock, Pong clock, Color clock

### Text scroll
- Arbitrary message, RGB color, adjustable speed via web UI

### TikTok stats
Fetches follower and like count for @odile.juarez every 20 minutes via Cloudflare Worker. When either value changes the new count scrolls across the display in TikTok red during the next clock phase.

### Web UI & API
Served at `http://<device-ip>/` — no app required.

### Over-the-air updates (OTA)
Triggered from the web UI. Pulls `firmware.bin` from a configured URL.

### Per-panel color calibration
Persistent RGB correction factors per panel, stored in NVS flash. Survives reboots.

## Setup

### 1. Credentials

```bash
cp src/secrets.h.example src/secrets.h
# Edit src/secrets.h with your WiFi SSIDs, passwords, and OpenWeatherMap key
```

`src/secrets.h` is gitignored and never committed.

### 2. Cloudflare Worker (optional)

The Worker at `worker/worker.js` handles two endpoints:
- `/gif` — proxies GIPHY trending GIFs (requires `GIPHY_KEY` secret)
- `/tiktok` — scrapes @odile.juarez follower/like counts

To deploy:

1. Create a free account at [cloudflare.com](https://cloudflare.com) and [giphy.com](https://developers.giphy.com)
2. Create a Cloudflare Worker and paste the contents of `worker/worker.js`
3. Add your GIPHY API key as a Worker secret named `GIPHY_KEY`
4. Update `GIF_WORKER_URL` and `TIKTOK_WORKER_URL` in `src/configs.h` with your Worker URL

The GIPHY API key lives only in Cloudflare — never in this repo.

### 3. Flash

```bash
# Install PlatformIO, then:
pio run --target upload
```

On first boot the device downloads 4 GIFs and connects to WiFi. Serial monitor at 115200 baud shows status.

## Web API

All endpoints are GET requests — usable directly from a browser address bar.

| Endpoint | Parameters | Description |
|----------|-----------|-------------|
| `GET /` | — | Web UI |
| `GET /api/mode` | `name=auto\|clock\|fire\|plasma\|rainbow\|matrix\|starfield\|meteor\|life\|wipe\|sparkle\|breathe` | Set display mode |
| `GET /api/text` | `msg=`, `color=RRGGBB`, `speed=ms` | Scroll text |
| `GET /api/brightness` | `val=1..255` | Set brightness |
| `GET /api/cal` | — | Show calibration state |
| `GET /api/cal` | `panel=0..15&r=0..255&g=0..255&b=0..255` | Set per-panel correction |
| `GET /api/cal` | `white=1` | Lock display to dim white for calibration |
| `GET /api/ota` | — | Trigger OTA firmware update |

## Panel Calibration

If a panel's white balance differs from the others (common with mixed LED batches):

```
# 1. Lock display to white
http://<ip>/api/cal?white=1

# 2. Find panel index — panel = row*4 + (3-col), rows/cols 0-indexed from top-left
#    Bottom row, 3rd from left = panel 13

# 3. Reduce the channel that looks too strong (steps of ~10)
http://<ip>/api/cal?panel=13&r=220&g=255&b=255&white=1

# 4. Return to normal when done
http://<ip>/api/mode?name=auto
```

Corrections are stored in NVS and applied automatically on every frame across all brightness levels.

## External services

All network calls made by the device at runtime:

| Service | URL / host | Credential | Purpose |
|---------|-----------|-----------|---------|
| NTP | `pool.ntp.org` | none | Time sync on boot and every clock phase |
| OpenWeatherMap | `api.openweathermap.org` | `WEATHER_API_KEY` in `secrets.h` | Temperature shown on clock face |
| Cloudflare Worker `/gif` | `32x32display.martin-garwil.workers.dev` | Worker secret `GIPHY_KEY` | Proxy for GIPHY trending GIFs |
| Cloudflare Worker `/tiktok` | `32x32display.martin-garwil.workers.dev` | none | TikTok @odile.juarez follower/like count |
| GitHub Releases | `github.com/plusmartin/16x-neopixel` | none | OTA firmware binary (`firmware.bin`) |

No credentials are stored in this repo. All secrets live in `src/secrets.h` (gitignored) or Cloudflare Worker secrets.

## OTA firmware updates

The device is identified by its firmware version (`FW_VERSION` in `src/configs.h`). The version is printed to serial every 10 seconds and shown in the web UI. To deploy a new version without USB:

1. Bump `FW_VERSION` in `src/configs.h` before building
2. Build: `pio run`
3. Upload `.pio/build/esp32dev/firmware.bin` to a GitHub Release as `firmware.bin`
4. Trigger OTA via `GET http://<device-ip>/api/ota` or the web UI button

Current deployed version: **1.3.0**

## Configuration

Key constants in `src/configs.h`:

| Constant | Default | Description |
|----------|---------|-------------|
| `FW_VERSION` | `"1.3.0"` | Firmware version — bump before each OTA release |
| `BRIGHTNESS` | 80 | Default LED brightness (0–255); night mode forces 3 between 22:00–06:00 |
| `TIMEZONE_OFFSET` | -21600 | UTC offset in seconds (Mexico City = UTC-6) |
| `GIF_BOOT_FETCH` | 4 | GIFs downloaded at boot |
| `GIF_FETCH_INTERVAL` | 3600000 | GIF refresh interval ms (1 hour) |
| `TIKTOK_CHECK_INTERVAL` | 1200000 | TikTok stats poll interval ms (20 min) |
| `POWER_BUDGET` | NUM_LEDS×3×100 | Max total LED channel sum (set in `display.h show()`) |

## Project structure

```
src/
  main.cpp          — setup, loop, WiFi, NTP, M_AUTO scheduler
  configs.h         — hardware pins, display size, tuneable constants
  secrets.h         — credentials (gitignored, see secrets.h.example)
  display.h         — backbuf/leds, setPixel, show, xy mapping
  calibration.h     — per-panel NVS color correction
  animations.h      — fire, plasma, rainbow, sparkle, etc.
  clocks.h          — 6 clock face animations
  life.h            — Conway's Game of Life
  text.h            — font renderer and scrolling text
  gif.h             — AnimatedGIF player (LittleFS + GIPHY worker)
  weather.h         — OpenWeatherMap temperature fetch
  tiktok.h          — TikTok follower/like fetch, triggers scroll on change
  api.h             — WebServer routes and HTML UI
  OTAClient.h       — HTTPS OTA firmware update
worker/
  worker.js         — Cloudflare Worker: /gif (GIPHY proxy) + /tiktok (TikTok stats)
partitions.csv      — 16 MB flash layout (3 MB app×2, ~10 MB LittleFS)
```
