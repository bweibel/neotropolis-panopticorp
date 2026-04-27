#include <Arduino_GFX_Library.h>

Arduino_ESP32SPI bus(9, 10, 12, 11, GFX_NOT_DEFINED);  // DC, CS, SCK, MOSI, MISO
Arduino_GC9A01 gfx(&bus, 8, 0, true);                   // RST=8, rotation=0, IPS=true

void setup() {
  Serial.begin(115200);
  Serial.println("start");

  if (!gfx.begin()) {
    Serial.println("init failed");
    return;
  }

  Serial.println("init ok");
  gfx.fillScreen(RED);
  Serial.println("fill done");
}

void loop() {}
