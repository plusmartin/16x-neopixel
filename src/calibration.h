#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <Preferences.h>
#include <FastLED.h>
#include "configs.h"

// Per-panel RGB scale factors stored in NVS.
// 255 = no correction. Values below 255 reduce that channel on that panel.
// Panel index = panel_y * PANELS_X + (PANELS_X - 1 - panel_x)
static uint8_t g_panelCal[16][3];  // [panel][0=r, 1=g, 2=b]

inline void calLoad()
{
    memset(g_panelCal, 255, sizeof(g_panelCal));
    Preferences p;
    if (p.begin("calib", true)) {
        p.getBytes("cal", g_panelCal, sizeof(g_panelCal));
        p.end();
    }
}

inline void calSave()
{
    Preferences p;
    p.begin("calib", false);
    p.putBytes("cal", g_panelCal, sizeof(g_panelCal));
    p.end();
}

inline void calSet(uint8_t panel, uint8_t r, uint8_t g, uint8_t b)
{
    if (panel >= 16) return;
    g_panelCal[panel][0] = r;
    g_panelCal[panel][1] = g;
    g_panelCal[panel][2] = b;
    calSave();
    Serial0.printf("[cal] panel %d → R=%d G=%d B=%d\n", panel, r, g, b);
}

inline void calApply(CRGB *leds)
{
    for (int panel = 0; panel < 16; panel++) {
        if (g_panelCal[panel][0] == 255 &&
            g_panelCal[panel][1] == 255 &&
            g_panelCal[panel][2] == 255) continue;
        int base = panel * (PANEL_W * PANEL_H);
        for (int i = base; i < base + PANEL_W * PANEL_H; i++) {
            leds[i].r = scale8(leds[i].r, g_panelCal[panel][0]);
            leds[i].g = scale8(leds[i].g, g_panelCal[panel][1]);
            leds[i].b = scale8(leds[i].b, g_panelCal[panel][2]);
        }
    }
}

#endif
