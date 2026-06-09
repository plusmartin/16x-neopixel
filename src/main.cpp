// 16x NeoPixel 4x4 matrix display — @plusmartin martin.garwil@gmail.com

#include <Arduino.h>
#include <WiFi.h>
#include <FastLED.h>
#include <WiFiUDP.h>
#include <NTPClient.h>
#include "configs.h"
#include "display.h"
#include "animations.h"
#include "text.h"
#include "life.h"
#include "secrets.h"
#include "weather.h"
#include "api.h"

// ── global state (extern'd by api.h) ─────────────────────────────────────────
CRGB             leds[NUM_LEDS];
CRGB             backbuf[NUM_LEDS];
volatile Mode    currentMode  = M_AUTO;
char             scrollMsg[128] = "Hello!";
uint32_t         scrollColorHex = 0xFFFFFF;
uint16_t         scrollSpeed    = 30;

// ── NTP ───────────────────────────────────────────────────────────────────────
static WiFiUDP    ntpUDP;
static NTPClient  timeClient(ntpUDP, "pool.ntp.org", TIMEZONE_OFFSET, 60000);

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

// ── clock helpers ─────────────────────────────────────────────────────────────
static const char* dayName(int d)
{
  static const char* days[] = {"DOM","LUN","MAR","MIE","JUE","VIE","SAB"};
  return days[d % 7];
}

static const char* greeting(int h)
{
  if (h >= 5  && h < 12) return "BUENOS DIAS";
  if (h >= 12 && h < 20) return "BUENAS TARDES";
  return "BUENAS NOCHES";
}

// Static 3-row clock: time / temperature / day
// Layout (5x7 font, 2px gap): rows at y=4, y=13, y=22
static void drawClockStatic()
{
  static uint32_t last = 0;
  if (millis() - last < 500) return;
  last = millis();

  clear();
  char buf[12];

  snprintf(buf, sizeof(buf), "%02d:%02d", timeClient.getHours(), timeClient.getMinutes());
  drawTextCentered(buf, 4, CRGB(0, 200, 255));

  float temp = weatherTempC();
  if (!isnan(temp)) {
    snprintf(buf, sizeof(buf), "%.0fC", temp);
    drawTextCentered(buf, 13, CRGB(255, 140, 0));
  }

  drawTextCentered(dayName(timeClient.getDay()), 22, CRGB(180, 180, 180));
  show();
}

// ── M_AUTO loop ───────────────────────────────────────────────────────────────
static const uint32_t PHASE_MS = 10000;

static void loopAuto()
{
  static uint32_t phaseStart = 0;
  static bool     textPhase  = true;
  static uint8_t  animIdx    = 0;
  static uint8_t  animHue    = 0;

  uint32_t now = millis();

  if (textPhase && now - phaseStart >= PHASE_MS) {
    textPhase  = false;
    phaseStart = now;
    animIdx    = random8(10);
    animHue    = random8();
    weatherUpdate();
  } else if (!textPhase && now - phaseStart >= PHASE_MS) {
    textPhase  = true;
    phaseStart = now;
    timeClient.update();
  }

  if (textPhase) {
    drawClockStatic();
  } else {
    switch (animIdx) {
      case 0: animFire();                  break;
      case 1: animMatrixRain();            break;
      case 2: animStarfield();             break;
      case 3: animMeteorShower();          break;
      case 4: animPlasma();                break;
      case 5: animRainbow();               break;
      case 6: animWipe();                  break;
      case 7: animSparkle();               break;
      case 8: animLife();                  break;
      case 9: animBreathe(CHSV(animHue, 220, 255));   break;
    }
  }
}

// ── setup ─────────────────────────────────────────────────────────────────────
void setup()
{
  Serial0.begin(115200);
  delay(500);
  Serial0.println("\n=== BOOT ===");

  Serial0.println("FastLED init...");
  FastLED.addLeds<LED_TYPE, DATA_PIN_0, COLOR_ORDER>(leds,               0, LEDS_PER_PIN);
  FastLED.addLeds<LED_TYPE, DATA_PIN_1, COLOR_ORDER>(leds,   LEDS_PER_PIN, LEDS_PER_PIN);
  FastLED.addLeds<LED_TYPE, DATA_PIN_2, COLOR_ORDER>(leds, 2*LEDS_PER_PIN, LEDS_PER_PIN);
  FastLED.addLeds<LED_TYPE, DATA_PIN_3, COLOR_ORDER>(leds, 3*LEDS_PER_PIN, LEDS_PER_PIN);
  FastLED.setBrightness(BRIGHTNESS);
  clear(); show();
  Serial0.println("FastLED OK");

  wifiConnect();
  Serial0.println("WiFi done");

  random16_set_seed(esp_random()); // hardware RNG → animations vary each boot

  if (WiFi.status() == WL_CONNECTED) {
    timeClient.begin();
    timeClient.update();
    weatherUpdate();
  }

  apiBegin();
}

// ── loop ──────────────────────────────────────────────────────────────────────
void loop()
{
  static uint32_t lastIPPrint = 0;
  if (millis() - lastIPPrint > 10000) {
    lastIPPrint = millis();
    String ip = (WiFi.status() == WL_CONNECTED)
                ? "STA " + WiFi.localIP().toString()
                : "AP  " + WiFi.softAPIP().toString();
    Serial0.println("[v" FW_VERSION "] " + ip);
  }

  apiLoop();

  switch (currentMode) {
    case M_AUTO:        loopAuto();                                        break;
    case M_CLOCK:
      timeClient.update();
      drawClockStatic();
      break;
    case M_PLASMA:      animPlasma();                                      break;
    case M_RAINBOW:     animRainbow();                                     break;
    case M_LIFE:        animLife();                                        break;
    case M_WIPE:        animWipe();                                        break;
    case M_SPARKLE:     animSparkle();                                     break;
    case M_BREATHE:     animBreathe(CRGB::Cyan);                          break;
    case M_FIRE:        animFire();                                        break;
    case M_MATRIX_RAIN: animMatrixRain();                                  break;
    case M_STARFIELD:   animStarfield();                                   break;
    case M_METEOR:      animMeteorShower();                                break;
    case M_SCROLL:
      scrollText(scrollMsg,
                 CRGB((scrollColorHex>>16)&0xFF,
                      (scrollColorHex>>8) &0xFF,
                       scrollColorHex     &0xFF),
                 (MATRIX_H - 7) / 2, scrollSpeed);
      break;
  }
}
