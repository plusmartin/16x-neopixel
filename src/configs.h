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
#define FW_VERSION "1.1.0"

// ── OTA ──────────────────────────────────────────────────────────────────────
#define OTA_URL "https://github.com/plusmartin/16x-neopixel/releases/latest/download/firmware.bin"

// ── Time ─────────────────────────────────────────────────────────────────────
// UTC offset in seconds. Mexico City = -6*3600 = -21600
#define TIMEZONE_OFFSET  (-6L * 3600L)

#endif
