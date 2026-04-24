// TFT_eSPI user setup — IRIS Display (GC9A01)
// Place this file in the IrisDisplay sketch folder.
// TFT_eSPI will use it automatically when USER_SETUP_LOADED is defined here.
//
// Two supported targets — uncomment exactly one BOARD block:
//
//   ESP32-S3-N16R8 devkit  — for wiring/hardware testing
//   XIAO ESP32-S3 Sense    — final installation target
//
// N16R8 note: GPIOs 26–37 are reserved (flash + OPI PSRAM). Avoid them.
// XIAO note:  Camera connector uses GPIOs 10–18. These assignments avoid
//             that range. Confirm no conflict if OV2640 is attached.

#define USER_SETUP_LOADED

#define GC9A01_DRIVER

#define TFT_WIDTH  240
#define TFT_HEIGHT 240

// --- ESP32-S3-N16R8 devkit (hardware SPI2, safe GPIOs) ---
#define TFT_MOSI  11
#define TFT_SCLK  12
#define TFT_CS    10
#define TFT_DC     9
#define TFT_RST    8

// --- XIAO ESP32-S3 Sense (uncomment when moving to final hardware) ---
// #define TFT_MOSI   9   // D10 — hardware SPI2 MOSI
// #define TFT_SCLK   7   // D8  — hardware SPI2 SCK
// #define TFT_CS     2   // D1
// #define TFT_DC     3   // D2
// #define TFT_RST    4   // D3

#define SPI_FREQUENCY      10000000  // reduced for wiring test — raise to 40000000 once confirmed
#define SPI_READ_FREQUENCY  5000000

#define LOAD_GLCD    // built-in font
#define LOAD_FONT2   // small font
#define LOAD_FONT4   // medium font
