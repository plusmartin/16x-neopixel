#ifndef CLOCKS_H
#define CLOCKS_H

#include <math.h>
#include "display.h"
#include "text.h"

static const float CK_TWO_PI = 6.28318530f;

// ── helpers ──────────────────────────────────────────────────────────────────

static void ckHand(int cx, int cy, float deg, int len, CRGB c)
{
  float r = (deg - 90.0f) * CK_TWO_PI / 360.0f;
  drawLine(cx, cy, cx + (int)(len * cosf(r) + 0.5f),
                   cy + (int)(len * sinf(r) + 0.5f), c);
}

static void ckArc(int cx, int cy, int rad, float a0, float a1, CRGB c)
{
  int steps = max(4, (int)(rad * fabsf(a1 - a0) * 2.5f));
  for (int i = 0; i <= steps; i++) {
    float a = a0 + (a1 - a0) * i / (float)steps;
    int x = cx + (int)(rad * sinf(a) + 0.5f);
    int y = cy - (int)(rad * cosf(a) + 0.5f);
    if (x >= 0 && x < MATRIX_W && y >= 0 && y < MATRIX_H)
      setPixel(x, y, c);
  }
}

static uint8_t ckLerp(uint8_t a, uint8_t b, float t)
{
  return (uint8_t)(a + (int)((b - a) * t));
}

// ── 1. Analog clock ──────────────────────────────────────────────────────────
inline void animClockAnalog(int h, int m, int s)
{
  static uint32_t last = 0;
  if (millis() - last < 200) return;
  last = millis();

  clear();
  const int cx = 15, cy = 15, r = 14;

  // Tick marks
  for (int i = 0; i < 12; i++) {
    float a = i * CK_TWO_PI / 12.0f;
    uint8_t bri = (i % 3 == 0) ? 110 : 45;
    setPixel(cx + (int)(r * sinf(a) + 0.5f),
             cy - (int)(r * cosf(a) + 0.5f),
             CRGB(bri, bri, bri));
  }

  // Second dot (red) on rim
  float sa = s * CK_TWO_PI / 60.0f;
  setPixel(cx + (int)(r * sinf(sa) + 0.5f),
           cy - (int)(r * cosf(sa) + 0.5f),
           CRGB(220, 0, 0));

  // Minute hand — white, length 12
  ckHand(cx, cy, m * 6.0f, 12, CRGB(255, 255, 255));

  // Hour hand — yellow, length 7
  ckHand(cx, cy, (h % 12) * 30.0f + m * 0.5f, 7, CRGB(255, 200, 0));

  // Center
  setPixel(cx, cy, CRGB(255, 255, 255));
  show();
}

// ── 2. Concentric arcs ───────────────────────────────────────────────────────
inline void animClockArcs(int h, int m, int s)
{
  static uint32_t last = 0;
  if (millis() - last < 200) return;
  last = millis();

  clear();
  const int cx = 15, cy = 15;

  // Background rings (dim)
  ckArc(cx, cy, 14, 0, CK_TWO_PI, CRGB(25, 0,  0));
  ckArc(cx, cy, 10, 0, CK_TWO_PI, CRGB(0,  20, 25));
  ckArc(cx, cy,  6, 0, CK_TWO_PI, CRGB(20, 15, 0));

  // Seconds — outer, red (r=14)
  if (s > 0) ckArc(cx, cy, 14, 0, s * CK_TWO_PI / 60.0f, CRGB(220, 0, 0));

  // Minutes — middle, cyan (r=10)
  float mEnd = m * CK_TWO_PI / 60.0f;
  if (m > 0) ckArc(cx, cy, 10, 0, mEnd, CRGB(0, 200, 255));

  // Hours — inner, yellow (r=6)
  float hEnd = (h % 12) * CK_TWO_PI / 12.0f + (m / 60.0f) * CK_TWO_PI / 12.0f;
  if (h % 12 > 0 || m > 0) ckArc(cx, cy, 6, 0, hEnd, CRGB(255, 200, 0));

  setPixel(cx, cy, CRGB(80, 80, 80));
  show();
}

// ── 3. Binary BCD clock ──────────────────────────────────────────────────────
// 4 columns (H-tens, H-units, M-tens, M-units) of 3×3 dot bits, bottom=bit0
inline void animClockBinary(int h, int m)
{
  static uint32_t last = 0;
  if (millis() - last < 500) return;
  last = millis();

  clear();

  const int digits[4]  = { h / 10, h % 10, m / 10, m % 10 };
  const int maxBit[4]  = { 2, 4, 3, 4 };
  const int colX[4]    = { 4, 11, 20, 27 };
  const CRGB lit[4]    = {
    CRGB(0,   100, 255),  // H-tens: blue
    CRGB(0,   220, 255),  // H-units: cyan
    CRGB(255, 140,   0),  // M-tens: orange
    CRGB(255, 220,   0),  // M-units: yellow
  };
  // Bit rows (bottom=bit0, top=bit3)
  const int rowY[4] = { 27, 21, 15, 9 };

  for (int col = 0; col < 4; col++) {
    for (int bit = 0; bit < 4; bit++) {
      if (bit >= maxBit[col]) continue;
      bool on = (digits[col] >> bit) & 1;
      CRGB c = on ? lit[col] : CRGB(15, 15, 15);
      for (int dy = -1; dy <= 1; dy++)
        for (int dx = -1; dx <= 1; dx++)
          setPixel(colX[col] + dx, rowY[bit] + dy, c);
    }
  }

  // Column labels (dim)
  drawChar(2,  0, 'H', CRGB(30, 30, 60));
  drawChar(18, 0, 'M', CRGB(60, 30,  0));
  show();
}

// ── 4. Bar chart clock ───────────────────────────────────────────────────────
// Left strip = hours (0-23), right strip = minutes (0-59), digits in center
inline void animClockBars(int h, int m)
{
  static uint32_t last = 0;
  if (millis() - last < 500) return;
  last = millis();

  clear();

  int hFill = (h * MATRIX_H) / 24;
  int mFill = (m * MATRIX_H) / 60;

  // Left bar (x=0-4): hours, fills from bottom, blue→red
  for (int y = 0; y < MATRIX_H; y++) {
    int fy = MATRIX_H - 1 - y;
    CRGB c = (fy < hFill)
      ? CRGB(CHSV(ckLerp(0, 160, (float)fy / (MATRIX_H-1)), 230, 210))
      : CRGB(12, 12, 12);
    for (int x = 0; x <= 4; x++) setPixel(x, y, c);
  }

  // Right bar (x=27-31): minutes, fills from bottom, green→yellow
  for (int y = 0; y < MATRIX_H; y++) {
    int fy = MATRIX_H - 1 - y;
    CRGB c = (fy < mFill)
      ? CRGB(CHSV(ckLerp(60, 100, (float)fy / (MATRIX_H-1)), 230, 210))
      : CRGB(12, 12, 12);
    for (int x = 27; x < MATRIX_W; x++) setPixel(x, y, c);
  }

  // Hour digits centered (yellow)
  char hb[3], mb[3];
  snprintf(hb, 3, "%02d", h);
  snprintf(mb, 3, "%02d", m);
  drawChar(9,  4,  hb[0], CRGB(255, 200,   0));
  drawChar(15, 4,  hb[1], CRGB(255, 200,   0));
  drawChar(9,  21, mb[0], CRGB(  0, 200, 255));
  drawChar(15, 21, mb[1], CRGB(  0, 200, 255));

  // Separator dots
  setPixel(15, 14, CRGB(80, 80, 80));
  setPixel(15, 16, CRGB(80, 80, 80));
  show();
}

// ── 5. Pong clock ────────────────────────────────────────────────────────────
static float pk_x = 15.5f, pk_y = 16.0f, pk_vx = 0.55f, pk_vy = 0.38f;

inline void animClockPong(int h, int m)
{
  static uint32_t last = 0;
  if (millis() - last < 33) return;
  last = millis();

  // Move ball
  pk_x += pk_vx;
  pk_y += pk_vy;

  // Bounce: top/bottom walls
  if (pk_y <= 0)           { pk_y = 0;           pk_vy =  fabsf(pk_vy); }
  if (pk_y >= MATRIX_H-1)  { pk_y = MATRIX_H-1;  pk_vy = -fabsf(pk_vy); }

  // Left paddle tracks ball, bounce
  int lpy = constrain((int)(pk_y - 2), 0, MATRIX_H - 5);
  if (pk_x <= 1 && pk_y >= lpy && pk_y <= lpy + 4)
    { pk_x = 1; pk_vx = fabsf(pk_vx); }
  else if (pk_x < 0) { pk_x = 0; pk_vx = fabsf(pk_vx); }

  // Right paddle tracks ball, bounce
  int rpy = constrain((int)(pk_y - 2), 0, MATRIX_H - 5);
  if (pk_x >= MATRIX_W-2 && pk_y >= rpy && pk_y <= rpy + 4)
    { pk_x = MATRIX_W-2; pk_vx = -fabsf(pk_vx); }
  else if (pk_x > MATRIX_W-1) { pk_x = MATRIX_W-1; pk_vx = -fabsf(pk_vx); }

  clear();

  // Net (dotted center line)
  for (int y = 0; y < MATRIX_H; y += 3)
    setPixel(15, y, CRGB(30, 30, 30));

  // Paddles
  for (int i = 0; i < 5; i++) {
    setPixel(0,          lpy + i, CRGB(255, 255, 255));
    setPixel(MATRIX_W-1, rpy + i, CRGB(255, 255, 255));
  }

  // Ball
  setPixel((int)pk_x, (int)pk_y, CRGB(255, 255, 0));

  // Scores (time) at top — hours left, minutes right
  char hb[3], mb[3];
  snprintf(hb, 3, "%02d", h);
  snprintf(mb, 3, "%02d", m);
  drawChar(2,  1, hb[0], CRGB(0, 180, 255));
  drawChar(8,  1, hb[1], CRGB(0, 180, 255));
  drawChar(19, 1, mb[0], CRGB(0, 180, 255));
  drawChar(25, 1, mb[1], CRGB(0, 180, 255));
  show();
}

// ── 6. Color of day ──────────────────────────────────────────────────────────
// Full-screen background shifts through sky colors; time centered on top
inline void animClockColor(int h, int m)
{
  static uint32_t last = 0;
  if (millis() - last < 1000) return;
  last = millis();

  float tod = h + m / 60.0f; // 0.0–24.0

  // Piecewise sky color (hue, sat, val key stops)
  struct Stop { float t; uint8_t hu, sa, va; };
  static const Stop stops[] = {
    { 0,  160, 220,  15 },  // midnight — deep navy
    { 5,  160, 220,  25 },  // pre-dawn
    { 6,   20, 240, 110 },  // dawn — orange
    { 8,  140, 210, 180 },  // morning — sky blue
    { 12, 145, 170, 240 },  // noon — bright blue
    { 16, 135, 200, 210 },  // afternoon
    { 18,  18, 240, 160 },  // sunset — orange/red
    { 20, 210, 230,  70 },  // dusk — purple
    { 22, 165, 220,  30 },  // evening
    { 24, 160, 220,  15 },  // midnight again
  };
  const int N = 10;

  CHSV bg = CHSV(stops[0].hu, stops[0].sa, stops[0].va);
  for (int i = 0; i < N - 1; i++) {
    if (tod >= stops[i].t && tod < stops[i+1].t) {
      float t = (tod - stops[i].t) / (stops[i+1].t - stops[i].t);
      bg = CHSV(ckLerp(stops[i].hu, stops[i+1].hu, t),
                ckLerp(stops[i].sa, stops[i+1].sa, t),
                ckLerp(stops[i].va, stops[i+1].va, t));
      break;
    }
  }
  fill(CRGB(bg));

  // Gradient: slightly darker at bottom
  for (int y = 16; y < MATRIX_H; y++) {
    uint8_t dim = 255 - (y - 16) * 4;
    for (int x = 0; x < MATRIX_W; x++)
      backbuf[xy(x, y)].nscale8(dim);
  }

  // Time (white, centered)
  char buf[6];
  snprintf(buf, sizeof(buf), "%02d:%02d", h, m);
  drawTextCentered(buf, 13, CRGB(255, 255, 255));
  show();
}

#endif
