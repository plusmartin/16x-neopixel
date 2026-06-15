#ifndef DISPLAY_H
#define DISPLAY_H

#include <FastLED.h>
#include "configs.h"
#include "calibration.h"

// Front buffer — written to FastLED on show().
extern CRGB leds[NUM_LEDS];

// Back buffer — draw here, then call show() to flip.
extern CRGB backbuf[NUM_LEDS];

// ── coordinate mapping ────────────────────────────────────────────────────────
// Panel chain: one per row, panels ordered right→left within each chain.
// Internal: every data row goes right→left, rows run bottom→top.
// pixel 0 of each panel = physical bottom-right corner.
inline uint16_t xy(uint8_t x, uint8_t y)
{
  if (x >= MATRIX_W || y >= MATRIX_H) return 0;
  uint8_t panel_x = x / PANEL_W;
  uint8_t panel_y = y / PANEL_H;
  uint8_t lx      = x % PANEL_W;
  uint8_t ly      = (PANEL_H - 1) - (y % PANEL_H);
  uint16_t panel  = panel_y * PANELS_X + (PANELS_X - 1 - panel_x);
  uint16_t local  = ly * PANEL_W + (PANEL_W - 1 - lx);
  return panel * (PANEL_W * PANEL_H) + local;
}

// ── primitives (draw into backbuf) ───────────────────────────────────────────
inline void setPixel(uint8_t x, uint8_t y, CRGB color)
{
  backbuf[xy(x, y)] = color;
}

inline void clear()
{
  memset(backbuf, 0, sizeof(backbuf));
}

inline void fill(CRGB color)
{
  for (uint16_t i = 0; i < NUM_LEDS; i++) backbuf[i] = color;
}

inline void drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, CRGB color)
{
  for (uint8_t i = x; i < x + w; i++) { setPixel(i, y,         color); setPixel(i, y+h-1,     color); }
  for (uint8_t j = y; j < y + h; j++) { setPixel(x,     j,     color); setPixel(x+w-1, j,     color); }
}

inline void fillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, CRGB color)
{
  for (uint8_t j = y; j < y + h; j++)
    for (uint8_t i = x; i < x + w; i++)
      setPixel(i, j, color);
}

inline void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, CRGB color)
{
  int16_t dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int16_t dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int16_t err = (dx > dy ? dx : -dy) / 2;
  while (true) {
    if (x0 >= 0 && x0 < MATRIX_W && y0 >= 0 && y0 < MATRIX_H)
      setPixel(x0, y0, color);
    if (x0 == x1 && y0 == y1) break;
    int16_t e2 = err;
    if (e2 > -dx) { err -= dy; x0 += sx; }
    if (e2 <  dy) { err += dx; y0 += sy; }
  }
}

// ── flip back→front and push to hardware ─────────────────────────────────────
inline void show()
{
  memcpy(leds, backbuf, sizeof(leds));
  calApply(leds);

  // Power budget: total channel sum must not exceed all LEDs at white 180/255.
  // If over budget, scale every LED down proportionally so bright pixels stay
  // bright relative to each other but the total draw is within limits.
  static const uint32_t POWER_BUDGET = (uint32_t)NUM_LEDS * 3 * 70;
  uint32_t total = 0;
  for (int i = 0; i < NUM_LEDS; i++)
    total += leds[i].r + leds[i].g + leds[i].b;
  if (total > POWER_BUDGET) {
    uint8_t scale = (uint8_t)((POWER_BUDGET * 255UL) / total);
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i].r = scale8(leds[i].r, scale);
      leds[i].g = scale8(leds[i].g, scale);
      leds[i].b = scale8(leds[i].b, scale);
    }
  }

  FastLED.show();
}

// ── crosshair test (validates xy mapping) ────────────────────────────────────
inline void testCrosshair()
{
  clear();
  for (uint8_t x = 0; x < MATRIX_W; x++) setPixel(x, MATRIX_H / 2, CRGB::Red);
  for (uint8_t y = 0; y < MATRIX_H; y++) setPixel(MATRIX_W / 2, y, CRGB::Green);
  setPixel(0, 0, CRGB::Blue);
  show();
}

#endif
