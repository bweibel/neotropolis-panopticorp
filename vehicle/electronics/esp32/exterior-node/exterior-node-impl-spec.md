# ESP32 Exterior Node: Implementation Spec

## Purpose

Spec for implementing the full runtime of `exterior-node.ino`. The stub has pin constants, FastLED setup, MOSFET pin init, and ESP-NOW code that must be removed and replaced with a wired serial listener.

---

## Architecture Change: Wired serial replaces ESP-NOW

The interior hub is now a **RP2040** with no wireless. Scene index arrives via a **wired UART TX line** from the RP2040 instead of ESP-NOW. The exterior node is still an ESP32 — no MCU change here.

### Remove from stub before implementing:
- `#include <esp_now.h>`
- `#include <WiFi.h>`
- `SceneMessage` struct
- `volatile` qualifier on `currentScene` (no longer written from ISR/callback)
- `onReceive()` callback function
- All ESP-NOW + WiFi init in `setup()`

### Add to stub:
- `const int PIN_HUB_RX` — UART RX pin from RP2040 TX (assign free GPIO, define as constant)
- `Serial2` for hub receive

### Wire requirement:
One wire: RP2040 `PIN_EXT_TX` → ESP32 `PIN_HUB_RX`, plus shared ground between the two battery modules.

---

## Part 1: Serial listener (Hub → Exterior Node)

Receives scene index as a newline-terminated integer string (`"0\n"`, `"1\n"`, `"2\n"`) from the RP2040 at 9600 baud.

### Add pin constant:

```cpp
const int PIN_HUB_RX = 16;  // adjust to actual wiring — must not conflict with existing pin assignments
const int PIN_HUB_TX = 17;  // unused, assigned for Serial2 init
```

Accent elements are reassigned from their stub values to free GPIOs (see pin conflict resolution below). Updated assignments:
- `PIN_ACCENT_1` → 4
- `PIN_ACCENT_2` → 2

### In `setup()`:

```cpp
Serial2.begin(9600, SERIAL_8N1, PIN_HUB_RX, PIN_HUB_TX);
```

### Parsing:

```cpp
void readSerial() {
  static String buf = "";
  while (Serial2.available()) {
    char c = Serial2.read();
    if (c == '\n') {
      buf.trim();
      int scene = buf.toInt();
      if (scene >= SCENE_OFF && scene <= SCENE_GREEN) {
        currentScene = (uint8_t)scene;
      }
      buf = "";
    } else {
      buf += c;
    }
  }
}
```

Scene index is validated against the known range before applying. Out-of-range values are silently dropped.

Call `readSerial()` at the top of `loop()` every iteration.

---

## Part 2: Glitter animation (all WS2812B zones)

Same algorithm as the interior hub. Applied to all four WS2812B zones: side strips (x2), hood scoop, accent elements.

### Add timing globals:

```cpp
unsigned long lastGlitterMs = 0;
const unsigned long GLITTER_INTERVAL_MS = 30;  // ~33fps
```

### Glitter function (identical to hub):

```cpp
void runGlitter(CRGB* strip, int numLeds, CRGB baseColor) {
  CRGB base = baseColor;
  base.nscale8(BASE_BRIGHTNESS);
  fill_solid(strip, numLeds, base);

  int glitterCount = max(1, (int)(numLeds * GLITTER_DENSITY));
  for (int i = 0; i < glitterCount; i++) {
    int idx = random16(numLeds);
    uint8_t brightness = random8(GLITTER_MIN, GLITTER_MAX);
    strip[idx] = baseColor;
    strip[idx].nscale8(brightness);
  }
}
```

### updateLighting():

```cpp
void updateLighting() {
  if (currentScene == SCENE_OFF) {
    FastLED.clear();
    FastLED.show();
    applySceneToWheelwells(SCENE_OFF);
    return;
  }

  unsigned long now = millis();
  if (now - lastGlitterMs >= GLITTER_INTERVAL_MS) {
    lastGlitterMs = now;
    CRGB color = (currentScene == SCENE_RED) ? COLOR_RED : COLOR_GREEN;
    runGlitter(sideLeft,  SIDE_NUM_LEDS,   color);
    runGlitter(sideRight, SIDE_NUM_LEDS,   color);
    runGlitter(hoodScoop, HOOD_NUM_LEDS,   color);
    runGlitter(accent1,   ACCENT_NUM_LEDS, color);
    runGlitter(accent2,   ACCENT_NUM_LEDS, color);
    FastLED.show();
    applySceneToWheelwells(currentScene);
  }
}
```

---

## Part 3: Wheel well MOSFET PWM (stretch goal)

Same pattern as hub footwells. Only implement if time allows — these can run standalone or be cut entirely.

**Assumption: N-channel MOSFETs, common cathode (active HIGH).** Invert if common anode.

```cpp
void applySceneToWheelwells(uint8_t scene) {
  uint8_t r = 0, g = 0, b = 0;
  if (scene == SCENE_RED)   { r = 0xCC; }
  if (scene == SCENE_GREEN) { g = 0xCC; }

  // Front left
  analogWrite(PIN_WHEEL_FL_R, r); analogWrite(PIN_WHEEL_FL_G, g); analogWrite(PIN_WHEEL_FL_B, b);
  // Front right
  analogWrite(PIN_WHEEL_FR_R, r); analogWrite(PIN_WHEEL_FR_G, g); analogWrite(PIN_WHEEL_FR_B, b);
  // Rear left
  analogWrite(PIN_WHEEL_RL_R, r); analogWrite(PIN_WHEEL_RL_G, g); analogWrite(PIN_WHEEL_RL_B, b);
  // Rear right
  analogWrite(PIN_WHEEL_RR_R, r); analogWrite(PIN_WHEEL_RR_G, g); analogWrite(PIN_WHEEL_RR_B, b);
}
```

If wheel wells are cut, replace `applySceneToWheelwells()` with an empty stub — `updateLighting()` calls it unconditionally.

Call once in `setup()` after pin modes:

```cpp
applySceneToWheelwells(SCENE_OFF);
```

---

## Final `loop()`:

```cpp
void loop() {
  readSerial();
  updateLighting();
}
```

---

## Pin assignments to update in stub

The following constants differ from the original stub and must be updated:

| Constant | Old | New | Reason |
|---|---|---|---|
| `PIN_ACCENT_1` | 16 | 4 | 16 reassigned to serial RX |
| `PIN_ACCENT_2` | 17 | 2 | 17 reassigned to serial TX |
| `PIN_HUB_RX` | (new) | 16 | Serial RX from RP2040 hub |
| `PIN_HUB_TX` | (new) | 17 | Serial TX, unused |

Note: pin 2 has an internal pulldown and onboard LED on some ESP32 modules — neither affects FastLED WS2812B output. If the onboard LED is a nuisance during testing, swap `PIN_ACCENT_2` to pin 15.

Also note: **pin 34 is input-only** on most ESP32 variants. It is currently assigned to `PIN_FOOTWELL_R_B`. If using an ESP32 that has pin 34 as input-only, reassign `PIN_FOOTWELL_R_B` to another free GPIO.

---

## Summary of changes to stub

| Location | Change |
|---|---|
| Includes | Remove `esp_now.h`, `WiFi.h` |
| Globals | Remove `SceneMessage`, `volatile` on `currentScene`; add `PIN_HUB_RX = 16`, `PIN_HUB_TX = 17`, `lastGlitterMs`, `GLITTER_INTERVAL_MS`; update `PIN_ACCENT_1 = 4`, `PIN_ACCENT_2 = 2` |
| `setup()` | Remove ESP-NOW + WiFi block; add `Serial2.begin(...)`; add `applySceneToWheelwells(SCENE_OFF)` |
| Remove functions | `onReceive()` |
| New functions | `readSerial()`, `runGlitter()`, `updateLighting()`, `applySceneToWheelwells()` |
| `loop()` | Replace TODOs with `readSerial()`, `updateLighting()` |

---

## Open questions

- [ ] **Wheel well strip polarity** — common anode or common cathode?
- [ ] **Pin 34** — confirm whether this ESP32 variant allows output on pin 34. If not, reassign `PIN_FOOTWELL_R_B`.
- [ ] **`PIN_HUB_RX` assignment** — pick a free GPIO after resolving accent pin conflict. Must match `PIN_EXT_TX` on the RP2040 hub (physically wired together).
- [ ] **Hood scoop and accent LED counts** — `HOOD_NUM_LEDS` and `ACCENT_NUM_LEDS` are placeholder values (8 each). Update once strips are measured.

---

## Verification

- Boot: all strips off, wheel wells off
- Scene `1` received on `Serial2`: all WS2812B zones glitter red, wheel wells solid red
- Scene `2` received: all zones glitter green, wheel wells solid green
- Scene `0` received: all strips clear, wheel wells off
- Out-of-range integer received: silently ignored, scene unchanged
- Glitter runs ~30fps, non-blocking
