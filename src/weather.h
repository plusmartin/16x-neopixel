#ifndef WEATHER_H
#define WEATHER_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "secrets.h"

static float    g_tempC      = NAN;
static uint32_t g_lastFetch  = 0;

inline void weatherUpdate()
{
  static const uint32_t INTERVAL = 10UL * 60UL * 1000UL; // 10 min
  if (!isnan(g_tempC) && millis() - g_lastFetch < INTERVAL) return;
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  String url = "http://api.openweathermap.org/data/2.5/weather?lat=";
  url += WEATHER_LAT;
  url += "&lon=";
  url += WEATHER_LON;
  url += "&appid=";
  url += WEATHER_API_KEY;
  url += "&units=metric";

  http.begin(url);
  http.setTimeout(8000);
  int code = http.GET();
  if (code == 200) {
    String body = http.getString();
    int mainIdx = body.indexOf("\"main\":");
    if (mainIdx >= 0) {
      int idx = body.indexOf("\"temp\":", mainIdx);
      if (idx >= 0) {
        g_tempC     = body.substring(idx + 7).toFloat();
        g_lastFetch = millis();
      }
    }
  }
  http.end();
}

inline float weatherTempC() { return g_tempC; }

#endif
