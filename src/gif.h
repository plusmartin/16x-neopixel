#ifndef GIF_H
#define GIF_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <LittleFS.h>
#include <AnimatedGIF.h>
#include "display.h"
#include "configs.h"

#define GIF_DIR      "/gifs"
#define GIF_SLOTS    8
#define GIF_MAX_BYTES 150000  // reject GIFs over 150KB

static AnimatedGIF  g_gif;
static int          g_gifW, g_gifH;
static uint8_t      g_gifSlot   = 0;
static bool         g_fsReady   = false;
static bool         g_gifActive = false;
static uint32_t     g_gifNext   = 0;
static uint8_t     *g_gifBuf    = nullptr;
static uint8_t     *g_frameBuf  = nullptr;
static uint16_t    *g_lastPal   = nullptr;  // palette from last draw callback
static bool         g_gifFailed = false;

// Callback: only saves the current palette pointer.
// Actual rendering happens after playFrame() from the composited g_frameBuf.
static void _gifDraw(GIFDRAW *pDraw)
{
    if (pDraw && pDraw->pPalette) g_lastPal = pDraw->pPalette;
}

// ── Init filesystem ───────────────────────────────────────────────────────────
inline void gifInit()
{
    g_gif.begin(GIF_PALETTE_RGB565_LE);  // initializes pLineBufAligned — required before open()
    g_fsReady = LittleFS.begin(true);
    if (g_fsReady && !LittleFS.exists(GIF_DIR))
        LittleFS.mkdir(GIF_DIR);
}

// ── Download one GIF from the Worker into the next slot ──────────────────────
inline bool gifFetch()
{
    if (!g_fsReady || WiFi.status() != WL_CONNECTED) return false;

    char path[40];
    snprintf(path, sizeof(path), GIF_DIR "/gif_%d.gif", g_gifSlot);
    g_gifSlot = (g_gifSlot + 1) % GIF_SLOTS;

    HTTPClient http;
    http.begin(GIF_WORKER_URL);
    http.setTimeout(20000);
    int code = http.GET();
    if (code != 200) {
        Serial0.printf("[gif] fetch HTTP %d\n", code);
        http.end(); return false;
    }

    File f = LittleFS.open(path, FILE_WRITE);
    if (!f) {
        Serial0.printf("[gif] open write failed: %s\n", path);
        http.end(); return false;
    }

    int written = http.writeToStream(&f);
    f.close();
    http.end();
    Serial0.printf("[gif] saved %s (%d bytes)\n", path, written);
    return written > 0;
}

// ── Count available GIF files ─────────────────────────────────────────────────
inline int gifCount()
{
    File dir = LittleFS.open(GIF_DIR);
    if (!dir || !dir.isDirectory()) return 0;
    int n = 0;
    File f = dir.openNextFile();
    while (f) { if (!f.isDirectory()) n++; f.close(); f = dir.openNextFile(); }
    dir.close();
    return n;
}

// ── Stop the currently playing GIF ───────────────────────────────────────────
inline void gifStop()
{
    if (g_gifActive) { g_gif.close(); g_gifActive = false; }
    if (g_gifBuf)   { free(g_gifBuf);   g_gifBuf   = nullptr; }
    if (g_frameBuf) { free(g_frameBuf); g_frameBuf = nullptr; }
    g_lastPal   = nullptr;
    g_gifFailed = false;
}

// ── Non-blocking GIF player — loads file to RAM, decodes from memory ─────────
inline void animGif()
{
    if (!g_fsReady || g_gifFailed) return;

    if (!g_gifActive) {
        // Pick a random file via directory scan
        String names[GIF_SLOTS];
        int n = 0;
        File dir = LittleFS.open(GIF_DIR);
        if (dir && dir.isDirectory()) {
            File f = dir.openNextFile();
            while (f && n < GIF_SLOTS) {
                if (!f.isDirectory()) names[n++] = String(GIF_DIR "/") + f.name();
                f.close(); f = dir.openNextFile();
            }
            dir.close();
        }
        if (n == 0) { g_gifFailed = true; return; }  // stop spam; main loop retries fetch

        char p[40];
        strncpy(p, names[random8(n)].c_str(), sizeof(p) - 1);

        // Load entire file into heap buffer
        File f = LittleFS.open(p, "r");
        if (!f) return;
        size_t fsize = f.size();
        Serial0.printf("[gif] loading %s (%u bytes)\n", p, fsize);

        if (fsize == 0 || fsize > GIF_MAX_BYTES) {
            f.close();
            Serial0.printf("[gif] file too large (%u), deleting\n", fsize);
            LittleFS.remove(p);
            return;
        }

        g_gifBuf = (uint8_t*)malloc(fsize);
        if (!g_gifBuf) {
            Serial0.printf("[gif] malloc failed (free heap: %u)\n", esp_get_free_heap_size());
            f.close(); return;
        }
        f.read(g_gifBuf, fsize);
        f.close();

        // Open from memory — no file callbacks needed
        if (!g_gif.open(g_gifBuf, fsize, _gifDraw)) {
            Serial0.println("[gif] open FAILED");
            free(g_gifBuf); g_gifBuf = nullptr; return;
        }

        g_gifW = g_gif.getCanvasWidth();
        g_gifH = g_gif.getCanvasHeight();
        Serial0.printf("[gif] canvas %dx%d\n", g_gifW, g_gifH);

        // Reject if canvas too large for frameBuf (need w*h bytes of heap)
        if (g_gifW == 0 || g_gifH == 0 || g_gifW > 200 || g_gifH > 200) {
            Serial0.printf("[gif] canvas too large, deleting %s\n", p);
            g_gif.close(); free(g_gifBuf); g_gifBuf = nullptr;
            LittleFS.remove(p);
            g_gifFailed = true; return;
        }

        // v2.2.2 requires a frame buffer for multi-frame compositing
        g_frameBuf = (uint8_t*)malloc(g_gifW * g_gifH);
        if (!g_frameBuf) {
            Serial0.printf("[gif] frameBuf malloc failed (heap=%u)\n", esp_get_free_heap_size());
            g_gif.close(); free(g_gifBuf); g_gifBuf = nullptr;
            LittleFS.remove(p);
            g_gifFailed = true; return;
        }
        memset(g_frameBuf, 0, g_gifW * g_gifH);
        g_gif.setFrameBuf(g_frameBuf);

        g_gifActive = true;
        g_gifNext   = 0;
    }

    uint32_t now = millis();
    if (now < g_gifNext) return;

    int frameDelay = 0;
    bool ok = g_gif.playFrame(false, &frameDelay);
    if (!ok) { g_gif.reset(); g_gif.playFrame(false, &frameDelay); }

    // Render entire composited frame from g_frameBuf (no clear needed — we write every pixel)
    if (g_frameBuf && g_lastPal) {
        for (int dstY = 0; dstY < MATRIX_H; dstY++) {
            int srcY = (dstY * g_gifH) / MATRIX_H;
            for (int dstX = 0; dstX < MATRIX_W; dstX++) {
                int srcX = (dstX * g_gifW) / MATRIX_W;
                uint8_t  idx = g_frameBuf[srcY * g_gifW + srcX];
                uint16_t c   = g_lastPal[idx];
                setPixel(dstX, dstY, CRGB(
                    ((c >> 11) & 0x1F) << 3,
                    ((c >>  5) & 0x3F) << 2,
                    ( c        & 0x1F) << 3));
            }
        }
        show();
    }
    g_gifNext = now + (uint32_t)constrain(frameDelay, 20, 500);
}

#endif
