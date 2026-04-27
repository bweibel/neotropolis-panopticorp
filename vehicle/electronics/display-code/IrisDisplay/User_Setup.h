// TFT_eSPI user setup — IRIS Display (GC9A01, Hosyond ESP32-S3 N16R8)
// Place this file in the IrisDisplay sketch folder.
//
// GPIOs 26–37 reserved (flash + OPI PSRAM) — all assignments below avoid that range.
// FQBN: esp32:esp32:esp32s3:PSRAM=opi,CDCOnBoot=cdc

#define USER_SETUP_LOADED

#define GC9A01_DRIVER

#define TFT_WIDTH  240
#define TFT_HEIGHT 240

#define TFT_MOSI  11   // Hardware SPI2 MOSI
#define TFT_SCLK  12   // Hardware SPI2 clock
#define TFT_CS    10
#define TFT_DC     9
#define TFT_RST    8

#define SPI_FREQUENCY      40000000
#define SPI_READ_FREQUENCY 20000000

#define LOAD_GLCD    // built-in font
#define LOAD_FONT2   // small font
#define LOAD_FONT4   // medium font
