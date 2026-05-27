# Project: 16x NeoPixel Display

## Overview
A 32×32 pixel RGB LED display built from 16 WS2812B 8×8 panels arranged in a 4×4 grid (1024 LEDs total), controlled by an ESP32. Designed for animations, interactive content, and remote control via WiFi. First use is personal/decorative; path to monetization is as a programmable display module.

## Goals
- Primary: Full-featured 32×32 LED display with animation, text, and remote control
- Monetization model: Sell as a plug-and-play display unit with web UI; target art installations, retail displays, maker community

## Tech Stack
- Hardware: ESP32 dev board, 16× WS2812B 8×8 panels, 5V PSU
- Firmware: PlatformIO / Arduino framework, FastLED library
- Infrastructure: OTA updates via AWS S3 (OTAClient), WiFi AP/STA

## Current Version
`v1.0-rc` — WiFi+API+WebUI complete. OTA blocked: GitHub repo is private, ESP32 needs auth token to download release asset.

## Last Session — 2026-05-26
- v0.1–v0.3 fully completed: hardware validated, display primitives, all animations, text scroll, Game of Life
- v1.0: WiFi STA+AP fallback, HTTP API (/api/mode, /api/text, /api/brightness, /api/ota), Web UI done
- OTA: switched from WiFiClientSecure to esp_https_ota + arduino_esp_crt_bundle_attach
- **BLOCKED**: GitHub repo is private → firmware.bin 404 without auth. Next session: add GitHub PAT header to OTA request.
- Next: add GITHUB_TOKEN to secrets.h, set Authorization header in OTAClient.cpp via esp_https_ota http_client_init_cb

## Known Blockers
- OTA: private GitHub repo requires `Authorization: token <PAT>` header. Fix: use `esp_https_ota_config_t.http_client_init_cb` to inject the header, store token in secrets.h

## Links
- Repo: https://github.com/plusmartin/16x-neopixel
- Global framework: https://github.com/plusmartin/PROJECTS
