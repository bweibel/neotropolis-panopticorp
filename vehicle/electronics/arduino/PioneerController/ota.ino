// =============================================================================
// ota — WiFi connection + OTA update support
// Role: Connect to WiFi on boot; accept sketch uploads over the network.
//       Gracefully skips OTA if WiFi is unavailable within timeout.
// Credentials: arduino_secrets.h (gitignored — do not commit)
// Upload: Sketch → Upload Using Programmer → select network port in Arduino IDE
// =============================================================================

#include <WiFiS3.h>
#include <ArduinoOTA.h>
#include "arduino_secrets.h"

static bool otaReady = false;

void otaInit() {
  Serial.print("WiFi connecting");
  WiFi.begin(SECRET_SSID, SECRET_PASS);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 5000) {
    delay(250);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi unavailable — OTA disabled");
    return;
  }

  Serial.print("WiFi connected: ");
  Serial.println(WiFi.localIP());

  ArduinoOTA.begin(WiFi.localIP(), "PioneerController", SECRET_OTA_PASS, InternalStorage);
  otaReady = true;
  Serial.println("OTA ready");
}

void otaUpdate() {
  if (otaReady) ArduinoOTA.poll();
}
