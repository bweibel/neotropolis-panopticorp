# IRIS Display: Implementation Spec

## Scope

Get the Waveshare 2" LCD (ST7789V, 240×320, SPI) up and running on the Arduino Uno R4 WiFi with a static test pattern. No animation, no serial integration, no mood states yet. Goal is confirmed display output before any further work.

---

## Phase 1: Test pattern (current scope)

A static image drawn once in `setup()`. Confirms wiring, SPI communication, and library are all working.

**Test pattern contents:**
- Black background
- Filled circle centered on screen (stand-in for the IRIS lens)
- A short text label ("IRIS" or "PANOPTICORP") in the primary color
- A colored bar across the bottom (stand-in for the ticker zone)

This is throwaway — just a smoke test. It will be replaced in Phase 2.

---

## Hardware

- **Display:** Waveshare 2inch LCD Module, ST7789V controller, 240×320, SPI
- **MCU:** Arduino Uno R4 WiFi (prototype)

### Pin conflict note

The Uno R4 hardware SPI pins (SCK=13, MOSI=11) are occupied by the X9C104 pot (`PIN_POT_CS=13`, `PIN_POT_INC=11`). Use **software SPI** on free pins for the display.

| Display pin | Uno R4 pin | Notes |
|---|---|---|
| SCK | A1 | Software SPI clock |
| MOSI (SDA) | A2 | Software SPI data |
| CS | A3 | Chip select |
| DC | A4 | Data/command |
| RST | A5 | Reset |
| VCC | 3.3V | ST7789V is 3.3V logic |
| GND | GND | |

All pins defined as named constants. No magic numbers.

---

## Library

Use **Adafruit ST7735 and ST7789 Library** + **Adafruit GFX Library**. Both available via Arduino Library Manager. Supports software SPI via constructor arguments.

```cpp
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
```

Constructor for software SPI:
```cpp
Adafruit_ST7789 tft = Adafruit_ST7789(PIN_LCD_CS, PIN_LCD_DC, PIN_LCD_MOSI, PIN_LCD_SCK, PIN_LCD_RST);
```

---

## Implementation

### New file: `display-code/IrisDisplay/IrisDisplay.ino`

Standalone sketch — not merged into `PioneerController.ino` at this stage. Develop and test independently, integrate later.

```cpp
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// Pin constants
const int PIN_LCD_SCK  = A1;
const int PIN_LCD_MOSI = A2;
const int PIN_LCD_CS   = A3;
const int PIN_LCD_DC   = A4;
const int PIN_LCD_RST  = A5;

// Display dimensions
const int LCD_W = 240;
const int LCD_H = 320;

Adafruit_ST7789 tft = Adafruit_ST7789(PIN_LCD_CS, PIN_LCD_DC, PIN_LCD_MOSI, PIN_LCD_SCK, PIN_LCD_RST);

void setup() {
  Serial.begin(115200);
  tft.init(LCD_W, LCD_H);
  tft.setRotation(0);  // adjust if display orientation is wrong
  drawTestPattern();
  Serial.println("Display init complete");
}

void loop() {
  // nothing — static display
}

void drawTestPattern() {
  tft.fillScreen(ST77XX_BLACK);

  // Lens stand-in: cyan circle centered
  tft.fillCircle(LCD_W / 2, LCD_H / 2, 80, 0x07FF);  // cyan

  // Label
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.setCursor(80, 20);
  tft.print("IRIS");

  // Ticker zone bar
  tft.fillRect(0, LCD_H - 40, LCD_W, 40, 0x07FF);  // cyan bar
}
```

---

## Next phases (not current scope)

- **Phase 2:** Static IRIS housing illustration — the approved visual design rendered as vector-style primitives
- **Phase 3:** Ticker scrolling text
- **Phase 4:** Mood state transitions driven by serial events from Uno
- **Phase 5:** Integration into `PioneerController.ino` — display updates on input events

---

## Verification

- Display initializes without hanging `setup()`
- Test pattern visible: black background, cyan circle, "IRIS" label, bottom bar
- `Serial` prints "Display init complete"
- If screen is blank/white: check VCC (must be 3.3V), confirm CS/DC/RST wiring
- If garbled: try `tft.setRotation(1)` or check SCK/MOSI aren't swapped
