// 16x NeoPixel 4x4 matrix display — @plusmartin martin.garwil@gmail.com

#include <Arduino.h>
#include <WiFi.h>
#include <FastLED.h>
#include "configs.h"
#include "display.h"
#include "animations.h"
#include "text.h"
#include "life.h"
#include "secrets.h"
#include "api.h"

// ── global state (extern'd by api.h) ─────────────────────────────────────────
CRGB             leds[NUM_LEDS];
CRGB             backbuf[NUM_LEDS];
volatile Mode    currentMode  = M_PLASMA;
char             scrollMsg[128] = "Hello!";
uint32_t         scrollColorHex = 0xFFFFFF;
uint16_t         scrollSpeed    = 30;

// ── WiFi ──────────────────────────────────────────────────────────────────────
static bool trySSID(const char* ssid, const char* pass)
{
  Serial0.printf("\nConnecting to %s", ssid);
  WiFi.begin(ssid, pass);
  uint32_t t = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t < 10000) {
    delay(250);
    Serial0.print('.');
  }
  return WiFi.status() == WL_CONNECTED;
}

static void wifiConnect()
{
  WiFi.mode(WIFI_STA);

  if (trySSID(WIFI_SSID_1, WIFI_PASS_1) ||
      (WiFi.disconnect(true), delay(500), trySSID(WIFI_SSID_2, WIFI_PASS_2))) {
    Serial0.println("\nIP: " + WiFi.localIP().toString());
  } else {
    Serial0.println("\nBoth SSIDs failed — starting AP: " AP_SSID);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    Serial0.println("AP IP: " + WiFi.softAPIP().toString());
  }
}

// ── setup ─────────────────────────────────────────────────────────────────────
void setup()
{
  Serial0.begin(115200);  // UART0 — same port as bootloader output
  delay(500);
  Serial0.println("\n=== BOOT ===");

  Serial0.println("FastLED init...");
  FastLED.addLeds<LED_TYPE, DATA_PIN_0, COLOR_ORDER>(leds,                 0, LEDS_PER_PIN);
  FastLED.addLeds<LED_TYPE, DATA_PIN_1, COLOR_ORDER>(leds,   LEDS_PER_PIN, LEDS_PER_PIN);
  FastLED.addLeds<LED_TYPE, DATA_PIN_2, COLOR_ORDER>(leds, 2*LEDS_PER_PIN, LEDS_PER_PIN);
  FastLED.addLeds<LED_TYPE, DATA_PIN_3, COLOR_ORDER>(leds, 3*LEDS_PER_PIN, LEDS_PER_PIN);
  FastLED.setBrightness(BRIGHTNESS);
  clear(); show();
  Serial0.println("FastLED OK");

  wifiConnect();
  Serial0.println("WiFi done");
  apiBegin();
}

// ── loop ──────────────────────────────────────────────────────────────────────
void loop()
{
  static uint32_t lastIPPrint = 0;
  if (millis() - lastIPPrint > 2000) {
    lastIPPrint = millis();
    if (WiFi.status() == WL_CONNECTED)
      Serial0.println("STA IP: " + WiFi.localIP().toString());
    else
      Serial0.println("AP  IP: " + WiFi.softAPIP().toString());
  }

  apiLoop();

  switch (currentMode) {
    case M_PLASMA:  animPlasma();                                        break;
    case M_RAINBOW: animRainbow();                                       break;
    case M_LIFE:    animLife();                                          break;
    case M_WIPE:    animWipe();                                          break;
    case M_SPARKLE: animSparkle();                                       break;
    case M_BREATHE: animBreathe(CRGB::Cyan);                            break;
    case M_SCROLL:
      scrollText(scrollMsg,
                 CRGB((scrollColorHex>>16)&0xFF,
                      (scrollColorHex>>8)&0xFF,
                       scrollColorHex&0xFF),
                 (MATRIX_H - 7) / 2,
                 scrollSpeed);
      break;
  }
}
