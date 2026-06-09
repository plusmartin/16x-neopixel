#ifndef API_H
#define API_H

#include <WebServer.h>
#include "display.h"
#include "OTAClient.h"
#include "configs.h"

// ── shared state (defined in main.cpp) ───────────────────────────────────────
enum Mode { M_PLASMA, M_RAINBOW, M_LIFE, M_SCROLL, M_WIPE, M_SPARKLE, M_BREATHE,
            M_FIRE, M_MATRIX_RAIN, M_STARFIELD, M_METEOR,
            M_CLOCK, M_AUTO };
extern volatile Mode currentMode;
extern char          scrollMsg[128];
extern uint32_t      scrollColorHex;
extern uint16_t      scrollSpeed;

static WebServer server(80);

// ── HTML UI (served from PROGMEM) ────────────────────────────────────────────
static const char HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>NeoPixel Display</title>
<style>
  body{font-family:sans-serif;max-width:480px;margin:2rem auto;padding:0 1rem;background:#111;color:#eee}
  h1{color:#f80;margin-bottom:1.5rem}
  h2{font-size:.9rem;text-transform:uppercase;color:#888;margin:1.5rem 0 .5rem}
  .grid{display:grid;grid-template-columns:repeat(3,1fr);gap:.5rem}
  button{padding:.6rem;border:none;border-radius:6px;background:#333;color:#eee;cursor:pointer;font-size:.85rem}
  button:hover{background:#555}
  button.active{background:#f80;color:#000}
  input[type=text]{width:100%;box-sizing:border-box;padding:.5rem;background:#222;border:1px solid #444;color:#eee;border-radius:6px}
  input[type=color]{width:3rem;height:2rem;border:none;background:none;cursor:pointer}
  input[type=range]{width:100%}
  .row{display:flex;gap:.5rem;align-items:center}
  .send{flex:1}
  label{font-size:.85rem;color:#aaa}
</style>
</head><body>
<h1>NeoPixel Display</h1>

<h2>Animation</h2>
<div class="grid">
  <button onclick="mode('auto')" style="background:#f80;color:#000">Auto</button>
  <button onclick="mode('clock')">Clock</button>
  <button onclick="mode('plasma')">Plasma</button>
  <button onclick="mode('rainbow')">Rainbow</button>
  <button onclick="mode('fire')">Fire</button>
  <button onclick="mode('matrix')">Matrix Rain</button>
  <button onclick="mode('starfield')">Starfield</button>
  <button onclick="mode('meteor')">Meteors</button>
  <button onclick="mode('life')">Game of Life</button>
  <button onclick="mode('wipe')">Wipe</button>
  <button onclick="mode('sparkle')">Sparkle</button>
  <button onclick="mode('breathe')">Breathe</button>
</div>

<h2>Text Scroll</h2>
<div class="row">
  <input type="text" id="msg" value="Hello!" maxlength="120">
  <input type="color" id="clr" value="#ffffff">
</div>
<div class="row" style="margin-top:.5rem">
  <label>Speed&nbsp;<span id="spd_val">30</span>ms/px</label>
  <input type="range" id="spd" min="10" max="120" value="30"
         oninput="document.getElementById('spd_val').textContent=this.value">
</div>
<button class="send" style="margin-top:.5rem;width:100%" onclick="sendText()">Scroll Text</button>

<h2>Brightness</h2>
<div class="row">
  <input type="range" id="bri" min="1" max="255" value="40"
         oninput="setBrightness(this.value)">
  <label id="bri_val">40</label>
</div>

<h2>Firmware</h2>
<button onclick="ota()" style="background:#600;width:100%">Trigger OTA Update</button>

<script>
function get(url){fetch(url).catch(()=>{})}
function mode(m){get('/api/mode?name='+m)}
function sendText(){
  const msg=encodeURIComponent(document.getElementById('msg').value);
  const clr=document.getElementById('clr').value.replace('#','');
  const spd=document.getElementById('spd').value;
  get('/api/text?msg='+msg+'&color='+clr+'&speed='+spd);
}
function setBrightness(v){
  document.getElementById('bri_val').textContent=v;
  get('/api/brightness?val='+v);
}
function ota(){get('/api/ota')}
</script>
</body></html>
)rawliteral";

// ── helpers ───────────────────────────────────────────────────────────────────
static CRGB hexToRGB(uint32_t hex)
{
  return CRGB((hex >> 16) & 0xFF, (hex >> 8) & 0xFF, hex & 0xFF);
}

// ── route handlers ────────────────────────────────────────────────────────────
static void handleRoot()
{
  server.send_P(200, "text/html", HTML);
}

static void handleMode()
{
  String name = server.arg("name");
  if      (name == "auto")     currentMode = M_AUTO;
  else if (name == "clock")    currentMode = M_CLOCK;
  else if (name == "plasma")   currentMode = M_PLASMA;
  else if (name == "rainbow")  currentMode = M_RAINBOW;
  else if (name == "life")     currentMode = M_LIFE;
  else if (name == "wipe")     currentMode = M_WIPE;
  else if (name == "sparkle")  currentMode = M_SPARKLE;
  else if (name == "breathe")  currentMode = M_BREATHE;
  else if (name == "fire")      currentMode = M_FIRE;
  else if (name == "matrix")    currentMode = M_MATRIX_RAIN;
  else if (name == "starfield") currentMode = M_STARFIELD;
  else if (name == "meteor")    currentMode = M_METEOR;
  else { server.send(400, "text/plain", "unknown mode"); return; }
  server.send(200, "text/plain", "ok");
}

static void handleText()
{
  if (server.hasArg("msg"))   strncpy(scrollMsg, server.arg("msg").c_str(), 127);
  if (server.hasArg("color")) scrollColorHex = strtoul(server.arg("color").c_str(), nullptr, 16);
  if (server.hasArg("speed")) scrollSpeed    = server.arg("speed").toInt();
  currentMode = M_SCROLL;
  server.send(200, "text/plain", "ok");
}

static void handleBrightness()
{
  if (server.hasArg("val")) FastLED.setBrightness(server.arg("val").toInt());
  server.send(200, "text/plain", "ok");
}

static void handleOTA()
{
  Serial0.println("[OTA] handler called");
  server.send(200, "text/plain", "OTA triggered — rebooting when done");
  otaUpdate(OTA_URL);
}

// ── public interface ──────────────────────────────────────────────────────────
inline void apiBegin()
{
  server.on("/",                handleRoot);
  server.on("/api/mode",        handleMode);
  server.on("/api/text",        handleText);
  server.on("/api/brightness",  handleBrightness);
  server.on("/api/ota",         handleOTA);
  server.begin();
  Serial0.println("HTTP server started");
}

inline void apiLoop() { server.handleClient(); }

#endif
