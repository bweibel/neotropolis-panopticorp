// StripTest — XIAO ESP32-S3 / WS2812B 150 LED sensor bar
// Send a number over Serial to run a test:
//   0  All off
//   1  All white (dim) — confirms strip responds and wiring is good
//   2  All red   (scene color)
//   3  All green (scene color)
//   4  Chase — single white pixel sweeps 0→149, then back; confirms LED count
//   5  GRB order check — first 3 LEDs: R, G, B separately
//   6  Brightness ramp — fades white in and out repeatedly

#include <FastLED.h>

const int PIN_STRIP = 1;   // D0
const int NUM_LEDS  = 150;
CRGB leds[NUM_LEDS];

const CRGB COLOR_RED   = CRGB(0xCC, 0x00, 0x00);
const CRGB COLOR_GREEN = CRGB(0x00, 0xCC, 0x00);

int currentTest = 0;

// ---------------------------------------------------------------------------

void testOff() {
  FastLED.clear(true);
}

void testWhite() {
  fill_solid(leds, NUM_LEDS, CRGB::White);
  FastLED.setBrightness(40);
  FastLED.show();
}

void testRed() {
  fill_solid(leds, NUM_LEDS, COLOR_RED);
  FastLED.setBrightness(200);
  FastLED.show();
}

void testGreen() {
  fill_solid(leds, NUM_LEDS, COLOR_GREEN);
  FastLED.setBrightness(200);
  FastLED.show();
}

void testChase() {
  FastLED.setBrightness(180);
  // Forward
  for (int i = 0; i < NUM_LEDS; i++) {
    FastLED.clear();
    leds[i] = CRGB::White;
    FastLED.show();
    delay(15);
  }
  // Reverse
  for (int i = NUM_LEDS - 1; i >= 0; i--) {
    FastLED.clear();
    leds[i] = CRGB::White;
    FastLED.show();
    delay(15);
  }
  FastLED.clear(true);
  Serial.println("Chase complete — did all 150 LEDs light?");
}

void testGrbOrder() {
  // Pixel 0 should be RED, pixel 1 GREEN, pixel 2 BLUE.
  // If colors are wrong, GRB order constant needs fixing.
  FastLED.clear();
  FastLED.setBrightness(180);
  leds[0] = CRGB::Red;
  leds[1] = CRGB::Green;
  leds[2] = CRGB::Blue;
  FastLED.show();
  Serial.println("GRB check: pixel 0=RED, 1=GREEN, 2=BLUE");
}

void testRamp() {
  for (int cycle = 0; cycle < 3; cycle++) {
    fill_solid(leds, NUM_LEDS, CRGB::White);
    for (int b = 0; b <= 200; b += 2) {
      FastLED.setBrightness(b);
      FastLED.show();
      delay(10);
    }
    for (int b = 200; b >= 0; b -= 2) {
      FastLED.setBrightness(b);
      FastLED.show();
      delay(10);
    }
  }
  FastLED.clear(true);
  Serial.println("Ramp complete");
}

// ---------------------------------------------------------------------------

void runTest(int t) {
  Serial.printf("Running test %d\n", t);
  switch (t) {
    case 0: testOff();      break;
    case 1: testWhite();    break;
    case 2: testRed();      break;
    case 3: testGreen();    break;
    case 4: testChase();    break;
    case 5: testGrbOrder(); break;
    case 6: testRamp();     break;
    default:
      Serial.println("Unknown test. Send 0-6.");
      return;
  }
}

void printMenu() {
  Serial.println("---");
  Serial.println("0 off | 1 white | 2 red | 3 green | 4 chase | 5 GRB-check | 6 ramp");
}

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<WS2812B, PIN_STRIP, GRB>(leds, NUM_LEDS);
  FastLED.clear(true);
  Serial.println("StripTest ready");
  printMenu();
}

void loop() {
  if (Serial.available()) {
    String s = Serial.readStringUntil('\n');
    s.trim();
    if (s.length() > 0) {
      currentTest = s.toInt();
      runTest(currentTest);
      printMenu();
    }
  }
}
