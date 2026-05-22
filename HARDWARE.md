# Hardware: 16x NeoPixel Display

## Components
| Component | Model | Qty | Status |
|-----------|-------|-----|--------|
| MCU | ESP32 Dev Board | 1 | Available |
| LED panels | WS2812B 8×8 (64 LEDs each) | 16 | Available |
| PSU | 5V / 10A minimum (for brightness 40) | 1 | TBD |
| Decoupling caps | 1000µF electrolytic | 4 | TBD |
| Resistor | 300–500Ω on each data line | 4 | TBD |

## Wiring / Pinout

### Data lines (4 parallel chains)
| GPIO | Panel row | Panels |
|------|-----------|--------|
| 5 | Top | 0–3 |
| 18 | 2nd | 4–7 |
| 19 | 3rd | 8–11 |
| 21 | Bottom | 12–15 |

Within each row: DOUT of panel N → DIN of panel N+1 (left to right).

### Power injection
Inject 5V + GND at the start of every 256-LED chain (one per row).
Add 1000µF cap across 5V/GND at each injection point.

### Current draw reference
| Brightness | Estimated current |
|------------|------------------|
| 40 (dev) | ~9.6A |
| 128 | ~30A |
| 255 (max) | ~61A |

## Firmware Flash Procedure
1. Install PlatformIO
2. Clone https://github.com/plusmartin/16x-neopixel
3. Connect ESP32 via USB
4. `pio run --target upload`
5. Open serial monitor at 115200 baud

## Test Protocol

### Phase 2 — xy() mapping validation
Flash current firmware. On boot, crosshair draws automatically.

**Pass criteria:**
- [ ] Solid red horizontal line at y=16 (pixels 0–31 on row 16)
- [ ] Solid green vertical line at x=16 (pixels 0–31 on column 16)
- [ ] Single blue pixel at (0,0) = physical top-left corner of display

**If wrong:**
- Broken/skipped panels → panel chain order wrong, adjust `panel_y * PANELS_X + panel_x` in `xy()`
- Zigzag within a panel → flip serpentine direction (`ly & 1` logic in `xy()`)
- Entire image mirrored → flip `lx` or `ly`

### Known hardware issues
_None yet — hardware not assembled._
