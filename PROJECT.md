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
`v0.2` — FastLED initialized, 4-pin parallel output configured, xy() mapping written, crosshair test ready. Awaiting hardware assembly for validation.

## Last Session — 2026-05-22
- Decided on 4×4 panel layout (32×32, 1024 LEDs) over 5×3
- Replaced Adafruit NeoPixel with FastLED
- Created configs.h with all constants
- Wrote xy() coordinate mapping with testCrosshair() in display.h
- Upgraded to 4 parallel data pins (GPIO 5, 18, 19, 21) for ~100 FPS
- Pushed to private GitHub repo: https://github.com/plusmartin/16x-neopixel
- Next: flash hardware, validate xy() mapping, then Phase 3 (display primitives)

## Known Blockers
- Hardware not yet assembled — panels, wiring, and PSU needed before any firmware validation

## Links
- Repo: https://github.com/plusmartin/16x-neopixel
- Global framework: https://github.com/plusmartin/PROJECTS
