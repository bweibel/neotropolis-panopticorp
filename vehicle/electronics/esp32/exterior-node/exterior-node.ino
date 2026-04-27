// =============================================================================
// exterior-node — Hosyond ESP32-S3 N16R8
// Role: Receives scene index from interior hub via wired UART; controls side
//       strips, hood scoop, and accent elements with glitter animation.
//       Top scanner (sensor bar) is handled by a separate XIAO ESP32-S3 Sense.
//
// Power: Battery USB-C 100W
// Board target: esp32:esp32:esp32s3:PSRAM=opi,CDCOnBoot=cdc
// See: vehicle/electronics/baja-lighting-spec.md
// =============================================================================

#include <FastLED.h>
#include "scenes.h"

// =============================================================================
// Pin constants
// GPIOs 26–37 reserved on N16R8 (flash + OPI PSRAM) — avoid.
// =============================================================================

// Side strips (WS2812B, 150 LEDs each)
const int PIN_SIDE_LEFT  = 13;
const int PIN_SIDE_RIGHT = 14;

// Hood scoop (WS2812B, ~8 LEDs — adjust HOOD_NUM_LEDS to actual count)
const int PIN_HOOD_SCOOP = 15;

// Accent elements (WS2812B, ~8 LEDs each — adjust ACCENT_NUM_LEDS to actual count)
const int PIN_ACCENT_1 = 4;
const int PIN_ACCENT_2 = 2;

// Serial from Interior Hub (shared Serial2 TX line — also wired to IRIS and sensor bar)
const int PIN_HUB_RX = 16;
const int PIN_HUB_TX = 17;  // unused; required by Serial2.begin()

// Wheel well static RGB strips (MOSFET PWM — stretch goal, active HIGH assumed)
// WARNING: Several pins below fall in or near the OPI PSRAM range (26–37).
// Reassign before wiring. Confirmed safe on N16R8: 18–25, 38–40, 42–48.
const int PIN_WHEEL_FL_R = 18;
const int PIN_WHEEL_FL_G = 19;
const int PIN_WHEEL_FL_B = 21;
const int PIN_WHEEL_FR_R = 22;
const int PIN_WHEEL_FR_G = 23;
const int PIN_WHEEL_FR_B = 25;
const int PIN_WHEEL_RL_R = 38;
const int PIN_WHEEL_RL_G = 39;
const int PIN_WHEEL_RL_B = 40;
const int PIN_WHEEL_RR_R = 42;
const int PIN_WHEEL_RR_G = 43;
const int PIN_WHEEL_RR_B = 44;

// =============================================================================
// LED strip configuration
// =============================================================================

const int SIDE_NUM_LEDS   = 150;
const int HOOD_NUM_LEDS   = 8;   // adjust to actual count
const int ACCENT_NUM_LEDS = 8;   // adjust to actual count
// Top scanner (150 LEDs) handled by XIAO ESP32-S3 Sense sensor bar node

CRGB sideLeft[SIDE_NUM_LEDS];
CRGB sideRight[SIDE_NUM_LEDS];
CRGB hoodScoop[HOOD_NUM_LEDS];
CRGB accent1[ACCENT_NUM_LEDS];
CRGB accent2[ACCENT_NUM_LEDS];

// =============================================================================
// State
// =============================================================================

uint8_t currentScene = SCENE_OFF;

unsigned long lastGlitterMs = 0;
const unsigned long GLITTER_INTERVAL_MS = 30;  // ~33fps

// =============================================================================
// Serial listener — hub → exterior node
// =============================================================================

void readSerial() {
  static char buf[8];
  static uint8_t bufIdx = 0;
  while (Serial2.available()) {
    char c = Serial2.read();
    if (c == '\n') {
      buf[bufIdx] = '\0';
      if (bufIdx > 0) {
        int scene = atoi(buf);
        if (scene >= SCENE_OFF && scene <= SCENE_GREEN) {
          currentScene = (uint8_t)scene;
        }
      }
      bufIdx = 0;
    } else if (c != '\r' && bufIdx < sizeof(buf) - 1) {
      buf[bufIdx++] = c;
    }
  }
}

// =============================================================================
// Glitter animation
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

// =============================================================================
// Wheel well MOSFET PWM (stub — active HIGH)
// =============================================================================

void applySceneToWheelwells(uint8_t scene) {
  // stub — implement when wheel wells are wired
}

// =============================================================================
// Lighting update
// =============================================================================

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

// =============================================================================
// Setup / loop
// =============================================================================

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, PIN_HUB_RX, PIN_HUB_TX);

  // FastLED
  FastLED.addLeds<WS2812B, PIN_SIDE_LEFT,  GRB>(sideLeft,  SIDE_NUM_LEDS);
  FastLED.addLeds<WS2812B, PIN_SIDE_RIGHT, GRB>(sideRight, SIDE_NUM_LEDS);
  FastLED.addLeds<WS2812B, PIN_HOOD_SCOOP, GRB>(hoodScoop, HOOD_NUM_LEDS);
  FastLED.addLeds<WS2812B, PIN_ACCENT_1,   GRB>(accent1,   ACCENT_NUM_LEDS);
  FastLED.addLeds<WS2812B, PIN_ACCENT_2,   GRB>(accent2,   ACCENT_NUM_LEDS);
  FastLED.clear(true);

  // Wheel well MOSFET pins
  pinMode(PIN_WHEEL_FL_R, OUTPUT); pinMode(PIN_WHEEL_FL_G, OUTPUT); pinMode(PIN_WHEEL_FL_B, OUTPUT);
  pinMode(PIN_WHEEL_FR_R, OUTPUT); pinMode(PIN_WHEEL_FR_G, OUTPUT); pinMode(PIN_WHEEL_FR_B, OUTPUT);
  pinMode(PIN_WHEEL_RL_R, OUTPUT); pinMode(PIN_WHEEL_RL_G, OUTPUT); pinMode(PIN_WHEEL_RL_B, OUTPUT);
  pinMode(PIN_WHEEL_RR_R, OUTPUT); pinMode(PIN_WHEEL_RR_G, OUTPUT); pinMode(PIN_WHEEL_RR_B, OUTPUT);
  applySceneToWheelwells(SCENE_OFF);
}

void loop() {
  readSerial();
  updateLighting();
}
