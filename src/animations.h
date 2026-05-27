#ifndef ANIMATIONS_H
#define ANIMATIONS_H

#include "display.h"

// ── rainbow ──────────────────────────────────────────────────────────────────
// Full hue spectrum shifts diagonally across the matrix over time.
inline void animRainbow()
{
  uint32_t t = millis() / 10;
  for (uint8_t y = 0; y < MATRIX_H; y++)
    for (uint8_t x = 0; x < MATRIX_W; x++)
      setPixel(x, y, CHSV((t + x * 8 + y * 8) & 0xFF, 255, 255));
  show();
}

// ── plasma ───────────────────────────────────────────────────────────────────
// Perlin noise colour field. Brightness follows a second noise layer so peaks
// glow and troughs dim — scale8(n,n) squares the value for visible contrast.
inline void animPlasma()
{
  uint32_t t = millis() / 8;
  for (uint8_t y = 0; y < MATRIX_H; y++) {
    for (uint8_t x = 0; x < MATRIX_W; x++) {
      uint8_t hue = inoise8(x * 20,       y * 20,       t);
      uint8_t val = inoise8(x * 20 + 200, y * 20 + 200, t + 500);
      val = scale8(val, val);  // square for pronounced bright/dark contrast
      setPixel(x, y, CHSV(hue, 220, val));
    }
  }
  show();
}

// ── breathe ──────────────────────────────────────────────────────────────────
// Single hue slowly fades in and out.
inline void animBreathe(CRGB color)
{
  uint8_t b = beatsin8(12, 10, 255);
  fill(color.nscale8(b));
  show();
}

// ── color wipe ───────────────────────────────────────────────────────────────
// Column-by-column sweep, alternating colour every pass.
inline void animWipe()
{
  static uint8_t  col   = 0;
  static uint8_t  hue   = 0;
  static uint32_t last  = 0;
  if (millis() - last < 40) return;
  last = millis();

  CRGB color = CHSV(hue, 255, 255);
  for (uint8_t y = 0; y < MATRIX_H; y++) setPixel(col, y, color);
  show();
  if (++col >= MATRIX_W) { col = 0; hue += 32; }
}

// ── sparkle ──────────────────────────────────────────────────────────────────
// Random pixels flash white then decay.
inline void animSparkle()
{
  // Decay all pixels slightly each frame
  for (uint16_t i = 0; i < NUM_LEDS; i++) backbuf[i].nscale8(210);

  // Randomly ignite new sparks
  uint8_t sparks = random8(3, 8);
  for (uint8_t s = 0; s < sparks; s++) {
    uint8_t x = random8(MATRIX_W);
    uint8_t y = random8(MATRIX_H);
    setPixel(x, y, CRGB::White);
  }
  show();
}

// ── runner ───────────────────────────────────────────────────────────────────
// Cycles through animations, 8 s each.
inline void runAnimations()
{
  uint32_t t    = millis();
  uint8_t  slot = (t / 8000) % 5;

  switch (slot) {
    case 0: animRainbow();              break;
    case 1: animPlasma();               break;
    case 2: animBreathe(CRGB::Cyan);    break;
    case 3: animWipe();                 break;
    case 4: animSparkle();              break;
  }
}

#endif
