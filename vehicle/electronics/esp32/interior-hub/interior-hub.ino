// =============================================================================
// interior-hub — ESP32 #1 (no camera)
// Role: Listens to Uno R4 serial events, controls interior lighting zones,
//       and forwards scene changes to exterior node via ESP-NOW.
//
// Power: Battery USB-C 18W
// See: vehicle/electronics/baja-lighting-spec.md
// =============================================================================

#include <FastLED.h>
#include <esp_now.h>
#include <WiFi.h>
#include "scenes.h"

// =============================================================================
// Pin constants — interior lighting zones
// =============================================================================

// Dashboard WS2812B strips (2.7mm, 160 LEDs each)
const int PIN_DASH_LEFT  = 13;
const int PIN_DASH_RIGHT = 14;

// Floor footwell static RGB strips (MOSFET PWM, 3 pins per zone)
// Zone layout: LEFT footwell
const int PIN_FOOTWELL_L_R = 25;
const int PIN_FOOTWELL_L_G = 26;
const int PIN_FOOTWELL_L_B = 27;
// Zone layout: RIGHT footwell
const int PIN_FOOTWELL_R_R = 32;
const int PIN_FOOTWELL_R_G = 33;
const int PIN_FOOTWELL_R_B = 34;

// Serial RX from Uno R4 (one-way receive)
const int PIN_SERIAL_RX = 16;
const int PIN_SERIAL_TX = 17;  // unused but assigned

// =============================================================================
// LED strip configuration
// =============================================================================

const int DASH_NUM_LEDS = 160;

CRGB dashLeft[DASH_NUM_LEDS];
CRGB dashRight[DASH_NUM_LEDS];

// =============================================================================
// ESP-NOW — exterior node
// =============================================================================

uint8_t EXTERIOR_NODE_MAC[] = { 0xE8, 0xF6, 0x0A, 0x8B, 0xDC, 0x34 };

typedef struct SceneMessage {
  uint8_t scene;  // 0 = off, 1 = red, 2 = green
} SceneMessage;

esp_now_peer_info_t peerInfo;

// =============================================================================
// State
// =============================================================================

uint8_t currentScene = SCENE_OFF;

// =============================================================================
// TODO: implement
//   - Serial listener (HardwareSerial or SoftwareSerial on PIN_SERIAL_RX)
//     Parse VOL_UP, VOL_DOWN, MUTE, SKIP_FWD, SKIP_BACK, SCENE_NEXT
//   - Scene transition: update currentScene, send ESP-NOW SceneMessage
//   - Glitter animation for dashboard WS2812B strips (non-blocking, main loop)
//   - MOSFET PWM output for footwell static RGB strips
// =============================================================================

void sendSceneToExterior(uint8_t scene) {
  SceneMessage msg = { scene };
  esp_now_send(EXTERIOR_NODE_MAC, (uint8_t*)&msg, sizeof(msg));
}

void setup() {
  Serial.begin(115200);

  // FastLED
  FastLED.addLeds<WS2812B, PIN_DASH_LEFT,  GRB>(dashLeft,  DASH_NUM_LEDS);
  FastLED.addLeds<WS2812B, PIN_DASH_RIGHT, GRB>(dashRight, DASH_NUM_LEDS);
  FastLED.clear(true);

  // Footwell MOSFET pins
  pinMode(PIN_FOOTWELL_L_R, OUTPUT);
  pinMode(PIN_FOOTWELL_L_G, OUTPUT);
  pinMode(PIN_FOOTWELL_L_B, OUTPUT);
  pinMode(PIN_FOOTWELL_R_R, OUTPUT);
  pinMode(PIN_FOOTWELL_R_G, OUTPUT);
  pinMode(PIN_FOOTWELL_R_B, OUTPUT);

  // ESP-NOW
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }
  memcpy(peerInfo.peer_addr, EXTERIOR_NODE_MAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  // TODO: initialize serial listener on PIN_SERIAL_RX at 9600 baud
}

void loop() {
  // TODO: read serial events from Uno
  // TODO: handle SCENE_NEXT — advance currentScene, call sendSceneToExterior()
  // TODO: run glitter animation on dashLeft / dashRight when scene != SCENE_OFF
  // TODO: update footwell MOSFET PWM to match current scene color
}
