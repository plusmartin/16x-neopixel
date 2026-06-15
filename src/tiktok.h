#ifndef TIKTOK_H
#define TIKTOK_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "configs.h"

static uint32_t g_ttFollowers = 0;
static uint32_t g_ttLikes     = 0;
static bool     g_ttNewData   = false;
static char     g_ttMsg[128]  = "";

inline void tiktokForceDisplay()
{
    if (g_ttFollowers == 0 && g_ttLikes == 0) return;  // no data yet
    g_ttNewData = true;
}

inline void tiktokCheck()
{
    if (WiFi.status() != WL_CONNECTED) return;

    HTTPClient http;
    http.begin(TIKTOK_WORKER_URL);
    http.setTimeout(12000);
    int code = http.GET();
    if (code != 200) {
        Serial0.printf("[tiktok] fetch failed: %d\n", code);
        http.end(); return;
    }

    String body = http.getString();
    http.end();

    // Parse {"followers":N,"likes":N} — no JSON library needed
    int fi = body.indexOf("\"followers\":");
    int li = body.indexOf("\"likes\":");
    if (fi < 0 || li < 0) {
        Serial0.printf("[tiktok] unexpected response: %s\n", body.substring(0,80).c_str());
        return;
    }

    uint32_t newF = (uint32_t)body.substring(fi + 12).toInt();
    uint32_t newL = (uint32_t)body.substring(li + 8).toInt();

    Serial0.printf("[tiktok] followers=%u likes=%u\n", newF, newL);

    if (newF != g_ttFollowers || newL != g_ttLikes) {
        g_ttFollowers = newF;
        g_ttLikes     = newL;

        auto fmtNum = [](uint32_t n, char* buf, int sz) {
            if      (n >= 1000000) snprintf(buf, sz, "%.1fM", n / 1000000.0f);
            else if (n >= 1000)    snprintf(buf, sz, "%.1fK", n / 1000.0f);
            else                   snprintf(buf, sz, "%u", n);
        };
        char fs[16], ls[16];
        fmtNum(newF, fs, sizeof(fs));
        fmtNum(newL, ls, sizeof(ls));
        snprintf(g_ttMsg, sizeof(g_ttMsg),
                 "  TikTok @odile.juarez  %s followers  %s likes  ", fs, ls);
        g_ttNewData = true;
    }
}

#endif
