# IRIS Display: Implementation Spec

## Scope

Drive a GC9A01 1.28" round TFT (240×240) on a dedicated Hosyond ESP32-S3 N16R8. Display the IRIS surveillance AI character — an animated eye lens filling the circular canvas. No housing chrome, no status bar, no ticker zone; the round format IS the eye.

---

## Hardware

- **Display:** 1.28" round TFT, GC9A01 controller, 240×240, SPI
- **MCU:** Hosyond ESP32-S3 N16R8 (16MB flash, 8MB PSRAM)
- **Power:** Car 12V permanent (same rail as Uno R4)
- **Library:** TFT_eSPI (Bodmer) — configured via `User_Setup.h` alongside the sketch

### Why TFT_eSPI

- Hardware SPI at 40–80MHz on ESP32-S3 — smooth animation
- GC9A01 driver built in
- Adafruit GFX-compatible drawing primitives
- PSRAM available for framebuffers as animation complexity grows

### Pin assignment (ESP32-S3 — TBD on hardware arrival)

All pins defined as named constants. Avoid strapping pins (0, 45, 46) and flash pins (26–32 on some variants — confirm datasheet).

| Signal | GPIO | Notes |
|---|---|---|
| MOSI (SDA) | TBD | Hardware SPI MOSI |
| SCK (CLK) | TBD | Hardware SPI clock |
| CS | TBD | Chip select |
| DC | TBD | Data/command |
| RST | TBD | Reset |
| VCC | 3.3V | GC9A01 is 3.3V logic |
| GND | GND | |

---

## TFT_eSPI configuration

`User_Setup.h` lives in the same sketch folder. Point TFT_eSPI to it by defining `USER_SETUP_LOADED` or placing the file at the library root — see TFT_eSPI docs for local setup method.

```cpp
#define USER_SETUP_LOADED
#define GC9A01_DRIVER
#define TFT_WIDTH  240
#define TFT_HEIGHT 240

// Update with actual GPIO numbers once hardware is in hand:
#define TFT_MOSI  /* GPIO TBD */
#define TFT_SCLK  /* GPIO TBD */
#define TFT_CS    /* GPIO TBD */
#define TFT_DC    /* GPIO TBD */
#define TFT_RST   /* GPIO TBD */

#define SPI_FREQUENCY     40000000
#define SPI_READ_FREQUENCY  20000000
```

---

## Serial input

Receives scene index from Interior Hub via wired UART (shared Serial2 broadcast TX).

- Baud: 9600
- Format: ASCII integer + newline (`"0\n"`, `"1\n"`, `"2\n"`)
- Same format as exterior node — both wired to the same TX pin on the hub

```cpp
const int PIN_IRIS_RX = /* TBD */;  // UART RX from hub Serial2 TX

// In setup():
Serial1.begin(9600, SERIAL_8N1, PIN_IRIS_RX, -1);  // RX only
```

---

## Phase roadmap

| Phase | Scope | Notes |
|---|---|---|
| 1 | Static test pattern — confirms wiring and SPI | Done |
| 2 | Static IRIS eye character — cream oval, almond slit, pupil dot, all mood draw functions stubbed | Current scope |
| 3 | Animated mood states — interpolate eyelid planes, pupil size/offset; full glitch scanlines | Non-blocking, millis() |
| 4 | Scene-driven color via UART — parses scene index from Interior Hub | Tints sclera per scene |
| 5 | Mood state machine — mood transitions driven by scene + autonomous timing | Driven by scene + timing |

---

## Phase 2: IRIS eye character (current scope)

See mockups: `assets/brand/iris/iris-mock.png`, `assets/brand/iris/iris-moods.png`

The eye fills the circular 240×240 canvas. All expression is in the eye — no housing chrome, no status bar.

### Rendering layers (draw order)

1. **Black background** — `fillScreen(COLOR_BLACK)`
2. **Cream oval sclera** — `fillEllipse(cx, cy, OVAL_RX, OVAL_RY, COLOR_CREAM)` — horizontal oval, warm off-white
3. **Almond slit** — `fillLens()` scanline function, two-circle intersection → pointed ends; cut into sclera in black
4. **Eyelid planes** — black `fillRect` from top and/or bottom of oval; clipping amount encodes mood
5. **Pupil dot** — `fillCircle` in white (green in glitch state)

### Eye geometry constants (tune on hardware)

| Symbol | Value | Description |
|---|---|---|
| `OVAL_RX` | 100 | Sclera semi-width (px) |
| `OVAL_RY` | 78 | Sclera semi-height (px) |
| `COLOR_CREAM` | `color565(238, 230, 196)` | Warm off-white — set post-init |

### Mood states

All six moods are stubbed as draw functions in Phase 2. Phase 3 animates between them.

| Mood | topLidY | botLidY | slitRx | slitRy | pR | pColor | Read |
|---|---|---|---|---|---|---|---|
| Neutral | −OVAL_RY | +OVAL_RY | 76 | 20 | 13 | white | Open, passive |
| Squint | −6 | +6 | 76 | 20 | 10 | white | Nearly closed, calculating |
| Surprised | −OVAL_RY | +OVAL_RY | 76 | 28 | 18 | white | Wider slit, larger dot |
| Suspicious | −10 | +32 | 76 | 20 | 11 | white | Top eyelid heavy, offset pupil |
| Wide scan | −OVAL_RY | +OVAL_RY | 88 | 24 | 15 | white | Wider slit, active sweep |
| Glitch | −OVAL_RY | +OVAL_RY | 76 | 20 | 16 | **green** | Full scanlines deferred to Phase 3 |

`topLidY` and `botLidY` are y offsets from `LCD_CY`. A top eyelid covers from `LCD_CY − OVAL_RY` down to `LCD_CY + topLidY`.

### Color modes (Phase 4)

`scleraColor` is passed into `drawIris()` — change it per scene index:

- **Scene 0 (OFF):** `COLOR_CREAM` — default, corporate/friendly
- **Scene 1 (RED):** tinted red sclera — surveillance/patrol
- **Scene 2 (GREEN):** tinted green sclera — sweep mode

Glitch state always overrides pupil to `COLOR_GLITCH_GREEN` regardless of scene.

---

## Verification

- Display initializes without hanging `setup()`
- Black background with cyan circle visible, "IRIS" label present
- Aperture ring outline at circle edge
- `Serial` prints "IRIS display init complete"
- If screen blank/white: check VCC (must be 3.3V), confirm CS/DC/RST wiring
- If garbled: try `tft.setRotation(1)`, check MOSI/SCK aren't swapped

---

## Open questions

- [x] Pin assignments — set in User_Setup.h (MOSI=11, SCLK=12, CS=10, DC=13, RST=14)
- [x] Hardware arrived — Hosyond ESP32-S3 N16R8 in hand
- [ ] Confirm GC9A01 module VCC tolerance (most modules accept 3.3V–5V input with onboard regulator, but logic must be 3.3V)
- [ ] Tune OVAL_RX, OVAL_RY, SLIT_RX, SLIT_RY on hardware to match mockup proportions
- [ ] Glitch scanline distortion implementation (Phase 3 workstream)
- [ ] Glitch state timing and trigger logic (Phase 5 workstream)
