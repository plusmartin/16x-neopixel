#ifndef CONFIGS_H
#define CONFIGS_H

// ----- Hardware -----
#define DATA_PIN       5      // GPIO pin connected to first panel DIN

// ----- Panel layout -----
#define PANEL_W        8      // pixels per panel (width)
#define PANEL_H        8      // pixels per panel (height)
#define PANELS_X       4      // panels across
#define PANELS_Y       4      // panels down

// ----- Derived -----
#define MATRIX_W       (PANEL_W * PANELS_X)   // 32
#define MATRIX_H       (PANEL_H * PANELS_Y)   // 32
#define NUM_LEDS       (MATRIX_W * MATRIX_H)  // 1024

// ----- FastLED -----
#define LED_TYPE       WS2812B
#define COLOR_ORDER    GRB
#define BRIGHTNESS     40     // 0-255, keep low during dev to limit current draw

#endif
