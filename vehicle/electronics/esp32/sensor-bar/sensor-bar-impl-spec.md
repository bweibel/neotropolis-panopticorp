# Sensor Bar: Implementation Spec

## Overview

Write `SensorBar.ino` for the Seeed XIAO ESP32-S3 Sense. The sketch reads the onboard PDM microphone, computes an amplitude envelope, and drives 150 WS2812B LEDs with a brightness pulse in the current scene color. Scene index arrives via wired UART from the Interior Hub.

---

## Hardware

- **MCU:** Seeed Studio XIAO ESP32-S3 Sense
- **Board target:** `esp32:esp32:XIAO_ESP32S3`
- **Strip:** WS2812B, 150 LEDs, GRB order
- **Mic:** Onboard PDM (MSM261D3526H1CPM or equivalent), DATA=GPIO41, CLK=GPIO42

---

## Pin constants

```cpp
const int PIN_STRIP    =  1;  // D0 — WS2812B data
const int PIN_SCENE_RX = 44;  // D7 — Serial1 RX ← Hub Serial2 TX
const int PIN_SCENE_TX = 43;  // D6 — Serial1 TX (unused; required by begin())
const int PIN_PDM_DATA = 41;  // onboard PDM mic data
const int PIN_PDM_CLK  = 42;  // onboard PDM mic clock
```

---

## Dependencies

- **FastLED** — WS2812B strip control
- **ESP_I2S** (Arduino ESP32 core ≥3.x) — PDM mic access via I2S in PDM mode

---

## Scene constants

Copy `scenes.h` from `esp32/exterior-node/` into the sketch folder. It defines `SCENE_OFF`, `SCENE_RED`, `SCENE_GREEN`, `COLOR_RED`, `COLOR_GREEN`. The sensor bar does not use `GLITTER_*` constants.

---

## Part 1: Serial listener

Identical pattern to exterior node and interior hub. Parses scene index from Hub Serial2 TX shared line.

```cpp
uint8_t currentScene = SCENE_OFF;

void readSerial() {
  static char buf[8];
  static uint8_t bufIdx = 0;
  while (Serial1.available()) {
    char c = Serial1.read();
    if (c == '\n') {
      buf[bufIdx] = '\0';
      if (bufIdx > 0) {
        int scene = atoi(buf);
        if (scene >= SCENE_OFF && scene <= SCENE_GREEN)
          currentScene = (uint8_t)scene;
      }
      bufIdx = 0;
    } else if (c != '\r' && bufIdx < sizeof(buf) - 1) {
      buf[bufIdx++] = c;
    }
  }
}
```

Call at top of `loop()` every iteration.

---

## Part 2: PDM microphone

Use the ESP32 Arduino core `ESP_I2S` library (available in ESP32 Arduino core ≥3.x).

### Setup

```cpp
#include <ESP_I2S.h>

I2SClass i2s;

// In setup():
i2s.setPins(PIN_PDM_CLK, PIN_PDM_DATA, -1, -1, -1);  // bck, ws, data_in, data_out, mck — PDM uses bck=CLK, ws=DATA
i2s.begin(I2S_MODE_PDM_RX, 16000, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO);
```

> **Note:** Check ESP32 Arduino core docs for exact `setPins` argument order for PDM mode on ESP32-S3. The `I2S_MODE_PDM_RX` constant and pin mapping may differ slightly between core versions. If `ESP_I2S` is unavailable, fall back to the older `driver/i2s.h` C API with `I2S_MODE_PDM` flag.

### Non-blocking read

Read available samples each loop without blocking:

```cpp
const int MIC_BUF_LEN = 64;
int16_t micBuf[MIC_BUF_LEN];

int16_t readAmplitude() {
  size_t bytesRead = 0;
  esp_err_t err = i2s_read(I2S_NUM_0, micBuf, sizeof(micBuf), &bytesRead, 0);  // timeout=0: non-blocking
  if (err != ESP_OK || bytesRead == 0) return 0;

  int samplesRead = bytesRead / sizeof(int16_t);
  int32_t peak = 0;
  for (int i = 0; i < samplesRead; i++) {
    int32_t s = abs((int32_t)micBuf[i]);
    if (s > peak) peak = s;
  }
  return (int16_t)min(peak, (int32_t)32767);
}
```

Returns peak absolute value over the sample window. Returns 0 if no samples available.

---

## Part 3: Amplitude envelope

Smooth the raw peak reading with an attack/decay envelope follower. Fast attack preserves transients; slow decay keeps the strip visually alive between beats.

```cpp
float envelope = 0.0f;

const float ATTACK  = 0.8f;   // fraction of gap closed per frame toward peak (tune up/down)
const float DECAY   = 0.05f;  // fraction subtracted per frame when signal drops (tune)
const int   AMP_MAX = 8000;   // expected peak amplitude ceiling (tune to room noise floor)
const uint8_t BRIGHTNESS_MIN = 20;   // floor brightness when signal is near zero
const uint8_t BRIGHTNESS_MAX = 255;

void updateEnvelope(int16_t rawPeak) {
  float target = (float)rawPeak / AMP_MAX;
  if (target > 1.0f) target = 1.0f;
  if (target > envelope)
    envelope += ATTACK * (target - envelope);
  else
    envelope -= DECAY;
  if (envelope < 0.0f) envelope = 0.0f;
}

uint8_t envelopeToBrightness() {
  return (uint8_t)(BRIGHTNESS_MIN + envelope * (BRIGHTNESS_MAX - BRIGHTNESS_MIN));
}
```

`AMP_MAX`, `ATTACK`, and `DECAY` are the primary tuning knobs. Adjust on hardware against actual room conditions.

---

## Part 4: FastLED strip

```cpp
#include <FastLED.h>

const int NUM_LEDS = 150;
CRGB leds[NUM_LEDS];

// In setup():
FastLED.addLeds<WS2812B, PIN_STRIP, GRB>(leds, NUM_LEDS);
FastLED.clear(true);
```

### Update function

```cpp
void updateStrip() {
  if (currentScene == SCENE_OFF) {
    FastLED.clear();
    FastLED.show();
    return;
  }

  int16_t raw = readAmplitude();
  updateEnvelope(raw);
  uint8_t brightness = envelopeToBrightness();

  CRGB color = (currentScene == SCENE_RED) ? COLOR_RED : COLOR_GREEN;
  fill_solid(leds, NUM_LEDS, color);
  FastLED.setBrightness(brightness);
  FastLED.show();
}
```

Call from `loop()` every iteration — non-blocking because `readAmplitude()` uses timeout=0.

---

## Setup and loop

```cpp
void setup() {
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, PIN_SCENE_RX, PIN_SCENE_TX);

  // PDM mic init (see Part 2 note re: exact API)
  i2s.setPins(PIN_PDM_CLK, PIN_PDM_DATA, -1, -1, -1);
  i2s.begin(I2S_MODE_PDM_RX, 16000, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO);

  FastLED.addLeds<WS2812B, PIN_STRIP, GRB>(leds, NUM_LEDS);
  FastLED.clear(true);

  Serial.println("SensorBar init complete");
}

void loop() {
  readSerial();
  updateStrip();
}
```

---

## Upload

```sh
arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32S3 SensorBar/
arduino-cli upload -p /dev/ttyACM0 --fqbn esp32:esp32:XIAO_ESP32S3 SensorBar/
arduino-cli monitor -p /dev/ttyACM0 -c baudrate=115200
```

---

## Verification

- Boot: strip off, Serial prints "SensorBar init complete"
- Scene 0: strip stays off regardless of audio
- Scene 1: strip glows red, brightness rises with clapping/loud audio
- Scene 2: strip glows green, same behaviour
- Scene change: strip responds within one serial packet (~1ms at 9600 baud)
- No blocking — `loop()` runs continuously without delay

---

## Tuning guide

| Parameter | Effect | Direction |
|---|---|---|
| `AMP_MAX` | Amplitude ceiling | Lower = more sensitive; raise if strip is always at max |
| `ATTACK` | How fast strip responds to loud sounds | Higher = snappier |
| `DECAY` | How fast strip fades after a sound | Lower = longer trails |
| `BRIGHTNESS_MIN` | Floor brightness (strip never fully off in active scene) | 0 = fully off between sounds |

---

## Open items

- [ ] Confirm PDM mic GPIO numbers against XIAO Sense schematic (41/42 assumed)
- [ ] Confirm `ESP_I2S` API for PDM mode on ESP32-S3 XIAO target — may need adjustment
- [ ] Confirm 150 LED count against physical strip
- [ ] Tune `AMP_MAX`, `ATTACK`, `DECAY` on hardware
