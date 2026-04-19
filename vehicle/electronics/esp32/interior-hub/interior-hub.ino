// =============================================================================
// interior-hub — QT PY RP2040 (Adafruit 4900)
// Role: Listens to Uno R4 serial events, controls interior lighting zones,
//       and forwards scene changes to exterior node via wired UART (Serial2).
//
// Power: Battery USB-C 18W
// Core: arduino-pico (Earle Philhower)
// See: vehicle/electronics/baja-lighting-spec.md
// =============================================================================

#include <FastLED.h>
#include "scenes.h"

// =============================================================================
// Pin constants
// =============================================================================

// Dashboard WS2812B strips (2.7mm, 160 LEDs each)
const int PIN_DASH_LEFT  = 6;   // SCK/D8
const int PIN_DASH_RIGHT = 3;   // MO/D10

// Footwell static RGB strips — single on/off signal via MOSFET
const int PIN_FOOTWELL   = 26;  // A3/D3

// Serial RX from Uno R4 (one-way receive)
const int PIN_SERIAL_RX  = 5;   // RX/D7
const int PIN_SERIAL_TX  = 20;  // TX/D6 (unused)

// Serial TX to exterior node
const int PIN_EXT_TX     = 4;   // MI/D9

// =============================================================================
// LED strip configuration
// =============================================================================

const int DASH_NUM_LEDS = 160;

CRGB dashLeft[DASH_NUM_LEDS];
CRGB dashRight[DASH_NUM_LEDS];

// =============================================================================
// State
// =============================================================================

uint8_t currentScene = SCENE_OFF;

unsigned long lastGlitterMs = 0;
const unsigned long GLITTER_INTERVAL_MS = 30;  // ~33fps

// =============================================================================
// Scene management
// =============================================================================

void sendSceneToExterior(uint8_t scene) {
  Serial2.println(scene);  // sends "0\n", "1\n", or "2\n"
}

void applySceneToFootwells(uint8_t scene) {
  digitalWrite(PIN_FOOTWELL, scene == SCENE_OFF ? LOW : HIGH);
}

void advanceScene() {
  currentScene = (currentScene + 1) % 3;
  sendSceneToExterior(currentScene);
  applySceneToFootwells(currentScene);
  Serial.print("Scene: "); Serial.println(currentScene);
}

void retreatScene() {
  currentScene = (currentScene + 2) % 3;  // -1 mod 3
  sendSceneToExterior(currentScene);
  applySceneToFootwells(currentScene);
  Serial.print("Scene: "); Serial.println(currentScene);
}

// =============================================================================
// Serial listener (Uno R4 → hub)
// =============================================================================

void handleEvent(const char* evt) {
  if (strcmp(evt, "SCENE_NEXT") == 0) {
    advanceScene();
  } else if (strcmp(evt, "SCENE_PREV") == 0) {
    retreatScene();
  }
  // VOL_UP, VOL_DOWN, MUTE, SKIP_FWD, SKIP_BACK: no action for MVP
  // Unknown events: silently ignored
  Serial.print("EVT: "); Serial.println(evt);
}

void readSerial() {
  static char buf[32];
  static uint8_t bufIdx = 0;
  while (Serial1.available()) {
    char c = Serial1.read();
    if (c == '\n') {
      buf[bufIdx] = '\0';
      if (bufIdx > 0) {
        handleEvent(buf);
      }
      bufIdx = 0;
    } else if (c != '\r' && bufIdx < sizeof(buf) - 1) {
      buf[bufIdx++] = c;
    }
  }
}

// =============================================================================
// Glitter animation (dashboard WS2812B)
// =============================================================================

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

// =============================================================================
// Setup + loop
// =============================================================================

void setup() {
  Serial.begin(115200);

  // Serial1: receive from Uno R4
  Serial1.setRX(PIN_SERIAL_RX);
  Serial1.setTX(PIN_SERIAL_TX);
  Serial1.begin(9600);

  // Serial2: send scene index to exterior node
  Serial2.setTX(PIN_EXT_TX);
  Serial2.begin(9600);

  // FastLED
  FastLED.addLeds<WS2812B, PIN_DASH_LEFT,  GRB>(dashLeft,  DASH_NUM_LEDS);
  FastLED.addLeds<WS2812B, PIN_DASH_RIGHT, GRB>(dashRight, DASH_NUM_LEDS);
  FastLED.clear(true);

  // Footwell on/off
  pinMode(PIN_FOOTWELL, OUTPUT);
  applySceneToFootwells(SCENE_OFF);
}

void loop() {
  readSerial();
  updateLighting();
}
