// KnobLedTest — WS2812B single LED on pin A5, Uno R4 WiFi
//
// Cycles through red → green → blue → white → off, 1s per step.
// Open Serial Monitor at 115200 to see current color.
//
// Wiring:
//   WS2812B VCC → 5V
//   WS2812B GND → GND
//   WS2812B DIN → A5 (with 300–500Ω series resistor recommended)

#include <Adafruit_NeoPixel.h>

#define PIN        A5
#define NUM_LEDS   1

Adafruit_NeoPixel strip(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);

struct Color { uint8_t r, g, b; const char* name; };

static const Color COLORS[] = {
  { 255,   0,   0, "RED"   },
  {   0, 255,   0, "GREEN" },
  {   0,   0, 255, "BLUE"  },
  { 255, 255, 255, "WHITE" },
  {   0,   0,   0, "OFF"   },
};
static const uint8_t NUM_COLORS = sizeof(COLORS) / sizeof(COLORS[0]);

void setup() {
  Serial.begin(115200);
  strip.begin();
  strip.setBrightness(64);  // ~25% — safe for bench testing without heatsink
  strip.show();
  Serial.println("KnobLedTest ready");
}

void loop() {
  static uint8_t idx = 0;
  static unsigned long lastMs = 0;

  if (millis() - lastMs >= 1000) {
    lastMs = millis();
    const Color& c = COLORS[idx];
    strip.setPixelColor(0, strip.Color(c.r, c.g, c.b));
    strip.show();
    Serial.println(c.name);
    idx = (idx + 1) % NUM_COLORS;
  }
}
