// =============================================================================
// IrisDisplay — ESP32-S3 N16R8
// Role: Drives GC9A01 1.28" round TFT (240×240) with IRIS surveillance AI
//       character. Receives scene index via wired UART from Interior Hub.
//
// Phase 1: Static test pattern — confirms wiring and SPI before animation work.
// Library: TFT_eSPI (Bodmer). Configure via User_Setup.h in this sketch folder.
//
// See: vehicle/electronics/display-code/iris-display-spec.md
// =============================================================================

#include <TFT_eSPI.h>

// =============================================================================
// Pin constants — update with actual GPIOs once hardware is in hand
// Avoid strapping pins (0, 45, 46) and flash pins (26–32 on some variants)
// =============================================================================

const int PIN_IRIS_RX = /* TBD */ -1;  // UART RX from Interior Hub Serial2 TX

// =============================================================================
// Display dimensions
// =============================================================================

const int LCD_W  = 240;
const int LCD_H  = 240;
const int LCD_CX = LCD_W / 2;
const int LCD_CY = LCD_H / 2;

// =============================================================================
// IRIS palette (RGB565)
// =============================================================================

const uint16_t COLOR_BLACK     = 0x0000;
const uint16_t COLOR_WHITE     = 0xFFFF;
const uint16_t COLOR_IRIS_CYAN = 0x07FF;

// =============================================================================
// TFT instance — pins configured via User_Setup.h
// =============================================================================

TFT_eSPI tft = TFT_eSPI();

// =============================================================================
// setup / loop
// =============================================================================

void setup() {
  Serial.begin(115200);

  // Serial1: UART RX from Interior Hub (scene index broadcasts)
  // PIN_IRIS_RX will be assigned once hardware is in hand
  // Serial1.begin(9600, SERIAL_8N1, PIN_IRIS_RX, -1);

  tft.init();
  tft.setRotation(0);
  tft.fillScreen(COLOR_BLACK);
  drawTestPattern();

  Serial.println("IRIS display init complete");
}

void loop() {
  // Phase 1: nothing — static display
}

// =============================================================================
// Test pattern (Phase 1)
// =============================================================================

void drawTestPattern() {
  // Lens — cyan fill nearly filling the round display
  tft.fillCircle(LCD_CX, LCD_CY, 110, COLOR_IRIS_CYAN);

  // Aperture ring — dark outline at edge (stand-in for blade geometry)
  tft.drawCircle(LCD_CX, LCD_CY, 110, COLOR_BLACK);
  tft.drawCircle(LCD_CX, LCD_CY, 109, COLOR_BLACK);

  // Pupil — black center
  tft.fillCircle(LCD_CX, LCD_CY, 30, COLOR_BLACK);

  // Label
  tft.setTextColor(COLOR_WHITE, COLOR_IRIS_CYAN);
  tft.setTextSize(2);
  tft.setCursor(LCD_CX - 20, LCD_CY + 50);
  tft.print("IRIS");
}
