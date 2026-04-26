// =============================================================================
// neopixel_knob — WS2812B single LED above rotary encoder (pin A5)
// Role: Boot sync, idle glow, interaction flash
//
// Must sort after lcd.ino and leds.ino — depends on LcdState enum,
// lcdState, bootStep, and BOOT_STEP_* constants defined there.
//
// Boot phases mirror LED matrix:
//   Scan     (0–14)  : dim orange, brightens with progress bar
//   Auth     (15–30) : dim white strobe
//   Denied   (31–35) : red strobe
//   Override (36–39) : dim white strobe
//   Granted  (40+)   : medium orange
// Post-boot idle : very dim orange
// Volume event   : bright orange flash (300ms)
// Track event    : amber flash (350ms)
// =============================================================================

#include <Adafruit_NeoPixel.h>

#define KNOB_LED_PIN   A5
#define KNOB_LED_COUNT 1

Adafruit_NeoPixel knobLed(KNOB_LED_COUNT, KNOB_LED_PIN, NEO_GRB + NEO_KHZ800);

static enum KnobPhase { KNOB_BOOT, KNOB_IDLE, KNOB_FLASH } knobPhase = KNOB_BOOT;

static uint32_t      knobFlashColor    = 0;
static unsigned long knobFlashDuration = 0;
static unsigned long knobFlashMs       = 0;
static bool          knobStrobe        = false;
static unsigned long knobStrobeMs      = 0;
static uint32_t      knobLastColor     = 0xFF000000;  // sentinel: valid colors always have top byte = 0

const unsigned long KNOB_STROBE_MS = 150;

// Only call show() when color changes — avoids interrupt disruption on every loop()
static void knobSet(uint32_t color) {
  if (color == knobLastColor) return;
  knobLastColor = color;
  knobLed.setPixelColor(0, color);
  knobLed.show();
}

void knobLedInit() {
  knobLed.begin();
  knobLed.show();
  knobPhase     = KNOB_BOOT;
  knobLastColor = 0xFF000000;
}

// Called from PulseVolumeUp() / PulseVolumeDown()
void triggerKnobLedVolume() {
  knobPhase         = KNOB_FLASH;
  knobFlashColor    = knobLed.Color(140, 35, 0);  // bright orange
  knobFlashDuration = 300;
  knobFlashMs       = millis();
}

// Called from PulseTrackForward() / PulseTrackBack()
void triggerKnobLedTrack() {
  knobPhase         = KNOB_FLASH;
  knobFlashColor    = knobLed.Color(120, 90, 10);  // amber — distinct from base orange
  knobFlashDuration = 350;
  knobFlashMs       = millis();
}

void updateKnobLed() {
  unsigned long now = millis();

  // Interaction flash — highest priority
  if (knobPhase == KNOB_FLASH) {
    if (now - knobFlashMs < knobFlashDuration) {
      knobSet(knobFlashColor);
      return;
    }
    knobPhase = (lcdState == LCD_BOOT) ? KNOB_BOOT : KNOB_IDLE;
  }

  // Post-boot: very dim orange
  if (lcdState != LCD_BOOT) {
    knobPhase = KNOB_IDLE;
    knobSet(knobLed.Color(15, 3, 0));
    return;
  }

  // Boot: GRANTED — medium orange
  if (bootStep >= BOOT_STEPS - 2) {
    knobSet(knobLed.Color(80, 20, 0));
    return;
  }

  // Boot: DENIED — red strobe
  if (bootStep >= BOOT_STEP_DENIED_START && bootStep < BOOT_STEP_OVERRIDE_START) {
    if (now - knobStrobeMs >= KNOB_STROBE_MS) {
      knobStrobeMs = now;
      knobStrobe   = !knobStrobe;
    }
    knobSet(knobStrobe ? knobLed.Color(150, 0, 0) : 0);
    return;
  }

  // Boot: AUTH / OVERRIDE — dim white strobe
  if (bootStep >= BOOT_STEP_AUTH_START) {
    if (now - knobStrobeMs >= KNOB_STROBE_MS) {
      knobStrobeMs = now;
      knobStrobe   = !knobStrobe;
    }
    knobSet(knobStrobe ? knobLed.Color(50, 50, 40) : 0);
    return;
  }

  // Boot: SCAN — dim orange brightening with progress (steps 0–14)
  uint8_t b = map(bootStep, 0, 14, 2, 18);
  knobSet(knobLed.Color(b * 10, b * 2, 0));
}
