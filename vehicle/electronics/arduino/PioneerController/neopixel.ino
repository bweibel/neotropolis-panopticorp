// =============================================================================
// knob_led — single WS2811 above the volume knob (pin A5)
// Mirrors boot drama phases; low green idle; red on mute.
// Boot step thresholds must match leds.ino constants.
// =============================================================================

Adafruit_NeoPixel knobPixel(1, PIN_KNOB_LED, NEO_RGB + NEO_KHZ400);

static bool          knobStrobe    = false;
static unsigned long knobStrobeMs  = 0;
static bool          knobBootDone  = false;

#define KNOB_GREEN_IDLE  knobPixel.Color(0, 40, 0)
#define KNOB_GREEN_FULL  knobPixel.Color(0, 180, 0)
#define KNOB_RED         knobPixel.Color(200, 0, 0)
#define KNOB_WHITE       knobPixel.Color(180, 180, 180)
#define KNOB_OFF         0

void knobLedInit() {
  knobPixel.begin();
  knobPixel.setBrightness(255);
  knobPixel.clear();
  knobPixel.show();
  knobBootDone = false;
}

static void knobShow(uint32_t color) {
  knobPixel.setPixelColor(0, color);
  knobPixel.show();
}

void updateKnobLed() {
  if (!ledMasterOn) {
    knobShow(KNOB_OFF);
    return;
  }

  // Post-boot: mute takes priority, otherwise low green
  if (lcdState != LCD_BOOT) {
    if (!knobBootDone) {
      knobBootDone = true;
      knobShow(isMuted ? KNOB_RED : KNOB_GREEN_IDLE);
    } else {
      knobShow(isMuted ? KNOB_RED : KNOB_GREEN_IDLE);
    }
    return;
  }

  knobBootDone = false;
  unsigned long now = millis();

  // ACCESS GRANTED — solid green
  if (bootStep >= BOOT_STEPS - 2) {
    knobShow(KNOB_GREEN_FULL);
    return;
  }

  // ACCESS DENIED — red strobe
  if (bootStep >= BOOT_STEP_DENIED_START && bootStep < BOOT_STEP_OVERRIDE_START) {
    if (now - knobStrobeMs >= 150) {
      knobStrobeMs = now;
      knobStrobe   = !knobStrobe;
      knobShow(knobStrobe ? KNOB_RED : KNOB_OFF);
    }
    return;
  }

  // Auth / override drama — white strobe
  if (bootStep >= BOOT_STEP_AUTH_START) {
    if (now - knobStrobeMs >= 150) {
      knobStrobeMs = now;
      knobStrobe   = !knobStrobe;
      knobShow(knobStrobe ? KNOB_WHITE : KNOB_OFF);
    }
    return;
  }

  // Normal boot scan — dim green steady
  knobShow(KNOB_GREEN_IDLE);
}
