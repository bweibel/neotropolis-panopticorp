# Character LCD: Implementation Spec

## Purpose

Add a WH1602B-TMI-JT 16×2 character LCD to the Arduino Uno R4 WiFi (PioneerController). Displays context-dependent status: boot messages, scene name, volume level, and Panopticorp flavor text. All display events are driven by existing Uno R4 input handlers — no new serial links needed.

---

## Hardware

- **Display:** Winstar WH1602B-TMI-JT, 16×2 character LCD, HD44780 controller
- **Interface:** Parallel 4-bit mode
- **Library:** Arduino built-in `LiquidCrystal`
- **MCU:** Arduino Uno R4 WiFi (permanent, car 12V)

---

## Pin assignment

| LCD Pin | Uno R4 Pin | Constant |
|---|---|---|
| RS (Register Select) | 10 | `PIN_LCD_RS` |
| EN (Enable) | A0 | `PIN_LCD_EN` |
| D4 | A1 | `PIN_LCD_D4` |
| D5 | A2 | `PIN_LCD_D5` |
| D6 | A3 | `PIN_LCD_D6` |
| D7 | A4 | `PIN_LCD_D7` |
| VCC | 5V | |
| GND | GND | |
| V0 (contrast) | Potentiometer wiper to GND | 10kΩ pot recommended |
| RW | GND | Write-only |
| A (backlight +) | 5V or digital pin for PWM dimming | |
| K (backlight −) | GND | |

**Note:** A4 is the hardware I2C SDA pin on Uno R4. Using it for D7 precludes I2C — acceptable since no I2C devices are used. A5 remains free for future use.

Previously free pins consumed by LCD: 10, A0–A4. A5 and BTN_4 (pin 8) remain free.

---

## Constants to add to PioneerController.ino

```cpp
#include <LiquidCrystal.h>

const int PIN_LCD_RS = 10;
const int PIN_LCD_EN = A0;
const int PIN_LCD_D4 = A1;
const int PIN_LCD_D5 = A2;
const int PIN_LCD_D6 = A3;
const int PIN_LCD_D7 = A4;

LiquidCrystal lcd(PIN_LCD_RS, PIN_LCD_EN, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);
```

---

## Display state machine

### States

```cpp
enum LcdState {
  LCD_BOOT,    // startup message, shown briefly
  LCD_IDLE,    // cycling flavor text
  LCD_SCENE,   // scene name, shown 2s then → IDLE
  LCD_VOLUME   // volume bar, shown 2s then → IDLE
};
```

### Globals

```cpp
LcdState      lcdState    = LCD_BOOT;
unsigned long lcdStateMs  = 0;
uint8_t       idleIndex   = 0;
int           lcdVolLevel = 8;   // relative vol indicator, 0–16, starts mid
```

### Timing constants

```cpp
const unsigned long LCD_BOOT_MS   = 1500;  // boot message hold
const unsigned long LCD_EVENT_MS  = 2000;  // scene/volume hold
const unsigned long LCD_IDLE_MS   = 8000;  // flavor text cycle interval
```

### Content

**BOOT** (shown once on startup):
```
PANOPTICORP RSU
UNIT 7 ONLINE
```

**IDLE** — cycling flavor text (8s per message):
```
[0] PANOPTICORP RSU  / ALL ZONES CLEAR
[1] MONITORING ZONE  / ACTIVITY: NORMAL
[2] SCAN MODE: AUTO  / 7 ZONES ACTIVE
[3] ACCESS GRANTED   / CLEARANCE: LVL3
[4] UPLINK: NOMINAL  / AWAITING INPUT
```

**SCENE** — shown 2s on SCENE_NEXT:
```
SCENE_OFF:    PANOPTICORP RSU / STANDBY MODE
SCENE_RED:    SCENE: PATROL   / RED ACTIVE
SCENE_GREEN:  SCENE: SWEEP    / GREEN ACTIVE
```

**VOLUME** — shown 2s on VOL_UP or VOL_DOWN:
```
Row 0: VOLUME
Row 1: ████████░░░░░░░░  (16-char block bar, lcdVolLevel filled blocks)
```

Block char: `0xFF` (full block). Empty: `0x20` (space).

---

## Implementation

### In `setup()` (replaces `// TODO: initialize LCD`)

```cpp
lcd.begin(16, 2);
lcd.setCursor(0, 0); lcd.print("PANOPTICORP RSU ");
lcd.setCursor(0, 1); lcd.print("UNIT 7 ONLINE   ");
lcdState   = LCD_BOOT;
lcdStateMs = millis();
```

### `updateLcd()` — called from `loop()` each iteration

```cpp
void updateLcd() {
  unsigned long now = millis();

  if (lcdState == LCD_BOOT && now - lcdStateMs >= LCD_BOOT_MS) {
    lcdState   = LCD_IDLE;
    lcdStateMs = now;
    idleIndex  = 0;
    showIdleMessage(0);
    return;
  }

  if (lcdState == LCD_IDLE && now - lcdStateMs >= LCD_IDLE_MS) {
    lcdStateMs = now;
    idleIndex  = (idleIndex + 1) % 5;
    showIdleMessage(idleIndex);
    return;
  }

  if ((lcdState == LCD_SCENE || lcdState == LCD_VOLUME) &&
      now - lcdStateMs >= LCD_EVENT_MS) {
    lcdState   = LCD_IDLE;
    lcdStateMs = now;
    showIdleMessage(idleIndex);
  }
}
```

### `showIdleMessage()`

```cpp
const char* IDLE_ROW0[] = {
  "PANOPTICORP RSU ", "MONITORING ZONE ", "SCAN MODE: AUTO ",
  "ACCESS GRANTED  ", "UPLINK: NOMINAL "
};
const char* IDLE_ROW1[] = {
  "ALL ZONES CLEAR ", "ACTIVITY: NORMAL", "7 ZONES ACTIVE  ",
  "CLEARANCE: LVL3 ", "AWAITING INPUT  "
};

void showIdleMessage(uint8_t idx) {
  lcd.setCursor(0, 0); lcd.print(IDLE_ROW0[idx]);
  lcd.setCursor(0, 1); lcd.print(IDLE_ROW1[idx]);
}
```

### `showSceneMessage()` — call on SCENE_NEXT

```cpp
void showSceneMessage(uint8_t scene) {
  lcd.setCursor(0, 0);
  switch (scene) {
    case 0: lcd.print("PANOPTICORP RSU "); break;
    case 1: lcd.print("SCENE: PATROL   "); break;
    case 2: lcd.print("SCENE: SWEEP    "); break;
  }
  lcd.setCursor(0, 1);
  switch (scene) {
    case 0: lcd.print("STANDBY MODE    "); break;
    case 1: lcd.print("RED ACTIVE      "); break;
    case 2: lcd.print("GREEN ACTIVE    "); break;
  }
  lcdState   = LCD_SCENE;
  lcdStateMs = millis();
}
```

### `showVolumeMessage()` — call on VOL_UP / VOL_DOWN

```cpp
void showVolumeMessage() {
  lcd.setCursor(0, 0); lcd.print("VOLUME          ");
  lcd.setCursor(0, 1);
  for (int i = 0; i < 16; i++) {
    lcd.write(i < lcdVolLevel ? 0xFF : 0x20);
  }
  lcdState   = LCD_VOLUME;
  lcdStateMs = millis();
}
```

### Triggering from event handlers

In `PulseVolumeUp()`:
```cpp
if (lcdVolLevel < 16) lcdVolLevel++;
showVolumeMessage();
```

In `PulseVolumeDown()`:
```cpp
if (lcdVolLevel > 0) lcdVolLevel--;
showVolumeMessage();
```

In protothread3 BTN_3 handler (after `Serial1.print(EVT_SCENE_NEXT)`):
```cpp
// currentLcdScene must be tracked as a global (0/1/2, mirrors hub scene state)
currentLcdScene = (currentLcdScene + 1) % 3;
showSceneMessage(currentLcdScene);
```

Add `uint8_t currentLcdScene = 0;` to globals.

---

## Wiring notes

- V0 (contrast) via 10kΩ trimmer from 5V to GND, wiper to V0. Adjust for visibility.
- If backlight is dim: check A/K polarity. Most modules need a current-limiting resistor (47–100Ω) on A.
- RW tied to GND — write-only mode, saves one pin.

---

## Verification

- Boot: LCD shows "PANOPTICORP RSU / UNIT 7 ONLINE" for ~1.5s
- After boot: transitions to first idle message
- Idle: flavor text cycles every 8s
- Volume turn: LCD shows volume bar for 2s, returns to idle
- Scene cycle (BTN_3): LCD shows scene name for 2s, returns to idle
- All transitions non-blocking — main loop continues running during hold periods
- If LCD is blank: check contrast pot, confirm RS/EN/D4–D7 wiring
- If garbled output: confirm 4-bit mode wiring (D0–D3 must be unconnected)
