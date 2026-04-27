// CameraTest — XIAO ESP32-S3 Sense
// Frame differencing detects motion in left/right halves of camera frame.
// Left motion → left half of strip brightness, right motion → right half.
// Strip is red throughout; brightness encodes motion magnitude per side.

#include <FastLED.h>
#include "esp_camera.h"

// ---------------------------------------------------------------------------
// Camera pins — XIAO ESP32-S3 Sense / OV2640
// ---------------------------------------------------------------------------
#define PWDN_GPIO_NUM  -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM  10
#define SIOD_GPIO_NUM  40
#define SIOC_GPIO_NUM  39
#define Y9_GPIO_NUM    48
#define Y8_GPIO_NUM    11
#define Y7_GPIO_NUM    12
#define Y6_GPIO_NUM    14
#define Y5_GPIO_NUM    16
#define Y4_GPIO_NUM    18
#define Y3_GPIO_NUM    17
#define Y2_GPIO_NUM    15
#define VSYNC_GPIO_NUM 38
#define HREF_GPIO_NUM  47
#define PCLK_GPIO_NUM  13

// ---------------------------------------------------------------------------
// FastLED
// ---------------------------------------------------------------------------
const int PIN_STRIP = 1;   // D0
const int NUM_LEDS  = 150;
CRGB leds[NUM_LEDS];

// ---------------------------------------------------------------------------
// Camera / motion
// ---------------------------------------------------------------------------
const int FRAME_W = 96;
const int FRAME_H = 96;

uint8_t prevFrame[FRAME_W * FRAME_H];
bool    prevFrameValid = false;
bool    cameraOk = false;

float motionLeft  = 0.0f;
float motionRight = 0.0f;

// MOTION_SCALE: lower = more sensitive. Tune if strip is always dark or always bright.
const float MOTION_SCALE  = 30.0f;
const float MOTION_ATTACK = 0.8f;
const float MOTION_DECAY  = 0.06f;

void setupCamera() {
  camera_config_t cfg = {};
  cfg.ledc_channel = LEDC_CHANNEL_0;
  cfg.ledc_timer   = LEDC_TIMER_0;
  cfg.pin_d0       = Y2_GPIO_NUM;
  cfg.pin_d1       = Y3_GPIO_NUM;
  cfg.pin_d2       = Y4_GPIO_NUM;
  cfg.pin_d3       = Y5_GPIO_NUM;
  cfg.pin_d4       = Y6_GPIO_NUM;
  cfg.pin_d5       = Y7_GPIO_NUM;
  cfg.pin_d6       = Y8_GPIO_NUM;
  cfg.pin_d7       = Y9_GPIO_NUM;
  cfg.pin_xclk     = XCLK_GPIO_NUM;
  cfg.pin_pclk     = PCLK_GPIO_NUM;
  cfg.pin_vsync    = VSYNC_GPIO_NUM;
  cfg.pin_href     = HREF_GPIO_NUM;
  cfg.pin_sccb_sda = SIOD_GPIO_NUM;
  cfg.pin_sccb_scl = SIOC_GPIO_NUM;
  cfg.pin_pwdn     = PWDN_GPIO_NUM;
  cfg.pin_reset    = RESET_GPIO_NUM;
  cfg.xclk_freq_hz = 20000000;
  cfg.pixel_format = PIXFORMAT_GRAYSCALE;
  cfg.frame_size   = FRAMESIZE_96X96;
  cfg.fb_count     = 1;
  cfg.fb_location  = CAMERA_FB_IN_DRAM;  // avoid PSRAM dependency
  cfg.grab_mode    = CAMERA_GRAB_LATEST;

  esp_err_t err = esp_camera_init(&cfg);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x — is OV2640 module attached?\n", err);
    cameraOk = false;
  } else {
    Serial.println("Camera OK");
    cameraOk = true;
  }
}

void updateMotion() {
  if (!cameraOk) return;
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) { Serial.println("Frame capture failed"); cameraOk = false; return; }

  if (prevFrameValid) {
    uint32_t diffLeft = 0, diffRight = 0;
    const int halfW = FRAME_W / 2;

    for (int y = 0; y < FRAME_H; y++) {
      for (int x = 0; x < FRAME_W; x++) {
        int idx  = y * FRAME_W + x;
        int diff = abs((int)fb->buf[idx] - (int)prevFrame[idx]);
        if (x < halfW) diffLeft  += diff;
        else            diffRight += diff;
      }
    }

    // Normalize to 0-1 range
    float rawLeft  = (float)diffLeft  / (FRAME_H * halfW * MOTION_SCALE);
    float rawRight = (float)diffRight / (FRAME_H * halfW * MOTION_SCALE);
    rawLeft  = constrain(rawLeft,  0.0f, 1.0f);
    rawRight = constrain(rawRight, 0.0f, 1.0f);

    // Envelope follower per side
    if (rawLeft > motionLeft)
      motionLeft  += MOTION_ATTACK * (rawLeft  - motionLeft);
    else
      motionLeft  = max(0.0f, motionLeft  - MOTION_DECAY);

    if (rawRight > motionRight)
      motionRight += MOTION_ATTACK * (rawRight - motionRight);
    else
      motionRight = max(0.0f, motionRight - MOTION_DECAY);
  }

  memcpy(prevFrame, fb->buf, FRAME_W * FRAME_H);
  prevFrameValid = true;
  esp_camera_fb_return(fb);
}

void updateStrip() {
  const int half = NUM_LEDS / 2;
  uint8_t briLeft  = (uint8_t)(motionLeft  * 255);
  uint8_t briRight = (uint8_t)(motionRight * 255);

  for (int i = 0; i < half; i++) {
    leds[i] = CRGB::Red;
    leds[i].nscale8(briLeft);
  }
  for (int i = half; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Red;
    leds[i].nscale8(briRight);
  }
  FastLED.setBrightness(255);
  FastLED.show();
}

// ---------------------------------------------------------------------------
// Setup / loop
// ---------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);

  FastLED.addLeds<WS2812B, PIN_STRIP, GRB>(leds, NUM_LEDS);
  FastLED.clear(true);

  setupCamera();
}

void loop() {
  updateMotion();
  updateStrip();

  static uint32_t lastPrint = 0;
  if (millis() - lastPrint >= 200) {
    lastPrint = millis();
    Serial.printf("left=%.2f right=%.2f\n", motionLeft, motionRight);
  }
}
