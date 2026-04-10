# RP2040 Interior Hub: Implementation Spec

## Purpose

Spec for implementing the full runtime of `interior-hub.ino`. The stub has pin constants, FastLED setup, MOSFET pin init, and a placeholder for ESP-NOW (now removed — see Architecture Change below).

---

## Architecture Change: RP2040 replaces ESP32

The interior hub MCU is a **RP2040** (e.g. Raspberry Pi Pico), not an ESP32. Use the **arduino-pico** core (Earle Philhower). This affects:

- **ESP-NOW is gone.** RP2040 has no wireless. Scene index is sent to the exterior node over a **wired UART TX line** instead.
- **Serial UARTs:** RP2040 has two hardware UARTs — `Serial1` and `Serial2` in arduino-pico. Use `Serial1` for Uno RX, `Serial2` for exterior node TX (or vice versa — assign consistently with pin constants).
- **FastLED:** Fully supported on arduino-pico.
- **`analogWrite()`:** Fully supported — RP2040 has hardware PWM on every GPIO.
- **Pin 34 input-only concern from prior draft:** Not applicable — that's an ESP32 variant issue.

### Remove from stub before implementing:
- `#include <esp_now.h>`
- `#include <WiFi.h>`
- `SceneMessage` struct
- `peerInfo` global
- `sendSceneToExterior()` function
- All ESP-NOW init in `setup()`
- `EXTERIOR_NODE_MAC` constant

### Add to stub:
- `const int PIN_EXT_TX` — UART TX pin to exterior node (assign a free GPIO, define as constant)
- `Serial2` for exterior node TX

---

## Part 1: Serial listener (Uno → Hub)

Receives one-way serial from Uno R4 at 9600 baud on `PIN_SERIAL_RX`.

### In `setup()`:

```cpp
Serial1.setRX(PIN_SERIAL_RX);
Serial1.setTX(PIN_SERIAL_TX);  // unused but set for completeness
Serial1.begin(9600);
```

arduino-pico requires explicit `setRX`/`setTX` before `begin()` when using non-default pins.

### Parsing:

```cpp
void readSerial() {
  static String buf = "";
  while (Serial1.available()) {
    char c = Serial1.read();
    if (c == '\n') {
      buf.trim();
      handleEvent(buf);
      buf = "";
    } else {
      buf += c;
    }
  }
}
```

### Event handler:

```cpp
void handleEvent(const String& evt) {
  if (evt == "SCENE_NEXT") {
    advanceScene();
  }
  // VOL_UP, VOL_DOWN, MUTE, SKIP_FWD, SKIP_BACK: no action for MVP
  // Unknown events: silently ignored
  Serial.println("EVT: " + evt);  // debug via USB Serial
}
```

Call `readSerial()` at the top of `loop()` every iteration.

---

## Part 2: Scene management + exterior TX

### Add pin constant to globals:

```cpp
const int PIN_EXT_TX = 4;  // assign a free GPIO — adjust to actual wiring
```

### In `setup()`:

```cpp
Serial2.setTX(PIN_EXT_TX);
Serial2.begin(9600);
```

### advanceScene():

```cpp
void advanceScene() {
  currentScene = (currentScene + 1) % 3;  // 0 → 1 → 2 → 0
  sendSceneToExterior(currentScene);
  applySceneToFootwells(currentScene);
  Serial.print("Scene: "); Serial.println(currentScene);
}
```

### sendSceneToExterior() — replaces the ESP-NOW version:

```cpp
void sendSceneToExterior(uint8_t scene) {
  Serial2.println(scene);  // sends "0\n", "1\n", or "2\n"
}
```

Simple integer string, newline terminated. Exterior node parses the integer directly — no need for named event strings since scene index is unambiguous.

---

## Part 3: Glitter animation (dashboard WS2812B)

Identical to prior draft — no changes from RP2040 swap.

### Add timing globals:

```cpp
unsigned long lastGlitterMs = 0;
const unsigned long GLITTER_INTERVAL_MS = 30;  // ~33fps
```

### Glitter function:

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
    return;
  }

  unsigned long now = millis();
  if (now - lastGlitterMs >= GLITTER_INTERVAL_MS) {
    lastGlitterMs = now;
    CRGB color = (currentScene == SCENE_RED) ? COLOR_RED : COLOR_GREEN;
    runGlitter(dashLeft,  DASH_NUM_LEDS, color);
    runGlitter(dashRight, DASH_NUM_LEDS, color);
    FastLED.show();
  }
}
```

---

## Part 4: Footwell MOSFET PWM

Static color, updated only on scene change. Confirm strip polarity before wiring.

**Assumption: N-channel MOSFETs, common cathode (active HIGH).** Invert values if common anode.

```cpp
void applySceneToFootwells(uint8_t scene) {
  uint8_t r = 0, g = 0, b = 0;
  if (scene == SCENE_RED)   { r = 0xCC; }
  if (scene == SCENE_GREEN) { g = 0xCC; }

  analogWrite(PIN_FOOTWELL_L_R, r);
  analogWrite(PIN_FOOTWELL_L_G, g);
  analogWrite(PIN_FOOTWELL_L_B, b);
  analogWrite(PIN_FOOTWELL_R_R, r);
  analogWrite(PIN_FOOTWELL_R_G, g);
  analogWrite(PIN_FOOTWELL_R_B, b);
}
```

Call from `advanceScene()` and once in `setup()` after pin modes:

```cpp
applySceneToFootwells(SCENE_OFF);
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

## Summary of changes to stub

| Location | Change |
|---|---|
| Includes | Remove `esp_now.h`, `WiFi.h` |
| Globals | Remove `EXTERIOR_NODE_MAC`, `SceneMessage`, `peerInfo`; add `PIN_EXT_TX`, `lastGlitterMs`, `GLITTER_INTERVAL_MS` |
| `setup()` | Replace ESP-NOW block with `Serial1` + `Serial2` init; add `applySceneToFootwells(SCENE_OFF)` |
| `sendSceneToExterior()` | Replace ESP-NOW send with `Serial2.println(scene)` |
| New functions | `readSerial()`, `handleEvent()`, `advanceScene()`, `runGlitter()`, `updateLighting()`, `applySceneToFootwells()` |
| `loop()` | Replace TODOs with `readSerial()`, `updateLighting()` |

---

## Open questions

- [ ] **Footwell strip polarity** — common anode or common cathode? Determines whether PWM values are inverted.
- [ ] **`PIN_EXT_TX` assignment** — pick a free GPIO consistent with physical wire routing, set as named constant.
- [ ] **RP2040 variant** — confirm which board (Pico, Pico W, other). Select correct board in arduino-pico. Pico W has WiFi but it's not used here.
- [ ] **arduino-pico core** — install via Arduino Board Manager: `https://github.com/earlephilhower/arduino-pico`

---

## Verification

- Boot: all lights off, footwells off, `Serial2` sending scene 0 to exterior
- `SCENE_NEXT` received on `Serial1`: scene advances, footwells update, exterior receives integer over `Serial2`
- Scene RED: dashboard glitters red, footwells solid red
- Scene GREEN: dashboard glitters green, footwells solid green
- Scene OFF: strips clear, footwells off
- Glitter runs ~30fps, non-blocking
