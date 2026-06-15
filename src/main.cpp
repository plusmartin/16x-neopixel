// 16x NeoPixel 4x4 matrix display — @plusmartin martin.garwil@gmail.com
// External services: NTP pool.ntp.org · OpenWeatherMap api.openweathermap.org
//   Cloudflare Worker /gif + /tiktok (32x32display.martin-garwil.workers.dev)
//   OTA: github.com/plusmartin/16x-neopixel/releases (firmware.bin)

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
#include "clocks.h"
#include "animations2.h"
#include "gif.h"
#include "api.h"
#include "tiktok.h"

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

// ── Night mode brightness ─────────────────────────────────────────────────────
static void updateBrightness()
{
  int h = timeClient.getHours();
  bool night = (h >= 22 || h < 6);
  FastLED.setBrightness(night ? 3 : BRIGHTNESS);
}

// ── M_AUTO loop ───────────────────────────────────────────────────────────────
static const uint32_t PHASE_MS = 20000;

static void loopAuto()
{
  static uint32_t phaseStart = 0;
  static bool     textPhase  = true;
  static uint8_t  animIdx    = 0;
  static uint8_t  animHue    = 0;
  static uint8_t  clockIdx   = 0;

  uint32_t now = millis();

  if (textPhase && now - phaseStart >= PHASE_MS) {
    g_ttNewData = false;           // done showing TikTok phase (if it was one)
    textPhase  = false;
    phaseStart = now;
    static const char* ANIM_NAMES[] = {
      "Fire","MatrixRain","Starfield","MeteorShower","Plasma",
      "Rainbow","Wipe","Sparkle","Life","Breathe","GIF",
      "Balls","Ripple","Fireworks","Twinkle","Kaleidoscope",
      "WaveInterference","Tunnel","DNA","Pendulum","Cube",
      "Lissajous","Glitch","Aurora","Spiral","Pulse",
      "ColorNoise","Lightning","Snow","CheckerWave","Smoke",
      "Langton","PacDots"
    };
    int gc = gifCount();
    animIdx = random8(gc > 0 ? 33 : 32);
    Serial0.printf("[anim] %d: %s\n", animIdx, ANIM_NAMES[animIdx]);
    animHue    = random8();
    weatherUpdate();
  } else if (!textPhase && now - phaseStart >= PHASE_MS) {
    gifStop();
    textPhase  = true;
    phaseStart = now;
    if (!g_ttNewData) clockIdx = (clockIdx + 1) % 7;  // don't advance clock for TikTok phase
    timeClient.update();
  }

  int ch = timeClient.getHours();
  int cm = timeClient.getMinutes();
  int cs = timeClient.getSeconds();

  if (textPhase) {
    if (g_ttNewData) {
      // TikTok update — scroll in TikTok red until phase ends
      scrollText(g_ttMsg, CRGB(0xFF, 0x00, 0x50), (MATRIX_H - 7) / 2, 35);
    } else {
      switch (clockIdx) {
        case 0: drawClockStatic();                    break;
        case 1: animClockAnalog(ch, cm, cs);          break;
        case 2: animClockArcs(ch, cm, cs);            break;
        case 3: animClockBinary(ch, cm);              break;
        case 4: animClockBars(ch, cm);                break;
        case 5: animClockPong(ch, cm);                break;
        case 6: animClockColor(ch, cm);               break;
      }
    }
  } else {
    switch (animIdx) {
      case 0: animFire();                           break;
      case 1: animMatrixRain();                     break;
      case 2: animStarfield();                      break;
      case 3: animMeteorShower();                   break;
      case 4: animPlasma();                         break;
      case 5: animRainbow();                        break;
      case 6: animWipe();                           break;
      case 7: animSparkle();                        break;
      case 8: animLife();                           break;
      case  9: animBreathe(CHSV(animHue, 220, 255)); break;
      case 10: animGif();              break;
      case 11: animBalls();            break;
      case 12: animRipple();           break;
      case 13: animFireworks();        break;
      case 14: animTwinkle();          break;
      case 15: animKaleidoscope();     break;
      case 16: animWaveInterference(); break;
      case 17: animTunnel();           break;
      case 18: animDNA();              break;
      case 19: animPendulum();         break;
      case 20: animCube();             break;
      case 21: animLissajous();        break;
      case 22: animGlitch();           break;
      case 23: animAurora();           break;
      case 24: animSpiral();           break;
      case 25: animPulse();            break;
      case 26: animColorNoise();       break;
      case 27: animLightning();        break;
      case 28: animSnow();             break;
      case 29: animCheckerWave();      break;
      case 30: animSmoke();            break;
      case 31: animLangton();          break;
      case 32: animPacDots();          break;
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
  calLoad();

  wifiConnect();
  Serial0.println("WiFi done");

  random16_set_seed(esp_random()); // hardware RNG → animations vary each boot

  gifInit();

  if (WiFi.status() == WL_CONNECTED) {
    timeClient.begin();
    timeClient.update();
    weatherUpdate();
    tiktokCheck();
    for (int i = 0; i < GIF_BOOT_FETCH; i++) gifFetch();
  }

  apiBegin();
}

// ── loop ──────────────────────────────────────────────────────────────────────
void loop()
{
  static uint32_t lastBriCheck = 0;
  if (millis() - lastBriCheck > 30000) { lastBriCheck = millis(); updateBrightness(); }

  static uint32_t lastIPPrint = 0;
  if (millis() - lastIPPrint > 10000) {
    lastIPPrint = millis();
    String ip = (WiFi.status() == WL_CONNECTED)
                ? "STA " + WiFi.localIP().toString()
                : "AP  " + WiFi.softAPIP().toString();
    Serial0.println("[v" FW_VERSION "] " + ip);
  }

  static uint32_t lastTikTok        = 0;
  static uint32_t lastTikTokDisplay = 0;
  if (millis() - lastTikTok > TIKTOK_CHECK_INTERVAL) { lastTikTok = millis(); tiktokCheck(); }
  if (millis() - lastTikTokDisplay > 3600000UL)      { lastTikTokDisplay = millis(); tiktokForceDisplay(); }

  static uint32_t lastGifFetch = 0;
  static bool     hasGifs      = false;
  uint32_t gifInterval = hasGifs ? GIF_FETCH_INTERVAL : 120000UL;
  if (millis() - lastGifFetch > gifInterval) {
    lastGifFetch = millis();
    if (gifFetch()) hasGifs = true;
    else if (!hasGifs) hasGifs = (gifCount() > 0);
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
    case M_CAL_WHITE:
      fill(CRGB(80, 80, 80)); show(); delay(100);
      break;
    case M_SCROLL:
      scrollText(scrollMsg,
                 CRGB((scrollColorHex>>16)&0xFF,
                      (scrollColorHex>>8) &0xFF,
                       scrollColorHex     &0xFF),
                 (MATRIX_H - 7) / 2, scrollSpeed);
      break;
  }
}
