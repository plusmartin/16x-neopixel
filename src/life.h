#ifndef LIFE_H
#define LIFE_H

#include "display.h"

// Three 32-bit-per-row grids: two live buffers + one snapshot 2 gens ago.
static uint32_t lifeGrid[2][MATRIX_H];
static uint32_t lifePrev2[MATRIX_H];   // grid state from 2 generations ago
static uint8_t  lifeCur      = 0;
static uint32_t lifeLastTick = 0;
static uint16_t lifeGen      = 0;
static uint8_t  lifeStale    = 0;  // consecutive unchanged generations

// ── seeds ─────────────────────────────────────────────────────────────────────
static void lifePlaceCells(const int8_t* xy, uint8_t count, uint8_t cx, uint8_t cy)
{
  for (uint8_t i = 0; i < count; i++) {
    uint8_t x = (cx + xy[i * 2])     % MATRIX_W;
    uint8_t y = (cy + xy[i * 2 + 1]) % MATRIX_H;
    lifeGrid[lifeCur][y] |= (1UL << x);
  }
}

static void lifeSeedRandom()
{
  for (uint8_t y = 0; y < MATRIX_H; y++) {
    lifeGrid[lifeCur][y] = 0;
    for (uint8_t x = 0; x < MATRIX_W; x++)
      if (random8(100) < 30)
        lifeGrid[lifeCur][y] |= (1UL << x);
  }
}

static void lifeSeedAcorn()
{
  memset(lifeGrid[lifeCur], 0, sizeof(lifeGrid[0]));
  static const int8_t cells[] = {
    1,0,  3,1,  0,2, 1,2, 4,2, 5,2, 6,2
  };
  lifePlaceCells(cells, 7, MATRIX_W / 2 - 3, MATRIX_H / 2 - 1);
}

static void lifeSeedRPentomino()
{
  memset(lifeGrid[lifeCur], 0, sizeof(lifeGrid[0]));
  static const int8_t cells[] = {
    1,0, 2,0,  0,1, 1,1,  1,2
  };
  lifePlaceCells(cells, 5, MATRIX_W / 2 - 1, MATRIX_H / 2 - 1);
}

static void lifeSeedGliderGun()
{
  // Simkin glider gun — fits in 33×16, centred on a 32×32 with horizontal wrap.
  memset(lifeGrid[lifeCur], 0, sizeof(lifeGrid[0]));
  static const int8_t cells[] = {
     0,0,  1,0,  0,1,  1,1,
     8,0,  8,1,  9,1, 10,2,  9,0, 11,1, 12,1, 13,1, 14,0, 15,0, 16,1,
    17,2, 17,6,  16,5, 15,5, 14,6,
    13,5, 12,4, 11,4, 10,5,  9,5,  8,6,  8,5
  };
  lifePlaceCells(cells, 27, 0, MATRIX_H / 2 - 4);
}

// ── core ──────────────────────────────────────────────────────────────────────
static uint8_t lifeNeighbors(uint8_t x, uint8_t y)
{
  uint8_t n = 0;
  for (int8_t dy = -1; dy <= 1; dy++) {
    uint32_t row = lifeGrid[lifeCur][(y + dy + MATRIX_H) % MATRIX_H];
    for (int8_t dx = -1; dx <= 1; dx++) {
      if (dx == 0 && dy == 0) continue;
      n += (row >> ((x + dx + MATRIX_W) % MATRIX_W)) & 1;
    }
  }
  return n;
}

static void lifeStep()
{
  uint8_t next = lifeCur ^ 1;
  for (uint8_t y = 0; y < MATRIX_H; y++) {
    lifeGrid[next][y] = 0;
    for (uint8_t x = 0; x < MATRIX_W; x++) {
      uint8_t n     = lifeNeighbors(x, y);
      uint8_t alive = (lifeGrid[lifeCur][y] >> x) & 1;
      if (alive ? (n == 2 || n == 3) : (n == 3))
        lifeGrid[next][y] |= (1UL << x);
    }
  }
  // Stagnant if period-1 (static) or period-2 (oscillator like blinker)
  bool p1 = memcmp(lifeGrid[next], lifeGrid[lifeCur], sizeof(lifeGrid[0])) == 0;
  bool p2 = memcmp(lifeGrid[next], lifePrev2,         sizeof(lifeGrid[0])) == 0;
  lifeStale = (p1 || p2) ? lifeStale + 1 : 0;
  memcpy(lifePrev2, lifeGrid[lifeCur], sizeof(lifePrev2));
  lifeCur = next;
  lifeGen++;
}

static void lifeInit(uint8_t seed)
{
  memset(lifeGrid, 0, sizeof(lifeGrid));
  memset(lifePrev2, 0, sizeof(lifePrev2));
  lifeCur = lifeStale = lifeGen = 0;
  switch (seed % 4) {
    case 0: lifeSeedRandom();     break;
    case 1: lifeSeedAcorn();      break;
    case 2: lifeSeedRPentomino(); break;
    case 3: lifeSeedGliderGun();  break;
  }
}

// ── animation entry point ────────────────────────────────────────────────────
// Call from loop(). Cycles seeds automatically on extinction or stagnation.
inline void animLife(uint16_t ms_per_gen = 80)
{
  static bool    inited  = false;
  static uint8_t seedIdx = 0;

  if (!inited) { lifeInit(seedIdx); inited = true; }

  uint32_t now = millis();
  if (now - lifeLastTick < ms_per_gen) return;
  lifeLastTick = now;

  // Population check: count set bits across all rows
  uint32_t pop = 0;
  for (uint8_t y = 0; y < MATRIX_H; y++) pop += __builtin_popcount(lifeGrid[lifeCur][y]);

  bool extinct   = (pop == 0);
  bool stagnant  = (lifeStale >= 4);
  bool exhausted = (lifeGen >= 600);

  if (extinct || stagnant || exhausted) {
    lifeInit(++seedIdx);
    return;
  }

  lifeStep();

  // Render: hue shifts slowly with generation, brighter = more crowded
  clear();
  uint8_t hue = lifeGen * 2;
  for (uint8_t y = 0; y < MATRIX_H; y++) {
    uint32_t row = lifeGrid[lifeCur][y];
    while (row) {
      uint8_t x = __builtin_ctz(row);
      setPixel(x, y, CHSV(hue + x * 4, 220, 255));
      row &= row - 1;
    }
  }
  show();
}

#endif
