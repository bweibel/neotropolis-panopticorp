# ESP32-S3 Interior Hub: Implementation Spec

## Purpose

Spec for porting `interior-hub.ino` from the QT Py RP2040 to the **Hosyond ESP32-S3 N16R8**. The existing RP2040 implementation is the reference — logic is identical, only the Serial API and pin assignments change.

---

## Architecture: ESP32-S3 N16R8 replaces QT Py RP2040

All three lighting/display boards are now **Hosyond ESP32-S3 N16R8** (16MB flash, 8MB PSRAM). Board target: `esp32:esp32:esp32s3`.

Key differences from RP2040 implementation:

- **Serial API:** ESP32 uses `Serial1.begin(baud, config, rx_pin, tx_pin)` — no separate `setRX()`/`setTX()` calls needed. Pins passed directly to `begin()`.
- **Serial2 shared TX:** Hub broadcasts scene index to both exterior node and IRIS display on a single Serial2 TX line. Both receivers are wired to the same TX pin and parse identical format — no third UART needed.
- **`analogWrite()`:** Fully supported on ESP32-S3.
- **FastLED:** Fully supported on ESP32-S3.
- **Pin assignments:** QT Py RP2040 pin numbers (6, 3, 26, 5, 20, 4) are board-specific and must be reassigned to Hosyond ESP32-S3 GPIOs. TBD on receipt of hardware.

---

## Part 1: Serial listener (Uno → Hub)

Receives one-way serial from Uno R4 at 9600 baud on `PIN_SERIAL_RX`.

### In `setup()`:

```cpp
Serial1.begin(9600, SERIAL_8N1, PIN_SERIAL_RX, PIN_SERIAL_TX);
```

ESP32 passes RX/TX pins directly to `begin()` — no `setRX()`/`setTX()` needed.

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
void handleEvent(const char* evt) {
  if (strcmp(evt, "SCENE_NEXT") == 0) {
    advanceScene();
  } else if (strcmp(evt, "SCENE_PREV") == 0) {
    retreatScene();
  }
  // VOL_UP, VOL_DOWN, MUTE, SKIP_FWD, SKIP_BACK: no action for MVP
  // Unknown events: silently ignored
  Serial.print("EVT: "); Serial.println(evt);  // debug via USB Serial
}
```

Call `readSerial()` at the top of `loop()` every iteration.

---

## Part 2: Scene management + exterior TX

### Add pin constants to globals:

```cpp
const int PIN_BROADCAST_TX = /* TBD */;  // Serial2 TX → exterior node RX + IRIS RX (shared line)
```

One TX pin wired to two receiver RX pins. Both receive identical scene index integers at 9600 baud — electrically valid over short car-interior runs.

### UART assignment:

| UART | Role | Direction |
|---|---|---|
| Serial | USB CDC | Debug output only |
| Serial1 | Uno R4 → Hub | RX only |
| Serial2 | Hub → Exterior Node + IRIS | TX only (shared) |

### In `setup()`:

```cpp
Serial1.begin(9600, SERIAL_8N1, PIN_SERIAL_RX, -1);      // Uno RX
Serial2.begin(9600, SERIAL_8N1, -1, PIN_BROADCAST_TX);   // broadcast TX
```

### advanceScene() and retreatScene():

```cpp
void advanceScene() {
  currentScene = (currentScene + 1) % 3;  // 0 → 1 → 2 → 0
  broadcastScene(currentScene);
  applySceneToFootwells(currentScene);
  Serial.print("Scene: "); Serial.println(currentScene);
}
```

```cpp
void retreatScene() {
  currentScene = (currentScene + 2) % 3;  // -1 mod 3: 0 → 2 → 1 → 0
  broadcastScene(currentScene);
  applySceneToFootwells(currentScene);
  Serial.print("Scene: "); Serial.println(currentScene);
}
```

### broadcastScene():

```cpp
void broadcastScene(uint8_t scene) {
  Serial2.println(scene);  // sends "0\n", "1\n", or "2\n" to exterior node + IRIS simultaneously
}
```

Single TX line reaches both exterior node and IRIS display. Both parse the same format.

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

## Summary of changes for port

| Location | Change |
|---|---|
| Board target | `rp2040:rp2040:adafruit_qtpy_rp2040` → `esp32:esp32:esp32s3` |
| Serial init | `Serial1.setRX()/setTX()/begin()` → `Serial1.begin(baud, config, rx, tx)` |
| Pin constants | QT Py GPIO numbers → Hosyond ESP32-S3 GPIOs (TBD on hardware receipt) |
| `advanceScene()` | Replace `sendSceneToExterior()` + `sendSceneToIris()` with `broadcastScene()` |
| New globals | `PIN_BROADCAST_TX` (replaces `PIN_EXT_TX`, covers both receivers) |
| New function | `broadcastScene()` — single Serial2 TX to exterior node + IRIS |
| `setup()` | `Serial2.begin(9600, SERIAL_8N1, -1, PIN_BROADCAST_TX)` |

Logic, animation, and footwell functions are unchanged.

---

## Open questions

- [ ] **Pin assignments** — all QT Py RP2040 pin numbers must be remapped to Hosyond ESP32-S3 GPIOs once hardware is in hand. Avoid strapping pins (0, 45, 46) and flash pins (26–32 on some variants — confirm datasheet).
- [ ] **UART assignment** — confirmed: Serial=debug, Serial1=Uno RX, Serial2=broadcast TX (shared to exterior node + IRIS). No third UART needed.
- [ ] **Footwell strip polarity** — common anode or common cathode?

---

## Verification

- Boot: all lights off, footwells off, `Serial2` broadcasting scene 0 to exterior node + IRIS
- `SCENE_NEXT` received on `Serial1`: scene advances, footwells update, both exterior node and IRIS receive integer over shared `Serial2` TX
- Scene RED: dashboard glitters red, footwells solid red
- Scene GREEN: dashboard glitters green, footwells solid green
- Scene OFF: strips clear, footwells off
- Glitter runs ~30fps, non-blocking
