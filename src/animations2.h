#ifndef ANIMATIONS2_H
#define ANIMATIONS2_H

#include "display.h"
#include <math.h>

// ── 1. Bouncing Balls ─────────────────────────────────────────────────────────
inline void animBalls()
{
    static float    bx[5], by[5], vx[5], vy[5];
    static uint8_t  bh[5];
    static bool     init = false;
    static uint32_t last = 0;
    if (!init) {
        for (int i = 0; i < 5; i++) {
            bx[i] = random8(MATRIX_W); by[i] = random8(MATRIX_H);
            vx[i] = (0.4f + random8(4)*0.15f) * (random8(2) ? 1 : -1);
            vy[i] = (0.3f + random8(4)*0.15f) * (random8(2) ? 1 : -1);
            bh[i] = random8();
        }
        init = true;
    }
    if (millis() - last < 30) return;
    last = millis();
    for (uint16_t i = 0; i < NUM_LEDS; i++) backbuf[i].nscale8(200);
    for (int i = 0; i < 5; i++) {
        bx[i] += vx[i]; by[i] += vy[i];
        if (bx[i] < 0)           { bx[i] = 0;           vx[i] = -vx[i]; }
        if (bx[i] >= MATRIX_W-1) { bx[i] = MATRIX_W-1;  vx[i] = -vx[i]; }
        if (by[i] < 0)           { by[i] = 0;           vy[i] = -vy[i]; }
        if (by[i] >= MATRIX_H-1) { by[i] = MATRIX_H-1;  vy[i] = -vy[i]; }
        setPixel((int)bx[i], (int)by[i], CHSV(bh[i], 255, 255));
    }
    show();
}

// ── 2. Ripple ─────────────────────────────────────────────────────────────────
inline void animRipple()
{
    struct Ring { int8_t x, y; uint8_t age; bool active; };
    static Ring     rings[5];
    static bool     init = false;
    static uint32_t last = 0;
    if (!init) { for (auto& r : rings) r.active = false; init = true; }
    if (millis() - last < 60) return;
    last = millis();

    if (random8(4) == 0)
        for (auto& r : rings)
            if (!r.active) { r = { (int8_t)random8(MATRIX_W), (int8_t)random8(MATRIX_H), 0, true }; break; }

    for (uint16_t i = 0; i < NUM_LEDS; i++) backbuf[i].nscale8(210);
    for (auto& r : rings) {
        if (!r.active) continue;
        if (++r.age > 20) { r.active = false; continue; }
        uint8_t bright = 255 - r.age * 12;
        uint8_t hue    = r.x * 8 + r.y * 4;
        int steps = max(1, r.age * 4);
        for (int a = 0; a < steps; a++) {
            float angle = a * 2.0f * M_PI / steps;
            int x = r.x + (int)(r.age * cosf(angle));
            int y = r.y + (int)(r.age * sinf(angle));
            if (x >= 0 && x < MATRIX_W && y >= 0 && y < MATRIX_H)
                setPixel(x, y, CHSV(hue, 255, bright));
        }
    }
    show();
}

// ── 3. Fireworks ─────────────────────────────────────────────────────────────
inline void animFireworks()
{
    struct Fw { float x, y, vx, vy; uint8_t hue, life; };
    static Fw       parts[24];
    static bool     init = false;
    static uint32_t last = 0;
    if (!init) { for (auto& p : parts) p.life = 0; init = true; }
    if (millis() - last < 35) return;
    last = millis();

    for (uint16_t i = 0; i < NUM_LEDS; i++) backbuf[i].nscale8(190);

    bool any = false;
    for (auto& p : parts) if (p.life > 0) { any = true; break; }
    if (!any) {
        float ex = 4 + random8(MATRIX_W - 8);
        float ey = 4 + random8(MATRIX_H * 2 / 3);
        uint8_t hue = random8();
        for (auto& p : parts) {
            float angle = random8() * (2.0f * M_PI / 256.0f);
            float spd   = 0.3f + random8(6) * 0.1f;
            p = { ex, ey, cosf(angle)*spd, sinf(angle)*spd, hue, 250 };
        }
    }
    for (auto& p : parts) {
        if (!p.life) continue;
        p.x += p.vx; p.y += p.vy; p.vy += 0.04f;
        p.life = (uint8_t)qsub8(p.life, 7);
        if (p.x >= 0 && p.x < MATRIX_W && p.y >= 0 && p.y < MATRIX_H)
            setPixel((int)p.x, (int)p.y, CHSV(p.hue, 255, p.life));
    }
    show();
}

// ── 4. Twinkle Stars ─────────────────────────────────────────────────────────
inline void animTwinkle()
{
    struct Star { uint8_t x, y, bright; int8_t dir; };
    static Star     stars[24];
    static bool     init = false;
    static uint32_t last = 0;
    if (!init) {
        for (auto& s : stars) {
            s.x = random8(MATRIX_W); s.y = random8(MATRIX_H);
            s.bright = random8(); s.dir = random8(2) ? 5 : -5;
        }
        init = true;
    }
    if (millis() - last < 40) return;
    last = millis();

    clear();
    for (auto& s : stars) {
        int nb = (int)s.bright + s.dir;
        s.bright = (uint8_t)constrain(nb, 0, 255);
        if (s.bright == 0 || s.bright == 255) s.dir = -s.dir;
        if (s.bright == 0 && random8(6) == 0)
            { s.x = random8(MATRIX_W); s.y = random8(MATRIX_H); }
        setPixel(s.x, s.y, CRGB(s.bright, s.bright, s.bright));
    }
    show();
}

// ── 5. Kaleidoscope ──────────────────────────────────────────────────────────
inline void animKaleidoscope()
{
    uint32_t t = millis() / 5;
    for (int y = 0; y < MATRIX_H/2; y++) {
        for (int x = 0; x < MATRIX_W/2; x++) {
            uint8_t hue = inoise8(x * 25 + (uint16_t)t, y * 25, (uint16_t)(t / 4));
            uint8_t val = inoise8(x * 25 + 500,          y * 25 + 500, (uint16_t)(t / 4));
            val = scale8(val, val);
            CRGB c = CHSV(hue, 220, val);
            setPixel(x,            y,            c);
            setPixel(MATRIX_W-1-x, y,            c);
            setPixel(x,            MATRIX_H-1-y, c);
            setPixel(MATRIX_W-1-x, MATRIX_H-1-y, c);
        }
    }
    show();
}

// ── 6. Wave Interference ─────────────────────────────────────────────────────
inline void animWaveInterference()
{
    uint32_t t = millis() / 15;
    for (int y = 0; y < MATRIX_H; y++) {
        for (int x = 0; x < MATRIX_W; x++) {
            float d1 = sqrtf((x-6.f)*(x-6.f)   + (y-8.f)*(y-8.f));
            float d2 = sqrtf((x-25.f)*(x-25.f) + (y-24.f)*(y-24.f));
            float d3 = sqrtf((x-16.f)*(x-16.f) + (y-4.f)*(y-4.f));
            uint8_t v = (sin8((uint8_t)(d1*10 - t)) +
                         sin8((uint8_t)(d2*10 + t)) +
                         sin8((uint8_t)(d3*10 - t/2))) / 3;
            setPixel(x, y, CHSV(v + (uint8_t)(t/4), 255, v));
        }
    }
    show();
}

// ── 7. Psychedelic Tunnel ────────────────────────────────────────────────────
inline void animTunnel()
{
    uint32_t t = millis() / 12;
    for (int y = 0; y < MATRIX_H; y++) {
        for (int x = 0; x < MATRIX_W; x++) {
            float dx = x - 15.5f, dy = y - 15.5f;
            float dist  = sqrtf(dx*dx + dy*dy) * 8.0f;
            float angle = atan2f(dy, dx) * 40.0f;
            uint8_t hue    = (uint8_t)(angle + t/3);
            uint8_t bright = sin8((uint8_t)(dist - t));
            setPixel(x, y, CHSV(hue, 255, bright));
        }
    }
    show();
}

// ── 8. DNA Helix ─────────────────────────────────────────────────────────────
inline void animDNA()
{
    static uint32_t t    = 0;
    static uint32_t last = 0;
    if (millis() - last < 30) return;
    last = millis();
    t += 3;

    clear();
    for (int y = 0; y < MATRIX_H; y++) {
        int x1 = (sin8((uint8_t)(y * 10 + t))       * (MATRIX_W-1)) / 255;
        int x2 = (sin8((uint8_t)(y * 10 + t + 128)) * (MATRIX_W-1)) / 255;
        if (y % 4 == 0) {
            int mn = min(x1, x2), mx = max(x1, x2);
            for (int x = mn; x <= mx; x++) setPixel(x, y, CRGB(50, 50, 50));
        }
        setPixel(x1, y, CHSV((uint8_t)(t/4 + y*6),       255, 255));
        setPixel(x2, y, CHSV((uint8_t)(t/4 + y*6 + 128), 255, 255));
    }
    show();
}

// ── 9. Pendulum Wave ─────────────────────────────────────────────────────────
inline void animPendulum()
{
    static uint32_t t    = 0;
    static uint32_t last = 0;
    if (millis() - last < 20) return;
    last = millis();
    t++;

    clear();
    for (int x = 0; x < MATRIX_W; x++) {
        int y = (MATRIX_H-1) - ((int)sin8((uint8_t)(t * (8 + x))) * (MATRIX_H-1)) / 255;
        setPixel(x, y, CHSV((uint8_t)(x * 8), 255, 255));
    }
    show();
}

// ── 10. Rotating Cube ────────────────────────────────────────────────────────
inline void animCube()
{
    static uint32_t last = 0;
    if (millis() - last < 30) return;
    last = millis();

    float a = millis() * 0.001f,  b = millis() * 0.00073f;
    float ca = cosf(a), sa = sinf(a), cb = cosf(b), sb = sinf(b);

    static const float V[8][3] = {
        {-1,-1,-1},{1,-1,-1},{1,1,-1},{-1,1,-1},
        {-1,-1, 1},{1,-1, 1},{1,1, 1},{-1,1, 1}
    };
    static const uint8_t E[12][2] = {
        {0,1},{1,2},{2,3},{3,0},
        {4,5},{5,6},{6,7},{7,4},
        {0,4},{1,5},{2,6},{3,7}
    };

    int px[8], py[8];
    for (int i = 0; i < 8; i++) {
        float rx = V[i][0]*ca - V[i][2]*sa;
        float rz = V[i][0]*sa + V[i][2]*ca;
        float ry2 = V[i][1]*cb - rz*sb;
        float rz2 = V[i][1]*sb + rz*cb;
        float d = 4.0f + rz2;
        px[i] = 16 + (int)(rx  * 9.0f / d);
        py[i] = 16 + (int)(ry2 * 9.0f / d);
    }

    clear();
    uint8_t hue = (uint8_t)(millis() / 20);
    for (auto& e : E) {
        drawLine(px[e[0]], py[e[0]], px[e[1]], py[e[1]], CHSV(hue, 200, 255));
        hue += 21;
    }
    show();
}

// ── 11. Lissajous ────────────────────────────────────────────────────────────
inline void animLissajous()
{
    static uint8_t  t    = 0;
    static uint32_t last = 0;
    if (millis() - last < 30) return;
    last = millis();
    t += 2;

    for (uint16_t i = 0; i < NUM_LEDS; i++) backbuf[i].nscale8(200);
    for (int i = 0; i < 255; i++) {
        int x = (int)(sin8((uint8_t)(i * 3 + t))    * (MATRIX_W-1)) / 255;
        int y = (int)(sin8((uint8_t)(i * 2 + t/2))  * (MATRIX_H-1)) / 255;
        setPixel(x, y, CHSV((uint8_t)(i + t), 255, 255));
    }
    show();
}

// ── 12. Glitch ───────────────────────────────────────────────────────────────
inline void animGlitch()
{
    static uint32_t last      = 0;
    static uint32_t glitchEnd = 0;
    if (millis() - last < 16) return;
    last = millis();

    uint32_t t   = millis() / 20;
    bool     glitching = millis() < glitchEnd;

    for (int y = 0; y < MATRIX_H; y++) {
        int shift = (glitching && random8(5) == 0) ? (int8_t)(random8(9) - 4) : 0;
        for (int x = 0; x < MATRIX_W; x++) {
            int sx = (x + shift + MATRIX_W) % MATRIX_W;
            uint8_t hue = inoise8(sx * 18, y * 18, (uint16_t)t);
            setPixel(x, y, CHSV(hue, 200, 200));
        }
    }
    if (random8() < 4) {
        glitchEnd = millis() + random8(150) + 50;
        fillRect(random8(MATRIX_W-6), random8(MATRIX_H-4), random8(6)+2, random8(4)+2,
                 CHSV(random8(), 255, 255));
    }
    show();
}

// ── 13. Aurora ───────────────────────────────────────────────────────────────
inline void animAurora()
{
    uint32_t t = millis() / 30;
    for (int x = 0; x < MATRIX_W; x++) {
        uint8_t wave = sin8((uint8_t)(x * 7 + t));
        int cy = MATRIX_H/4 + (wave * MATRIX_H/2) / 255;
        for (int y = 0; y < MATRIX_H; y++) {
            int dist   = abs(y - cy);
            uint8_t br = (dist < 10) ? (uint8_t)(255 - dist * 26) : 0;
            uint8_t hue = 96 + (uint8_t)(x * 3) + sin8((uint8_t)(y * 8 + t/2)) / 12;
            setPixel(x, y, br > 10 ? CRGB(CHSV(hue, 230, br)) : CRGB::Black);
        }
    }
    show();
}

// ── 14. Hypnotic Spiral ──────────────────────────────────────────────────────
inline void animSpiral()
{
    uint32_t t = millis() / 15;
    for (int y = 0; y < MATRIX_H; y++) {
        for (int x = 0; x < MATRIX_W; x++) {
            float dx = x - 15.5f, dy = y - 15.5f;
            float r     = sqrtf(dx*dx + dy*dy);
            float theta = atan2f(dy, dx);
            uint8_t v   = sin8((uint8_t)(r * 12.0f - theta * 32.0f / (float)M_PI + t));
            setPixel(x, y, CHSV((uint8_t)(theta * 40.0f / (float)M_PI + t/3), 255, v));
        }
    }
    show();
}

// ── 15. Pulse Grid ───────────────────────────────────────────────────────────
inline void animPulse()
{
    uint32_t t = millis() / 8;
    for (int y = 0; y < MATRIX_H; y++) {
        for (int x = 0; x < MATRIX_W; x++) {
            float dx = x - 15.5f, dy = y - 15.5f;
            uint8_t dist = (uint8_t)(sqrtf(dx*dx + dy*dy) * 10.0f);
            uint8_t v    = sin8((uint8_t)(dist - t));
            setPixel(x, y, CHSV((uint8_t)(t/4 + dist), 255, v));
        }
    }
    show();
}

// ── 16. Color Noise ──────────────────────────────────────────────────────────
inline void animColorNoise()
{
    static uint16_t t = 0;
    t += 2;
    for (int y = 0; y < MATRIX_H; y++)
        for (int x = 0; x < MATRIX_W; x++) {
            uint8_t hue = inoise8(x * 22 + t, y * 22, t / 2);
            uint8_t val = inoise8(x * 22 + 1000, y * 22 + 1000, t / 2);
            setPixel(x, y, CHSV(hue, 255, scale8(val, val)));
        }
    show();
}

// ── 17. Lightning ────────────────────────────────────────────────────────────
inline void animLightning()
{
    static uint8_t  boltX[MATRIX_H];
    static uint8_t  bright   = 0;
    static uint32_t nextBolt = 0;
    static uint32_t last     = 0;
    if (millis() - last < 20) return;
    last = millis();

    uint32_t now = millis();

    // Dark sky
    for (int y = 0; y < MATRIX_H; y++)
        for (int x = 0; x < MATRIX_W; x++)
            setPixel(x, y, CRGB(3, 3, 10));

    // New bolt
    if (bright == 0 && now >= nextBolt) {
        boltX[0] = random8(4, MATRIX_W-4);
        for (int y = 1; y < MATRIX_H; y++)
            boltX[y] = (uint8_t)constrain((int)boltX[y-1] + (int8_t)(random8(5)-2), 0, MATRIX_W-1);
        bright    = 255;
        nextBolt  = now + 800 + (uint32_t)random8() * 8;  // 0.8–2.8 s between bolts
    }

    if (bright > 0) {
        for (int y = 0; y < MATRIX_H; y++)
            setPixel(boltX[y], y, CRGB(bright, bright, scale8(bright, 200)));
        bright = qsub8(bright, 20);  // fades in ~13 frames = ~260 ms
    }
    show();
}

// ── 18. Snow ─────────────────────────────────────────────────────────────────
inline void animSnow()
{
    static uint32_t last     = 0;
    static uint32_t lastCall = 0;
    uint32_t now = millis();
    if (now - lastCall > 12000) clear();  // fresh start after being dormant
    lastCall = now;
    if (now - last < 70) return;
    last = now;

    for (int y = MATRIX_H-2; y >= 0; y--) {
        for (int x = 0; x < MATRIX_W; x++) {
            if (backbuf[xy(x, y)].getLuma() < 50) continue;
            CRGB c = backbuf[xy(x, y)];
            if (backbuf[xy(x, y+1)].getLuma() < 30) {
                setPixel(x, y+1, c); setPixel(x, y, CRGB::Black);
            } else if (x > 0 && backbuf[xy(x-1, y+1)].getLuma() < 30) {
                setPixel(x-1, y+1, c); setPixel(x, y, CRGB::Black);
            } else if (x < MATRIX_W-1 && backbuf[xy(x+1, y+1)].getLuma() < 30) {
                setPixel(x+1, y+1, c); setPixel(x, y, CRGB::Black);
            }
        }
    }
    if (random8(3) == 0) setPixel(random8(MATRIX_W), 0, CRGB(200, 220, 255));
    if (random8(18) == 0)
        for (int x = 0; x < MATRIX_W; x++) setPixel(x, MATRIX_H-1, CRGB::Black);
    show();
}

// ── 19. Checkerboard Wave ────────────────────────────────────────────────────
inline void animCheckerWave()
{
    uint32_t t = millis() / 8;
    for (int y = 0; y < MATRIX_H; y++)
        for (int x = 0; x < MATRIX_W; x++) {
            uint8_t wave = sin8((uint8_t)(x*12 + t)) / 4 + sin8((uint8_t)(y*12 + t + 64)) / 4;
            bool    cell = ((x + y + wave/32) & 1);
            setPixel(x, y, cell ? CHSV((uint8_t)(t/3),       240, 255)
                                : CHSV((uint8_t)(t/3 + 128), 240, 180));
        }
    show();
}

// ── 20. Smoke ────────────────────────────────────────────────────────────────
inline void animSmoke()
{
    static uint8_t  grid[MATRIX_H][MATRIX_W] = {};
    static uint32_t last = 0;
    if (millis() - last < 50) return;
    last = millis();

    for (int y = 0; y < MATRIX_H-1; y++)
        for (int x = 0; x < MATRIX_W; x++) {
            uint16_t sum = (uint16_t)grid[y][x] * 2
                         + grid[y+1][x] * 3
                         + (x > 0           ? grid[y+1][x-1] : 0)
                         + (x < MATRIX_W-1  ? grid[y+1][x+1] : 0);
            grid[y][x] = (uint8_t)(sum / 7);
        }
    for (int i = 0; i < 3; i++)
        grid[MATRIX_H-1][(uint8_t)constrain((int)(MATRIX_W/2) + (int8_t)(random8(7)-3), 0, MATRIX_W-1)] = 255;

    for (int y = 0; y < MATRIX_H; y++)
        for (int x = 0; x < MATRIX_W; x++) {
            uint8_t v = grid[y][x];
            setPixel(x, y, CHSV(200 + v/16, (uint8_t)(180 - v/3), v));
        }
    show();
}

// ── 21. Langton's Ant ────────────────────────────────────────────────────────
inline void animLangton()
{
    static uint8_t  grid[MATRIX_H][MATRIX_W/8] = {};
    static uint8_t  ax = 16, ay = 16, adir = 0;
    static uint32_t steps = 0;
    static uint32_t last  = 0;
    if (millis() - last < 20) return;
    last = millis();

    for (int s = 0; s < 8; s++) {
        bool white = !((grid[ay][ax>>3] >> (ax&7)) & 1);
        if (white) { adir = (adir+1)&3; grid[ay][ax>>3] |=  (1<<(ax&7)); }
        else        { adir = (adir+3)&3; grid[ay][ax>>3] &= ~(1<<(ax&7)); }
        const int8_t dx[] = {0,1,0,-1}, dy[] = {-1,0,1,0};
        ax = (uint8_t)((ax + dx[adir] + MATRIX_W) % MATRIX_W);
        ay = (uint8_t)((ay + dy[adir] + MATRIX_H) % MATRIX_H);
        if (++steps > 40000) { memset(grid,0,sizeof(grid)); ax=16; ay=16; adir=0; steps=0; }
    }
    for (int y = 0; y < MATRIX_H; y++)
        for (int x = 0; x < MATRIX_W; x++) {
            bool on = (grid[y][x>>3] >> (x&7)) & 1;
            setPixel(x, y, on ? CRGB(200,180,80) : CRGB(15,10,0));
        }
    setPixel(ax, ay, CRGB::Red);
    show();
}

// ── 22. Pac-Dots ─────────────────────────────────────────────────────────────
inline void animPacDots()
{
    static uint8_t  dots[MATRIX_H][MATRIX_W/8];
    static float    px = 8, py = 8, pvx = 0.45f, pvy = 0.22f;
    static uint8_t  hue = 0;
    static bool     init = false;
    static uint32_t last = 0;
    if (!init) { memset(dots, 0xFF, sizeof(dots)); init = true; }
    if (millis() - last < 55) return;
    last = millis();

    px += pvx; py += pvy;
    if (px < 0 || px >= MATRIX_W) { pvx = -pvx; px += pvx; hue += 30; }
    if (py < 0 || py >= MATRIX_H) { pvy = -pvy; py += pvy; hue += 30; }
    int ix = (int)px, iy = (int)py;
    dots[iy][ix>>3] &= ~(1<<(ix&7));

    bool any = false;
    for (int i = 0; i < (int)sizeof(dots); i++) if (((uint8_t*)dots)[i]) { any=true; break; }
    if (!any) memset(dots, 0xFF, sizeof(dots));

    clear();
    for (int y = 0; y < MATRIX_H; y++)
        for (int x = 0; x < MATRIX_W; x++)
            if ((dots[y][x>>3] >> (x&7)) & 1) setPixel(x, y, CRGB(130,130,0));
    setPixel(ix, iy, CHSV(hue, 255, 255));
    show();
}

#endif
