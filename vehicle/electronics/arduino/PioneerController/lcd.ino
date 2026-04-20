// =============================================================================
// lcd — WH1602B-TMI-JT character LCD state machine
// Role: Boot sequence, idle flavor text, event-driven messages
// =============================================================================

LiquidCrystal lcd(PIN_LCD_RS, PIN_LCD_EN, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);

// Custom characters — loaded into CGRAM slots 0–6 (8 slots total, slot 7 free)
byte CC_RARROW_DEF[8]   = { 0x10, 0x18, 0x1C, 0x1E, 0x1C, 0x18, 0x10, 0x00 };  // ▶ filled right arrow
byte CC_LARROW_DEF[8]   = { 0x01, 0x03, 0x07, 0x0F, 0x07, 0x03, 0x01, 0x00 };  // ◀ filled left arrow
byte CC_EYE_DEF[8]      = { 0x00, 0x0E, 0x11, 0x15, 0x11, 0x0E, 0x00, 0x00 };  // ⊙ oval outline with pupil
byte CC_SMILEY_DEF[8]   = { 0x00, 0x00, 0x0A, 0x00, 0x11, 0x0E, 0x00, 0x00 };  // :) smiley face
byte CC_ANTENNA_DEF[8]  = { 0x04, 0x0A, 0x11, 0x04, 0x04, 0x04, 0x0E, 0x00 };  // ʎ antenna — spread + mast + base
byte CC_ARROWUP_DEF[8]  = { 0x04, 0x0E, 0x1F, 0x04, 0x04, 0x04, 0x00, 0x00 };  // ↑ filled up arrow with stem
byte CC_ARROWDN_DEF[8]  = { 0x00, 0x04, 0x04, 0x04, 0x1F, 0x0E, 0x04, 0x00 };  // ↓ filled down arrow with stem

#define CC_RARROW   0
#define CC_LARROW   1
#define CC_EYE      2
#define CC_SMILEY   3
#define CC_ANTENNA  4
#define CC_ARROWUP  5
#define CC_ARROWDN  6

enum LcdState { LCD_BOOT, LCD_IDLE, LCD_SCENE, LCD_VOLUME, LCD_TRACK };

LcdState      lcdState        = LCD_BOOT;
unsigned long lcdStateMs      = 0;
uint8_t       idleIndex       = 0;
uint8_t       bootStep        = 0;
uint8_t       currentLcdScene = 0;  // mirrors hub scene state (0/1/2)

const unsigned long LCD_EVENT_MS = 2000;
const unsigned long LCD_IDLE_MS  = 8000;

// Boot sequence — each frame has its own duration, row 0 text, and row 1 text.
// r1 == nullptr means draw the dynamic progress bar for that step.
// Full bar string: '[' + 14×0xFF + ']' (16 chars)
#define FULL_BAR  "[\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF]"

struct BootFrame { uint16_t ms; const char* r0; const char* r1; };
const BootFrame BOOT_FRAMES[] = {
  // ms    row 0                  row 1 (nullptr = dynamic bar)
  { 2400, nullptr,               nullptr    },  //  0  splash  ⊙ PANOPTICORP ⊙
  {  600, "PANOPTICORP RSU ",    nullptr    },  //  1
  {  600, "PANOPTICORP RSU ",    nullptr    },  //  2
  {  600, "PANOPTICORP RSU ",    nullptr    },  //  3
  {  600, "PANOPTICORP RSU ",    nullptr    },  //  4
  {  600, "STARTUP SEQUENCE",    nullptr    },  //  5
  {  600, "STARTUP SEQUENCE",    nullptr    },  //  6
  {  600, "CHECKING SYSTEMS",    nullptr    },  //  7
  {  600, "CHECKING SYSTEMS",    nullptr    },  //  8
  {  600, "CHECKING SYSTEMS",    nullptr    },  //  9
  {  600, "MAINFRAME:BOOTED",    nullptr    },  // 10
  {  600, "WEAPONS: NOMINAL",    nullptr    },  // 11
  {  600, "SCANNERS:NOMINAL",    nullptr    },  // 12
  {  600, "SECURITY CHECK  ",    nullptr    },  // 13
  {  600, "SECURITY CHECK  ",    nullptr    },  // 14  bar full
  // --- error ---
  {  1200,"AUTHORIZING USER",   "[--------------]"  },  // 15
  {  200,"AUTHORIZING USER",    "[\xFF------------]"  },  // 15
  {  200,"AUTHORIZING USER",    "[\xFF\xFF------------]"  },  // 15
  {  900, "ACCESS DENIED   ",    FULL_BAR   },  // 15
  {  120, "#CC3SS D3N13D % ",    "[\xFF\xFF$\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF]" },  // 16  glitch
  {  300, "ACCESS DENIED   ",    FULL_BAR   },  // 17
  {  120, "4CC3$$ @#N!3D ! ",    "[\xFF\xFF\xFF\xFF\xFF%\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF]"},  // 18  glitch
  {  120, "ACC!$S ##N!3D   ",    "[#\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF]"},  // 19  glitch
  // --- override ---
  {  900, "OVERRIDE ACTIVE ",    FULL_BAR   },  // 20
  {  120, "0V3RR!D3 ACT!V3 ",   "[\xFF\xFF\xFF\xFF$\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF]"},  // 21  glitch
  {  300, "OVERRIDE ACTIVE ",    FULL_BAR   },  // 22
  {  120, "0V3RR!D3 4CT!V3 ",   "[\xFF\xFF\xFF\xFF\xFF\xFF\xFF#\xFF\xFF\xFF\xFF\xFF\xFF]"},  // 23  glitch
  // --- granted ---
  { 1200, "ACCESS GRANTED  ",    FULL_BAR   },  // 24
  { 1800, "ACCESS GRANTED  ",    FULL_BAR   },  // 25  linger
};
const uint8_t BOOT_STEPS = sizeof(BOOT_FRAMES) / sizeof(BOOT_FRAMES[0]);

// Idle messages — 14 chars each; lcdPrintBookend() adds ▶ ◀ bookends
const char* IDLE_ROW0[] = {
  "PANOPTICORP   ", "ZONE MONITOR  ", "SCAN MODE:AUTO",
  "ACCESS GRANTED", "UPLINK:NOMINAL"
};
const char* IDLE_ROW1[] = {
  "ALL ZONES CLR ", "STATUS:NOMINAL", "7 ZONES ACTIVE",
  "CLEARANCE:LVL3", "AWAITING INPUT"
};

// =============================================================================
// Init (called from setup())
// =============================================================================

void lcdInit() {
  lcd.begin(16, 2);
  lcd.createChar(CC_RARROW,  CC_RARROW_DEF);
  lcd.createChar(CC_LARROW,  CC_LARROW_DEF);
  lcd.createChar(CC_EYE,     CC_EYE_DEF);
  lcd.createChar(CC_SMILEY,  CC_SMILEY_DEF);
  lcd.createChar(CC_ANTENNA, CC_ANTENNA_DEF);
  lcd.createChar(CC_ARROWUP, CC_ARROWUP_DEF);
  lcd.createChar(CC_ARROWDN, CC_ARROWDN_DEF);
  bootStep   = 0;
  lcdState   = LCD_BOOT;
  lcdStateMs = millis();
  showBootStep(0);
}

// =============================================================================
// State machine (called every loop())
// =============================================================================

void updateLcd() {
  unsigned long now = millis();

  if (lcdState == LCD_BOOT && now - lcdStateMs >= BOOT_FRAMES[bootStep].ms) {
    bootStep++;
    lcdStateMs = now;
    if (bootStep >= BOOT_STEPS) {
      lcdState  = LCD_IDLE;
      idleIndex = 0;
      showIdleMessage(0);
    } else {
      showBootStep(bootStep);
    }
    return;
  }

  if (lcdState == LCD_IDLE && now - lcdStateMs >= LCD_IDLE_MS) {
    lcdStateMs = now;
    idleIndex  = (idleIndex + 1) % 5;
    showIdleMessage(idleIndex);
    return;
  }

  if ((lcdState == LCD_SCENE || lcdState == LCD_VOLUME || lcdState == LCD_TRACK) &&
      now - lcdStateMs >= LCD_EVENT_MS) {
    lcdState   = LCD_IDLE;
    lcdStateMs = now;
    showIdleMessage(idleIndex);
  }
}

// =============================================================================
// Display helpers
// =============================================================================

// Print 14-char string padded to 16 on row 0 (no bookends)
void lcdPrintRow0(const char* msg14) {
  lcd.setCursor(0, 0);
  lcd.print(" ");
  lcd.print(msg14);
  lcd.print(" ");
}

// Print | + 14-char string + | on row 1
void lcdPrintBookend(uint8_t row, const char* msg14) {
  lcd.setCursor(0, row);
  lcd.write("|");
  lcd.print(msg14);
  lcd.write("|");
}

void showBootStep(uint8_t step) {
  if (step == 0) {
    // Splash: ⊙ PANOPTICORP ⊙ on row 0, blank row 1
    lcd.setCursor(0, 0);
    lcd.print("PANOPTICORP  ");
    lcd.write((uint8_t)CC_EYE);
    lcd.setCursor(0, 1);
    lcd.print("                ");
    return;
  }
  // Row 0
  lcd.setCursor(0, 0);
  lcd.print(BOOT_FRAMES[step].r0);
  // Row 1: nullptr = dynamic progress bar, otherwise literal string
  if (BOOT_FRAMES[step].r1 == nullptr) {
    lcd.setCursor(0, 1);
    lcd.write('[');
    for (uint8_t i = 0; i < 14; i++) {
      lcd.write(i < step ? (uint8_t)0xFF : (uint8_t)0x20);
    }
    lcd.write(']');
  } else {
    lcd.setCursor(0, 1);
    lcd.print(BOOT_FRAMES[step].r1);
  }
}

void showIdleMessage(uint8_t idx) {
  lcdPrintRow0(IDLE_ROW0[idx]);
  lcdPrintBookend(1, IDLE_ROW1[idx]);
}

void showSceneMessage(uint8_t scene) {
  switch (scene) {
    case 0: lcdPrintRow0(" SCENE: OFF   "); break;
    case 1: lcdPrintRow0(" SCENE: 1     "); break;
    case 2: lcdPrintRow0(" SCENE: 2     "); break;
  }
  lcdPrintBookend(1, "              ");
  lcdState   = LCD_SCENE;
  lcdStateMs = millis();
}

void showTrackMessage(bool forward) {
  lcdPrintRow0(forward ? " SKIP FORWARD " : " SKIP BACK    ");
  lcdPrintBookend(1, forward ? " TRACK +1     " : " TRACK -1     ");
  lcdState   = LCD_TRACK;
  lcdStateMs = millis();
}

void showVolumeMessage(bool up) {
  lcdPrintRow0("    VOLUME    ");
  lcd.setCursor(0, 1);
  lcd.write((uint8_t)CC_RARROW);
  lcd.write((uint8_t)(up ? CC_ARROWUP : CC_ARROWDN));
  lcd.print(up ? "   VOL UP    " : "  VOL DOWN   ");
  lcd.write((uint8_t)CC_LARROW);
  lcdState   = LCD_VOLUME;
  lcdStateMs = millis();
}
