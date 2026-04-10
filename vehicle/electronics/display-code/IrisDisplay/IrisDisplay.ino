#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// Pin constants — software SPI (hardware SPI pins occupied by X9C104 pot)
const int PIN_LCD_SCK  = A1;
const int PIN_LCD_MOSI = A2;
const int PIN_LCD_CS   = A3;
const int PIN_LCD_DC   = A4;
const int PIN_LCD_RST  = A5;

// Display dimensions
const int LCD_W = 240;
const int LCD_H = 320;

// Accent color — cyan (RGB565)
const uint16_t COLOR_ACCENT = 0x07FF;

Adafruit_ST7789 tft = Adafruit_ST7789(PIN_LCD_CS, PIN_LCD_DC, PIN_LCD_MOSI, PIN_LCD_SCK, PIN_LCD_RST);

void setup() {
  Serial.begin(115200);
  Serial.println("Init starting...");
  tft.init(LCD_W, LCD_H);
  tft.setRotation(0);  // adjust if display orientation is wrong
  drawTestPattern();
  Serial.println("Display init complete");
}

void loop() {
  // nothing — static display
}

void drawTestPattern() {
  tft.fillScreen(ST77XX_BLACK);

  // Lens stand-in: accent circle centered
  tft.fillCircle(LCD_W / 2, LCD_H / 2, 80, COLOR_ACCENT);

  // Label — centered at top (4 chars × 12px wide at size 2 = 48px; x = (240-48)/2 = 96)
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.setCursor(96, 20);
  tft.print("IRIS");

  // Ticker zone bar
  tft.fillRect(0, LCD_H - 40, LCD_W, 40, COLOR_ACCENT);
}
