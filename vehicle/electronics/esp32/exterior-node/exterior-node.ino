// =============================================================================
// exterior-node — ESP32 #2 (with camera)
// Role: Receives scene changes from interior hub via ESP-NOW and controls
//       exterior lighting zones.
//
// Power: Battery USB-C 100W
// See: vehicle/electronics/baja-lighting-spec.md
// =============================================================================

#include <FastLED.h>
#include <esp_now.h>
#include <WiFi.h>
#include "scenes.h"

// =============================================================================
// Pin constants — exterior lighting zones
// =============================================================================

// Side strips (WS2812B, 150 LEDs each)
const int PIN_SIDE_LEFT  = 13;
const int PIN_SIDE_RIGHT = 14;

// Hood scoop (WS2812B, <10 LEDs — join nearest side data line or own pin)
const int PIN_HOOD_SCOOP = 15;

// Accent elements (WS2812B, ~8 LEDs each)
const int PIN_ACCENT_1 = 16;
const int PIN_ACCENT_2 = 17;

// Wheel well static RGB strips (MOSFET PWM — stretch goal)
// Front left
const int PIN_WHEEL_FL_R = 25;
const int PIN_WHEEL_FL_G = 26;
const int PIN_WHEEL_FL_B = 27;
// Front right
const int PIN_WHEEL_FR_R = 32;
const int PIN_WHEEL_FR_G = 33;
const int PIN_WHEEL_FR_B = 34;
// Rear left
const int PIN_WHEEL_RL_R = 18;
const int PIN_WHEEL_RL_G = 19;
const int PIN_WHEEL_RL_B = 21;
// Rear right
const int PIN_WHEEL_RR_R = 22;
const int PIN_WHEEL_RR_G = 23;
const int PIN_WHEEL_RR_B = 5;

// =============================================================================
// LED strip configuration
// =============================================================================

const int SIDE_NUM_LEDS   = 150;
const int HOOD_NUM_LEDS   = 8;    // adjust to actual count
const int ACCENT_NUM_LEDS = 8;    // adjust to actual count

CRGB sideLeft[SIDE_NUM_LEDS];
CRGB sideRight[SIDE_NUM_LEDS];
CRGB hoodScoop[HOOD_NUM_LEDS];
CRGB accent1[ACCENT_NUM_LEDS];
CRGB accent2[ACCENT_NUM_LEDS];

// =============================================================================
// ESP-NOW — receive from interior hub
// =============================================================================

typedef struct SceneMessage {
  uint8_t scene;  // 0 = off, 1 = red, 2 = green
} SceneMessage;

// =============================================================================
// State
// =============================================================================

volatile uint8_t currentScene = SCENE_OFF;

// =============================================================================
// ESP-NOW receive callback
// =============================================================================

void onReceive(const esp_now_recv_info_t* info, const uint8_t* data, int len) {
  if (len == sizeof(SceneMessage)) {
    SceneMessage msg;
    memcpy(&msg, data, sizeof(msg));
    currentScene = msg.scene;
  }
}

// =============================================================================
// TODO: implement
//   - Glitter animation for all WS2812B zones when scene != SCENE_OFF (non-blocking)
//   - MOSFET PWM output for wheel well static RGB strips (stretch goal)
//   - Scene OFF: all strips off, all MOSFETs LOW
// =============================================================================

void setup() {
  Serial.begin(115200);

  // FastLED
  FastLED.addLeds<WS2812B, PIN_SIDE_LEFT,  GRB>(sideLeft,  SIDE_NUM_LEDS);
  FastLED.addLeds<WS2812B, PIN_SIDE_RIGHT, GRB>(sideRight, SIDE_NUM_LEDS);
  FastLED.addLeds<WS2812B, PIN_HOOD_SCOOP, GRB>(hoodScoop, HOOD_NUM_LEDS);
  FastLED.addLeds<WS2812B, PIN_ACCENT_1,   GRB>(accent1,   ACCENT_NUM_LEDS);
  FastLED.addLeds<WS2812B, PIN_ACCENT_2,   GRB>(accent2,   ACCENT_NUM_LEDS);
  FastLED.clear(true);

  // Wheel well MOSFET pins (stretch goal)
  pinMode(PIN_WHEEL_FL_R, OUTPUT); pinMode(PIN_WHEEL_FL_G, OUTPUT); pinMode(PIN_WHEEL_FL_B, OUTPUT);
  pinMode(PIN_WHEEL_FR_R, OUTPUT); pinMode(PIN_WHEEL_FR_G, OUTPUT); pinMode(PIN_WHEEL_FR_B, OUTPUT);
  pinMode(PIN_WHEEL_RL_R, OUTPUT); pinMode(PIN_WHEEL_RL_G, OUTPUT); pinMode(PIN_WHEEL_RL_B, OUTPUT);
  pinMode(PIN_WHEEL_RR_R, OUTPUT); pinMode(PIN_WHEEL_RR_G, OUTPUT); pinMode(PIN_WHEEL_RR_B, OUTPUT);

  // ESP-NOW
  WiFi.mode(WIFI_STA);
  Serial.print("Exterior node MAC: ");
  Serial.println(WiFi.macAddress());
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }
  esp_now_register_recv_cb(onReceive);
}

void loop() {
  // TODO: run glitter animation on all WS2812B zones when currentScene != SCENE_OFF
  // TODO: update wheel well MOSFET PWM to match current scene color (stretch)
  // TODO: on SCENE_OFF, clear all strips and set all MOSFETs LOW
}
