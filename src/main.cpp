// 16x NeoPixel 4x4 matrix display — @plusmartin martin.garwil@gmail.com

#include <Arduino.h>
#include <WiFi.h>
#include <FastLED.h>
#include "configs.h"
#include "display.h"
#include "OTAClient.h"

CRGB leds[NUM_LEDS];
OTAClient OTA;

void setup()
{
  Serial.begin(115200);
  delay(2000);
  FastLED.addLeds<LED_TYPE, DATA_PIN_0, COLOR_ORDER>(leds,                 0, LEDS_PER_PIN);
  FastLED.addLeds<LED_TYPE, DATA_PIN_1, COLOR_ORDER>(leds,   LEDS_PER_PIN, LEDS_PER_PIN);
  FastLED.addLeds<LED_TYPE, DATA_PIN_2, COLOR_ORDER>(leds, 2*LEDS_PER_PIN, LEDS_PER_PIN);
  FastLED.addLeds<LED_TYPE, DATA_PIN_3, COLOR_ORDER>(leds, 3*LEDS_PER_PIN, LEDS_PER_PIN);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();

  // Phase 2 test: crosshair to verify coordinate mapping
  // Expected: red horizontal line at y=16, green vertical at x=16, blue dot at (0,0)
  testCrosshair();
  Serial.println("Test dots drawn.");
}

void loop()
{
}
