// 16x NeoPixel 4x4 matrix display — @plusmartin martin.garwil@gmail.com

#include <Arduino.h>
#include <WiFi.h>
#include <FastLED.h>
#include "configs.h"
#include "OTAClient.h"

CRGB leds[NUM_LEDS];
OTAClient OTA;

void setup()
{
  Serial.begin(115200);
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();
  Serial.println("Display ready.");
}

void loop()
{
}
