// =============================================================================
// interior-hub — Hosyond ESP32-S3 N16R8
// Role: Listens to Uno R4 serial events, controls interior lighting zones,
//       and forwards scene changes to exterior node and IRIS display via
//       wired UART (Serial2, shared TX line).
//
// Power: Battery USB-C 18W
// Board target: esp32:esp32:esp32s3
// See: vehicle/electronics/baja-lighting-spec.md
// =============================================================================

#include <FastLED.h>
#include "scenes.h"

// =============================================================================
// Pin constants
// =============================================================================

// Serial1: RX from Uno R4 (one-way receive)
const int PIN_SERIAL_RX    = 16;
const int PIN_SERIAL_TX    = 17;  // unused; required by Serial1.begin()

// Serial2: shared TX → exterior node RX + IRIS RX
const int PIN_BROADCAST_TX = 18;

// Dashboard WS2812B strips (2.7mm, 160 LEDs each)
const int PIN_DASH_LEFT    =  5;
const int PIN_DASH_RIGHT   =  6;

// Footwell static RGB strips — N-channel MOSFET, active HIGH
// TODO: confirm polarity before install (common cathode assumed; invert values if common anode)
const int PIN_FOOTWELL_L_R =  1;
const int PIN_FOOTWELL_L_G =  2;
const int PIN_FOOTWELL_L_B =  4;
const int PIN_FOOTWELL_R_R =  7;
const int PIN_FOOTWELL_R_G =  8;
const int PIN_FOOTWELL_R_B =  9;

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

void broadcastScene(uint8_t scene) {
  Serial2.println(scene);  // "0\n", "1\n", or "2\n" → exterior node + IRIS
}

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

void advanceScene() {
  currentScene = (currentScene + 1) % 3;
  broadcastScene(currentScene);
  applySceneToFootwells(currentScene);
  Serial.print("Scene: "); Serial.println(currentScene);
}

void retreatScene() {
  currentScene = (currentScene + 2) % 3;  // -1 mod 3
  broadcastScene(currentScene);
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
  Serial1.begin(9600, SERIAL_8N1, PIN_SERIAL_RX, PIN_SERIAL_TX);

  // Serial2: broadcast scene index to exterior node + IRIS display
  Serial2.begin(9600, SERIAL_8N1, -1, PIN_BROADCAST_TX);

  // FastLED
  FastLED.addLeds<WS2812B, PIN_DASH_LEFT,  GRB>(dashLeft,  DASH_NUM_LEDS);
  FastLED.addLeds<WS2812B, PIN_DASH_RIGHT, GRB>(dashRight, DASH_NUM_LEDS);
  FastLED.clear(true);

  // Footwells
  pinMode(PIN_FOOTWELL_L_R, OUTPUT);
  pinMode(PIN_FOOTWELL_L_G, OUTPUT);
  pinMode(PIN_FOOTWELL_L_B, OUTPUT);
  pinMode(PIN_FOOTWELL_R_R, OUTPUT);
  pinMode(PIN_FOOTWELL_R_G, OUTPUT);
  pinMode(PIN_FOOTWELL_R_B, OUTPUT);
  applySceneToFootwells(SCENE_OFF);
}

void loop() {
  readSerial();
  updateLighting();
}
