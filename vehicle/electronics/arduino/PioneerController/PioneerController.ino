// =============================================================================
// PioneerController — Arduino Uno R4 WiFi
// Role: Reads physical inputs, controls Pioneer head unit via X9C104 digital
//       pot, and broadcasts serial events to ESP32 Interior Hub.
//
// See: vehicle/electronics/baja-lighting-spec.md
// =============================================================================

#include "X9C.h"
#include "pt.h"
#include <LiquidCrystal.h>
#include "Arduino_LED_Matrix.h"
#include <Adafruit_NeoPixel.h>

// =============================================================================
// Pin constants
// =============================================================================

const int ENC_CLK    = 2;   // Rotary encoder CLK
const int ENC_DT     = 3;   // Rotary encoder DT
const int ENC_SW     = 4;   // Rotary encoder button (mute)
const int BTN_1      = 5;   // Skip back (SCAN)
const int BTN_2      = 6;   // Skip forward (SWEEP)
const int BTN_3      = 7;   // Scene cycle forward (PURGE)
const int BTN_4      = 8;   // Scene cycle backward (UPLINK)
const int TOGGLE_LED = 9;   // T2: LED master power state

const int PIN_SERIAL_TX = 1;  // Hardware Serial1 TX — Uno R4 pin 1

// Character LCD (WH1602B-TMI-JT, 16×2, HD44780 parallel 4-bit)
const int PIN_LCD_RS = 10;
const int PIN_LCD_EN = A0;
const int PIN_LCD_D4 = A1;
const int PIN_LCD_D5 = A2;
const int PIN_LCD_D6 = A3;
const int PIN_LCD_D7 = A4;  // A4 is hardware I2C SDA — acceptable, no I2C devices used

// Digital pot (X9C104) pins
const int PIN_POT_CS  = 13;
const int PIN_POT_UD  = 12;
const int PIN_POT_INC = 11;

const int PIN_KNOB_LED = A5;  // WS2811 above volume knob

// =============================================================================
// Event string constants
// =============================================================================

const char* EVT_VOL_UP     = "VOL_UP\n";
const char* EVT_VOL_DOWN   = "VOL_DOWN\n";
const char* EVT_MUTE       = "MUTE\n";
const char* EVT_SKIP_FWD   = "SKIP_FWD\n";
const char* EVT_SKIP_BACK  = "SKIP_BACK\n";
const char* EVT_SCENE_NEXT = "SCENE_NEXT\n";
const char* EVT_SCENE_PREV = "SCENE_PREV\n";

// =============================================================================
// State
// =============================================================================

bool ledMasterOn = false;
bool isMuted     = false;
