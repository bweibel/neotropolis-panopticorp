// =============================================================================
// IrisDisplay — ESP32-S3 N16R8
// Role: GC9A01 1.28" round TFT (240×240), IRIS surveillance AI character.
//       Receives scene index via UART from Interior Hub.
//
// Phase 2: Static IRIS eye — cream oval sclera, almond slit, white pupil dot.
//          All mood state draw functions stubbed for Phase 3 animation.
//
// Character design: assets/brand/iris/iris-mock.png, iris-moods.png
// See: vehicle/electronics/display-code/iris-display-spec.md
// =============================================================================

#include <TFT_eSPI.h>
#include <math.h>

// =============================================================================
// Pin constants
// =============================================================================

const int PIN_IRIS_RX = 16;  // Serial1 RX ← Interior Hub Serial2 TX

// =============================================================================
// Display
// =============================================================================

const int LCD_W  = 240;
const int LCD_H  = 240;
const int LCD_CX = LCD_W / 2;
const int LCD_CY = LCD_H / 2;

// =============================================================================
// Eye geometry — tune these on hardware against the mockup
// =============================================================================

const int OVAL_RX = 100;  // sclera semi-width  (px)
const int OVAL_RY =  78;  // sclera semi-height (px)

// =============================================================================
// Palette — COLOR_CREAM resolved after tft.init()
// =============================================================================

const uint16_t COLOR_BLACK        = 0x0000;
const uint16_t COLOR_WHITE        = 0xFFFF;
const uint16_t COLOR_GLITCH_GREEN = 0x07E0;
uint16_t       COLOR_CREAM;

// =============================================================================
// TFT
// =============================================================================

TFT_eSPI tft = TFT_eSPI();

// =============================================================================
// Geometry helpers
// =============================================================================

// Filled horizontal almond (lens) shape via two-circle intersection scanline.
// Produces a shape with pointed ends: rx = semi-width, ry = semi-height.
void fillLens(int cx, int cy, int rx, int ry, uint16_t color) {
  if (ry <= 0) return;
  float R = ((float)rx * rx + (float)ry * ry) / (2.0f * ry);
  float d = R - ry;
  for (int dy = -ry; dy <= ry; dy++) {
    float hwt2 = R * R - (dy + d) * (dy + d);
    float hwb2 = R * R - (dy - d) * (dy - d);
    int   hw   = (int)sqrtf(fmaxf(0.0f, fminf(hwt2, hwb2)));
    tft.drawFastHLine(cx - hw, cy + dy, 2 * hw + 1, color);
  }
}

// =============================================================================
// Core eye renderer
// =============================================================================

// Draws the full IRIS eye in a given state.
//
// topLidY     — lower edge of top eyelid, as y offset from LCD_CY.
//               -OVAL_RY = no top eyelid (fully open). 0 = half closed.
// botLidY     — upper edge of bottom eyelid, as y offset from LCD_CY.
//               +OVAL_RY = no bottom eyelid (fully open). 0 = half closed.
// slitRx/Ry   — almond slit dimensions (vary by mood).
// pOffX/Y     — pupil dot offset from center (px).
// pR          — pupil dot radius.
// pColor      — pupil dot color (white; green in glitch).
// scleraColor — oval fill color (cream default; tinted per scene in Phase 4).
void drawIris(int topLidY, int botLidY,
              int slitRx,  int slitRy,
              int pOffX,   int pOffY, int pR, uint16_t pColor,
              uint16_t scleraColor) {
  tft.fillScreen(COLOR_BLACK);

  // Sclera
  tft.fillEllipse(LCD_CX, LCD_CY, OVAL_RX, OVAL_RY, scleraColor);

  // Almond slit cut into sclera
  fillLens(LCD_CX, LCD_CY, slitRx, slitRy, COLOR_BLACK);

  // Top eyelid plane
  if (topLidY > -OVAL_RY) {
    tft.fillRect(LCD_CX - OVAL_RX - 2, LCD_CY - OVAL_RY,
                 (OVAL_RX + 2) * 2, topLidY + OVAL_RY, COLOR_BLACK);
  }

  // Bottom eyelid plane
  if (botLidY < OVAL_RY) {
    tft.fillRect(LCD_CX - OVAL_RX - 2, LCD_CY + botLidY,
                 (OVAL_RX + 2) * 2, OVAL_RY - botLidY, COLOR_BLACK);
  }

  // Pupil dot
  tft.fillCircle(LCD_CX + pOffX, LCD_CY + pOffY, pR, pColor);
}

// =============================================================================
// Mood state draw functions
// Phase 3: these become animation targets — interpolate eyelid/pupil params.
// All geometry values are rough estimates; tune on hardware.
// =============================================================================

void irisNeutral() {
  // Fully open, centered pupil
  drawIris(-OVAL_RY, OVAL_RY, 76, 20, 0, 0, 13, COLOR_WHITE, COLOR_CREAM);
}

void irisSquint() {
  // Eyelids nearly closed — thin slit remains
  drawIris(-6, 6, 76, 20, 0, 0, 10, COLOR_WHITE, COLOR_CREAM);
}

void irisSurprised() {
  // Fully open, wider slit, larger pupil dot
  drawIris(-OVAL_RY, OVAL_RY, 76, 28, 0, 0, 18, COLOR_WHITE, COLOR_CREAM);
}

void irisSuspicious() {
  // Asymmetric — top eyelid heavier, slight pupil offset right/down
  drawIris(-10, 32, 76, 20, 8, 5, 11, COLOR_WHITE, COLOR_CREAM);
}

void irisWideScan() {
  // Fully open, widened slit
  drawIris(-OVAL_RY, OVAL_RY, 88, 24, 0, 0, 15, COLOR_WHITE, COLOR_CREAM);
}

void irisGlitch() {
  // Green pupil — full scanline distortion deferred to Phase 3
  drawIris(-OVAL_RY, OVAL_RY, 76, 20, 0, 0, 16, COLOR_GLITCH_GREEN, COLOR_CREAM);
}

// =============================================================================
// Setup / loop
// =============================================================================

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, PIN_IRIS_RX, -1);

  tft.init();
  tft.setRotation(0);
  COLOR_CREAM = tft.color565(238, 230, 196);

  irisNeutral();

  Serial.println("IRIS display init complete");
}

void loop() {
  // Phase 2: static display.
  // Phase 3: UART scene rx + mood state machine here.
}