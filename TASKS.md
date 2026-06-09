# Tasks: 16x NeoPixel Display

## Legend
- `[ ]` pending  `[x]` done  `[~]` in progress  `[!]` blocked

## Complexity Scale
- `S` < 1h  `M` 1–4h  `L` 4–8h  `XL` multi-day

---

## Milestone: v0.1 — Hardware Validated

| Status | Task | Complexity | Notes |
|--------|------|------------|-------|
| [x] | Choose library (FastLED) | S | Over Adafruit NeoPixel — better perf on ESP32 |
| [x] | Configure platformio.ini, partitions, lib_deps | S | 16MB flash, custom partitions.csv |
| [x] | Define constants in configs.h | S | GPIO 5/18/19/21, 4 parallel pins |
| [x] | Write xy() coordinate mapping | M | No serpentine; all rows right→left; y-flip |
| [x] | Flash and validate xy() mapping on hardware | M | Confirmed correct |
| [ ] | Power injection wiring (every 256 LEDs) | M | 4 injection points, 1000µF cap each |

## Milestone: v0.2 — Display Primitives

| Status | Task | Complexity | Notes |
|--------|------|------------|-------|
| [x] | setPixel, fill, clear | S | display.h |
| [x] | drawRect, drawLine | M | display.h |
| [x] | Double-buffer for flicker-free updates | M | backbuf → leds via show() |

## Milestone: v0.3 — Content

| Status | Task | Complexity | Notes |
|--------|------|------------|-------|
| [x] | 5×7 font + horizontal text scroll | L | text.h, PROGMEM font, non-blocking |
| [x] | Fades, wipes, rainbow animations | M | animations.h |
| [x] | Plasma / Perlin noise animation | M | FastLED inoise8, squared contrast |
| [x] | Game of Life (Acorn, Gosper Gun seeds) | M | Period-1+2 stagnation detection, 600 gen cap |
| [ ] | GIF player via AnimatedGIF + LittleFS | L | Backlog |

## Milestone: v1.0 — Remote Control (Monetizable)

| Status | Task | Complexity | Notes |
|--------|------|------------|-------|
| [x] | WiFi AP/STA connection (2 SSIDs + AP fallback) | M | main.cpp |
| [x] | HTTP API: mode, text, brightness | L | api.h |
| [x] | Web UI | L | api.h PROGMEM HTML |
| [x] | OTA firmware update | M | HTTPUpdate + redirect resolver; repo público en GitHub |

## Blocked

| Task | Reason | Resolution |
|------|--------|------------|

## Backlog

| Status | Task | Complexity | Notes |
|--------|------|------------|-------|
| [ ] | More animations — fire, matrix rain, voronoi, cellular automata, Fourier, starfield | M–L | Research task for openclaw |
| [ ] | GIF player via AnimatedGIF + LittleFS | L | |
| [ ] | Frame-based animation player (JSON or PROGMEM) | L | |
| [ ] | PC-driven frame streaming over WiFi/Serial | XL | |
| [ ] | Sprite renderer | M | |
