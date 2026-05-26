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
| [x] | Configure platformio.ini, partitions, lib_deps | S | |
| [x] | Define constants in configs.h | S | GPIO 5/18/19/21, 4 parallel pins |
| [x] | Write xy() coordinate mapping | M | Serpentine per panel, panel chain order |
| [x] | Write testCrosshair() | S | Red H-line, green V-line, blue origin dot |
| [~] | Flash and validate xy() mapping on hardware | M | Board flashed, dots correct, green line interlaced — serpentine bug open |
| [ ] | Power injection wiring (every 256 LEDs) | M | 4 injection points, 1000µF cap each |

## Milestone: v0.2 — Display Primitives

| Status | Task | Complexity | Notes |
|--------|------|------------|-------|
| [ ] | setPixel, fill, clear | S | In display.h |
| [ ] | drawRect, drawLine | M | |
| [ ] | Double-buffer for flicker-free updates | M | |

## Milestone: v0.3 — Content

| Status | Task | Complexity | Notes |
|--------|------|------------|-------|
| [ ] | 5×7 font + horizontal text scroll | L | |
| [ ] | Fades, wipes, rainbow animations | M | |
| [ ] | Plasma / Perlin noise animation | M | FastLED noise built-in |
| [ ] | Game of Life (Acorn, Gosper Gun seeds) | M | Toroidal wrap |
| [ ] | GIF player via AnimatedGIF + SPIFFS | L | |

## Milestone: v1.0 — Remote Control (Monetizable)

| Status | Task | Complexity | Notes |
|--------|------|------------|-------|
| [ ] | WiFi AP/STA connection | M | |
| [ ] | HTTP API: set color, trigger animation, scroll text | L | |
| [ ] | Web UI | L | |
| [ ] | OTA firmware update | M | OTAClient already implemented |

## Backlog

| Status | Task | Complexity | Notes |
|--------|------|------------|-------|
| [ ] | Frame-based animation player (JSON or PROGMEM) | L | |
| [ ] | PC-driven frame streaming over WiFi/Serial | XL | |
| [ ] | Sprite renderer | M | |
| [ ] | Multi-pin parallel output tuning (RMT) | M | Currently ~100 FPS theoretical |

## Blocked

| Task | Reason | Resolution |
|------|--------|------------|
| xy() serpentine | lx=0 maps to col 16 and col 23 alternately for green line at x=16 | Empirically test pixel indices 64 and 127 of panel 1 to confirm physical serpentine direction |
