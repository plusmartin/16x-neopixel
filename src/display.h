#ifndef DISPLAY_H
#define DISPLAY_H

#include <FastLED.h>
#include "configs.h"

extern CRGB leds[NUM_LEDS];

// Panel chain: all rows right→left, each driven by its own GPIO.
// Internal wiring: serpentine, pixel 0 at top-right.
//   Even rows (from top): right→left  →  offset = PANEL_W-1-lx
//   Odd rows  (from top): left→right  →  offset = lx
inline uint16_t xy(uint8_t x, uint8_t y)
{
  if (x >= MATRIX_W || y >= MATRIX_H) return 0;

  uint8_t panel_x = x / PANEL_W;
  uint8_t panel_y = y / PANEL_H;
  uint8_t lx = x % PANEL_W;
  uint8_t ly = (PANEL_H - 1) - (y % PANEL_H);

  uint16_t panel = panel_y * PANELS_X + (PANELS_X - 1 - panel_x);

  uint16_t local = (ly & 1) ? (ly * PANEL_W + lx)
                             : (ly * PANEL_W + (PANEL_W - 1 - lx));

  return panel * (PANEL_W * PANEL_H) + local;
}

inline void setPixel(uint8_t x, uint8_t y, CRGB color)
{
  leds[xy(x, y)] = color;
}

// Draws a crosshair at the center of the matrix — use to verify xy() mapping.
// Center column and center row light up in different colors.
// Lights each column of panels a different color to reveal physical chain order.
inline void testPanelColumns()
{
  FastLED.clear();
  uint16_t sz = PANEL_W * PANEL_H;
  for (uint8_t p = 0; p < 16; p++) {
    uint8_t col = p % PANELS_X;
    CRGB color = (col == 0) ? CRGB::Red :
                 (col == 1) ? CRGB::Green :
                 (col == 2) ? CRGB::Blue :
                               CRGB::White;
    for (uint16_t i = 0; i < sz; i++) leds[p * sz + i] = color;
  }
  FastLED.show();
}

inline void testCrosshair()
{
  FastLED.clear();
  for (uint8_t x = 0; x < MATRIX_W; x++) setPixel(x, MATRIX_H / 2, CRGB::Red);
  for (uint8_t y = 0; y < MATRIX_H; y++) setPixel(MATRIX_W / 2, y,  CRGB::Green);
  setPixel(0, 0, CRGB::Blue);
  FastLED.show();
}

#endif
