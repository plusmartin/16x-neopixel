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

// ── fire ─────────────────────────────────────────────────────────────────────
// Classic fire2012 algorithm per-column, rendered with FastLED HeatColor().
// COOLING 55, SPARKING 120 → visible gradient from hot base to cold top.
static const uint8_t FIRE_COOLING  = 30;
static const uint8_t FIRE_SPARKING = 160;

inline void animFire()
{
  static uint8_t  heat[MATRIX_H * MATRIX_W] = {};
  static uint32_t last = 0;
  if (millis() - last < 30) return;
  last = millis();

  for (int x = 0; x < MATRIX_W; x++) {

    // 1. Cool every cell in this column
    for (int y = 0; y < MATRIX_H; y++)
      heat[y * MATRIX_W + x] = qsub8(heat[y * MATRIX_W + x],
                                      random8(0, ((FIRE_COOLING * 10) / MATRIX_H) + 2));

    // 2. Heat drifts up: each cell averages the two cells below it
    for (int y = 0; y < MATRIX_H - 2; y++)
      heat[y * MATRIX_W + x] = ((uint16_t)heat[(y+1)*MATRIX_W+x]
                                          + heat[(y+1)*MATRIX_W+x]
                                          + heat[(y+2)*MATRIX_W+x]) / 3;

    // 3. Randomly ignite new sparks at the bottom
    if (random8() < FIRE_SPARKING)
      heat[(MATRIX_H-1)*MATRIX_W+x] = qadd8(heat[(MATRIX_H-1)*MATRIX_W+x],
                                             random8(160, 255));
  }

  // 4. Draw — custom palette with exaggerated orange/yellow transition
  for (int y = 0; y < MATRIX_H; y++) {
    for (int x = 0; x < MATRIX_W; x++) {
      uint8_t h = heat[y * MATRIX_W + x];
      CRGB c;
      if      (h == 0)   c = CRGB::Black;
      else if (h < 90)   c = CRGB(h * 2 + 35,  0,          0);   // black → red
      else if (h < 170)  c = CRGB(255,          (h-90)*3,   0);   // red → orange → yellow
      else               c = CRGB(255,          255,        (h-170)*3); // yellow → white
      setPixel(x, y, c);
    }
  }
  show();
}

// ── matrix rain ──────────────────────────────────────────────────────────────
struct MatrixDrop { float y; float speed; uint8_t len; bool active; };

inline void animMatrixRain()
{
  static MatrixDrop drops[MATRIX_W];
  static bool     init = false;
  static uint32_t last = 0;

  if (!init) {
    for (int x = 0; x < MATRIX_W; x++) {
      drops[x] = { (float)random8(MATRIX_H), 0.4f + random8(6)*0.1f,
                   (uint8_t)(5 + random8(10)), (bool)random8(2) };
    }
    init = true;
  }

  if (millis() - last < 40) return;
  last = millis();

  for (uint16_t i = 0; i < NUM_LEDS; i++) backbuf[i].nscale8(170);

  for (int x = 0; x < MATRIX_W; x++) {
    if (!drops[x].active) {
      if (random8(100) < 6) { drops[x].active = true; drops[x].y = 0;
                               drops[x].speed = 0.4f + random8(8)*0.1f; }
      continue;
    }
    drops[x].y += drops[x].speed;
    if (drops[x].y > MATRIX_H + drops[x].len) drops[x].active = false;
    int head = (int)drops[x].y;
    if (head >= 0 && head < MATRIX_H) setPixel(x, head, CRGB(180, 255, 180));
    if (head-1 >= 0 && head-1 < MATRIX_H) setPixel(x, head-1, CRGB(80, 200, 80));
  }
  show();
}

// ── starfield ────────────────────────────────────────────────────────────────
inline void animStarfield()
{
  static float    sx[80], sy[80], sz[80];
  static bool     init  = false;
  static uint32_t last  = 0;

  if (!init) {
    for (int i = 0; i < 80; i++) {
      sx[i] = (random8() - 128) / 128.0f;
      sy[i] = (random8() - 128) / 128.0f;
      sz[i] = random8() / 255.0f + 0.02f;
    }
    init = true;
  }

  if (millis() - last < 30) return;
  last = millis();

  const float cx = MATRIX_W / 2.0f, cy = MATRIX_H / 2.0f;
  const float fov = MATRIX_W / 2.0f, speed = 0.018f;

  for (uint16_t i = 0; i < NUM_LEDS; i++) backbuf[i].nscale8(210);
  for (int i = 0; i < 80; i++) {
    sz[i] -= speed;
    if (sz[i] < 0.02f) {
      sx[i] = (random8() - 128) / 128.0f;
      sy[i] = (random8() - 128) / 128.0f;
      sz[i] = 1.0f;
    }
    int px = (int)(cx + sx[i] / sz[i] * fov);
    int py = (int)(cy + sy[i] / sz[i] * fov);
    if (px >= 0 && px < MATRIX_W && py >= 0 && py < MATRIX_H) {
      uint8_t b = 50 + (uint8_t)(205.0f * (1.0f - sz[i]));
      setPixel(px, py, CRGB(b, b, b));
    }
  }
  show();
}

// ── meteor shower ────────────────────────────────────────────────────────────
struct Meteor { float x, y, dx, dy; uint8_t len; CRGB color; bool active; };

inline void animMeteorShower()
{
  static Meteor   meteors[6];
  static bool     init = false;
  static uint32_t last = 0;

  if (!init) { for (auto& m : meteors) m.active = false; init = true; }

  if (millis() - last < 25) return;
  last = millis();

  for (uint16_t i = 0; i < NUM_LEDS; i++) backbuf[i].nscale8(195);

  for (auto& m : meteors) {
    if (!m.active && random8(100) < 8) {
      m = { (float)random8(MATRIX_W), 0.0f,
            (random8(2) ? 1.0f : -1.0f) * (0.4f + random8(4)*0.15f),
            0.6f + random8(5)*0.15f,
            (uint8_t)(5 + random8(8)),
            CHSV(random8(), 200, 255), true };
    }
    if (!m.active) continue;
    m.x += m.dx; m.y += m.dy;
    if (m.y > MATRIX_H + m.len || m.x < -m.len || m.x > MATRIX_W + m.len)
      { m.active = false; continue; }
    int px = (int)m.x, py = (int)m.y;
    if (px >= 0 && px < MATRIX_W && py >= 0 && py < MATRIX_H)
      setPixel(px, py, m.color);
  }
  show();
}

// ── runner ───────────────────────────────────────────────────────────────────
inline void runAnimations()
{
  uint8_t slot = (millis() / 8000) % 8;
  switch (slot) {
    case 0: animRainbow();              break;
    case 1: animPlasma();               break;
    case 2: animBreathe(CRGB::Cyan);    break;
    case 3: animWipe();                 break;
    case 4: animSparkle();              break;
    case 5: animFire();                 break;
    case 6: animMatrixRain();           break;
    case 7: animStarfield();            break;
  }
}

#endif
