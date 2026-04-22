// TFT_eSPI user setup — IRIS Display (GC9A01, ESP32-S3 N16R8)
// Place this file in the IrisDisplay sketch folder.
// TFT_eSPI will use it automatically if USER_SETUP_LOADED is defined here.
//
// Update GPIO numbers once Hosyond ESP32-S3 hardware is in hand.
// Avoid strapping pins (0, 45, 46) and flash pins (26–32 on some variants).

#define USER_SETUP_LOADED

#define GC9A01_DRIVER

#define TFT_WIDTH  240
#define TFT_HEIGHT 240

// SPI pin assignments — ESP32-S3 hardware SPI2 defaults
#define TFT_MOSI  11
#define TFT_SCLK  12
#define TFT_CS    10
#define TFT_DC    13
#define TFT_RST   14

// Use HSPI port on ESP32-S3 (prevents known crash on some configurations)
#define USE_HSPI_PORT

#define SPI_FREQUENCY      40000000
#define SPI_READ_FREQUENCY 20000000

#define LOAD_GLCD    // built-in font
#define LOAD_FONT2   // small font
#define LOAD_FONT4   // medium font
