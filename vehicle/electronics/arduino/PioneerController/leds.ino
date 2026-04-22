// =============================================================================
// leds — Arduino Uno R4 WiFi 12×8 LED matrix animations
// Role: Boot column scan / auth strobe / skull flash / granted fill;
//       track-change chevron sweep (highest priority, overrides boot anim).
//
// Boot phases (tied to bootStep from lcd.ino):
//   0–14                        : column bounce scan (80ms/step)
//   15–(DENIED_START-1)         : auth strobe 150ms
//   DENIED_START–(OVERRIDE_START-1) : skull flash 150ms — ACCESS DENIED
//   OVERRIDE_START–(STEPS-3)    : override strobe 150ms
//   BOOT_STEPS-2+               : all on — ACCESS GRANTED
//   post-boot                   : matrix clear (once)
//
// Step thresholds must match BOOT_FRAMES in lcd.ino.
// Track sweep: chevron crosses the matrix in ~490ms (35ms × 14 steps).
// =============================================================================

#include "Arduino_LED_Matrix.h"

ArduinoLEDMatrix  ledMatrix;

// Boot animation state
unsigned long     ledAnimMs      = 0;
int8_t            ledScanCol     = 0;
int8_t            ledScanDir     = 1;
bool              ledStrobe      = false;
bool              ledBootDone    = false;

// Track sweep state
bool              ledTrackActive = false;
bool              ledTrackFwd    = true;
int8_t            ledTrackPos    = 0;
unsigned long     ledTrackMs     = 0;

// Boot step thresholds — must match BOOT_FRAMES array in lcd.ino
const uint8_t BOOT_STEP_AUTH_START     = 15;
const uint8_t BOOT_STEP_DENIED_START   = 31;
const uint8_t BOOT_STEP_OVERRIDE_START = 36;

// Skull bitmap — 12×8, 3-pixel eye sockets, 3 two-pixel teeth
static uint8_t SKULL_FRAME[8][12] = {
  {0,0,1,1,1,1,1,1,1,1,0,0},  // dome top
  {0,1,1,1,1,1,1,1,1,1,1,0},  // dome
  {1,1,0,0,1,1,1,1,0,0,1,1},  // eye sockets
  {1,1,0,0,1,1,1,1,0,0,1,1},  // eye sockets
  {0,1,1,1,1,1,1,1,1,1,1,0},  // cheeks
  {0,0,1,1,1,1,1,1,1,1,0,0},  // jaw
  {0,1,1,0,0,1,1,0,0,1,1,0},  // teeth (3 × 2-pixel)
  {0,0,0,0,0,0,0,0,0,0,0,0},  // empty
};

// Chevron arrow: offset of each row's lit pixel from the tip column.
// Rows 3/4 are the tip (offset 0); outer rows trail by up to 3 columns.
static const int8_t   ARROW_OFFSET[8]   = {-3, -2, -1, 0, 0, -1, -2, -3};
const int8_t          ARROW_MAX_OFFSET  = 3;
const unsigned long   LED_TRACK_MS      = 35;

// =============================================================================
// Render helpers
// =============================================================================

static void ledRenderFill(bool on) {
  uint8_t frame[8][12];
  for (uint8_t r = 0; r < 8; r++)
    for (uint8_t c = 0; c < 12; c++)
      frame[r][c] = on ? 1 : 0;
  ledMatrix.renderBitmap(frame, 8, 12);
}

static void ledRenderSkull() {
  ledMatrix.renderBitmap(SKULL_FRAME, 8, 12);
}

static void ledRenderScanCol(int8_t col) {
  uint8_t frame[8][12] = {0};
  for (uint8_t r = 0; r < 8; r++)
    frame[r][col] = 1;
  ledMatrix.renderBitmap(frame, 8, 12);
}

static void ledRenderTrack() {
  uint8_t frame[8][12] = {0};
  for (uint8_t r = 0; r < 8; r++) {
    int8_t col = ledTrackFwd
      ? (ledTrackPos + ARROW_OFFSET[r])   // > chevron: tip at pos, trails left
      : (ledTrackPos - ARROW_OFFSET[r]);  // < chevron: tip at pos, trails right
    if (col >= 0 && col < 12) frame[r][col] = 1;
  }
  ledMatrix.renderBitmap(frame, 8, 12);
}

// =============================================================================
// Public trigger — called from PulseTrackForward / PulseTrackBack
// =============================================================================

void triggerTrackAnimation(bool forward) {
  ledTrackActive = true;
  ledTrackFwd    = forward;
  ledTrackPos    = forward ? 0 : 11;
  ledTrackMs     = 0;
}

// =============================================================================
// Init / update (called from pioneer.ino)
// =============================================================================

void ledMatrixInit() {
  ledMatrix.begin();
  ledScanCol     = 0;
  ledScanDir     = 1;
  ledAnimMs      = 0;
  ledBootDone    = false;
  ledTrackActive = false;
  ledRenderFill(false);
}

void updateLedMatrix() {
  unsigned long now = millis();

  // Track sweep — highest priority, runs regardless of boot state
  if (ledTrackActive) {
    if (now - ledTrackMs >= LED_TRACK_MS) {
      ledTrackMs = now;
      ledRenderTrack();
      ledTrackPos += ledTrackFwd ? 1 : -1;
      bool done = ledTrackFwd
        ? (ledTrackPos - ARROW_MAX_OFFSET >= 12)
        : (ledTrackPos + ARROW_MAX_OFFSET <  0);
      if (done) {
        ledTrackActive = false;
        ledRenderFill(false);
      }
    }
    return;
  }

  // Boot animation
  if (lcdState != LCD_BOOT) {
    if (!ledBootDone) {
      ledBootDone = true;
      ledRenderFill(false);
    }
    return;
  }

  ledBootDone = false;

  // ACCESS GRANTED — all on
  if (bootStep >= BOOT_STEPS - 2) {
    ledRenderFill(true);
    return;
  }

  // ACCESS DENIED — skull flash
  if (bootStep >= BOOT_STEP_DENIED_START && bootStep < BOOT_STEP_OVERRIDE_START) {
    if (now - ledAnimMs >= 150) {
      ledAnimMs = now;
      ledStrobe  = !ledStrobe;
      ledStrobe ? ledRenderSkull() : ledRenderFill(false);
    }
    return;
  }

  // Auth / override drama — full strobe
  if (bootStep >= BOOT_STEP_AUTH_START) {
    if (now - ledAnimMs >= 150) {
      ledAnimMs = now;
      ledStrobe = !ledStrobe;
      ledRenderFill(ledStrobe);
    }
    return;
  }

  // Normal boot — column bounce scan
  if (now - ledAnimMs >= 80) {
    ledAnimMs = now;
    ledRenderScanCol(ledScanCol);
    ledScanCol += ledScanDir;
    if (ledScanCol >= 12) { ledScanCol = 10; ledScanDir = -1; }
    if (ledScanCol < 0)   { ledScanCol = 1;  ledScanDir =  1; }
  }
}
