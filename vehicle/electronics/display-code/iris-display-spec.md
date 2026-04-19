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
| 1 | Static test pattern — confirms wiring and SPI | Current scope |
| 2 | IRIS lens illustration — aperture blades, iris ring, glass arc | Static, drawn once |
| 3 | Animated lens — eyelid, pupil position, aperture spread | Non-blocking, millis() |
| 4 | Scene-driven color (cyan / red / green) via UART | Parses scene index |
| 5 | Mood state transitions — squint, surprised, wide scan, glitch | Driven by scene + timing |

---

## Phase 1: Test pattern (current scope)

Static image drawn once in `setup()`. Confirms wiring, SPI, and library before any animation work.

```cpp
#include <TFT_eSPI.h>

// Display dimensions
const int LCD_W = 240;
const int LCD_H = 240;
const int LCD_CX = LCD_W / 2;
const int LCD_CY = LCD_H / 2;

// IRIS palette
const uint16_t COLOR_IRIS_CYAN  = 0x07FF;  // cyan
const uint16_t COLOR_BLACK      = 0x0000;
const uint16_t COLOR_WHITE      = 0xFFFF;

TFT_eSPI tft = TFT_eSPI();

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(COLOR_BLACK);
  drawTestPattern();
  Serial.println("IRIS display init complete");
}

void loop() {
  // nothing — static display
}

void drawTestPattern() {
  // Lens fill — cyan circle nearly filling the round display
  tft.fillCircle(LCD_CX, LCD_CY, 110, COLOR_IRIS_CYAN);

  // Aperture ring — dark outline at edge
  tft.drawCircle(LCD_CX, LCD_CY, 110, COLOR_BLACK);
  tft.drawCircle(LCD_CX, LCD_CY, 109, COLOR_BLACK);

  // Pupil — black center
  tft.fillCircle(LCD_CX, LCD_CY, 30, COLOR_BLACK);

  // Label
  tft.setTextColor(COLOR_WHITE, COLOR_IRIS_CYAN);
  tft.setTextSize(2);
  tft.setCursor(LCD_CX - 20, LCD_CY + 50);
  tft.print("IRIS");
}
```

---

## Visual identity (for Phase 2+)

The IRIS lens fills the circular canvas. All expression is carried by the lens — there is no face.

Expression variables:
1. **Eyelid position** — dark plane sliding in from top or bottom
2. **Pupil size and position** — centered/shifted, large/small
3. **Iris ring brightness** — normal vs. heightened
4. **Aperture blade spread** — closed/standard vs. retracted/open

Color modes (driven by scene index):
- **Cyan** (scene 0 / OFF): default, corporate/friendly
- **Red** (scene 1 / RED): surveillance/patrol mode
- **Green** (scene 2 / GREEN): sweep mode

Mood states (Phase 5):
| Mood | Read |
|---|---|
| Neutral | Baseline, passive |
| Squint | Calculating |
| Surprised | Input detected, iris snaps open |
| Suspicious | Asymmetric, watching |
| Wide scan | Active surveillance sweep |
| Bleed/glitch | The other thing looking out |

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

- [ ] Pin assignments — all TBD pending Hosyond ESP32-S3 hardware arrival
- [ ] Confirm GC9A01 module VCC tolerance (most modules accept 3.3V–5V input with onboard regulator, but logic must be 3.3V)
- [ ] Glitch state timing and trigger logic (Phase 5 workstream)
- [ ] Full ticker token vocabulary if ticker is ever added back (currently cut for round format)
