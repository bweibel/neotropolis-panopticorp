// Standalone LCD test — WH1602B-TMI-JT on Uno R4 WiFi
// Press BTN_1 (pin 5) to cycle through all display messages.

#include <LiquidCrystal.h>

const int PIN_LCD_RS = 10;
const int PIN_LCD_EN = A0;
const int PIN_LCD_D4 = A1;
const int PIN_LCD_D5 = A2;
const int PIN_LCD_D6 = A3;
const int PIN_LCD_D7 = A4;

const int BTN_1 = 5;

LiquidCrystal lcd(PIN_LCD_RS, PIN_LCD_EN, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);

const char* ROW0[] = {
  "PANOPTICORP RSU ",
  "PANOPTICORP RSU ",
  "MONITORING ZONE ",
  "SCAN MODE: AUTO ",
  "ACCESS GRANTED  ",
  "UPLINK: NOMINAL ",
  "SCENE: PATROL   ",
  "SCENE: SWEEP    ",
  "VOLUME          ",
};
const char* ROW1[] = {
  "UNIT 7 ONLINE   ",
  "ALL ZONES CLEAR ",
  "ACTIVITY: NORMAL",
  "7 ZONES ACTIVE  ",
  "CLEARANCE: LVL3 ",
  "AWAITING INPUT  ",
  "RED ACTIVE      ",
  "GREEN ACTIVE    ",
  "################",
};

const int NUM_SCREENS = 9;
int current = 0;
int lastBtn = HIGH;

void setup() {
  pinMode(BTN_1, INPUT_PULLUP);
  lcd.begin(16, 2);
  showScreen(0);
}

void loop() {
  int btn = digitalRead(BTN_1);
  if (btn == LOW && lastBtn == HIGH) {
    current = (current + 1) % NUM_SCREENS;
    showScreen(current);
    delay(20);  // debounce
  }
  lastBtn = btn;
}

void showScreen(int idx) {
  lcd.setCursor(0, 0); lcd.print(ROW0[idx]);
  lcd.setCursor(0, 1); lcd.print(ROW1[idx]);
}
