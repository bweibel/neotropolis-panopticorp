// =============================================================================
// IrisDisplay — Seeed XIAO ESP32-S3 Sense
// Role: GC9A01 1.28" round TFT (240×240), IRIS surveillance AI character.
//
// Rendering: PIXEL=4 → 60×60 logical grid on 240×240 display.
// Framebuffer: Arduino_Canvas (PSRAM) — all drawing to RAM, one flush per frame.
// Effects: time-fading trails + 6-band rainbow chromatic aberration.
// Animation: two-layer float — sclera body + leading iris/pupil secondary motion.
//
// Library: GFX Library for Arduino (Arduino_GFX, moononournation)
// =============================================================================

#include <Arduino_GFX_Library.h>
#include <math.h>

// =============================================================================
// Pin constants
// =============================================================================

const int PIN_IRIS_RX   = 17;  // GPIO 44 on XIAO
const int PIN_CYCLE_BTN = 0;

Arduino_ESP32SPI bus(9, 10, 12, 11, GFX_NOT_DEFINED);
Arduino_GC9A01   display(&bus, 8, 0, true);
Arduino_Canvas   canvas(240, 240, &display);
// XIAO: Arduino_ESP32SPI bus(3, 2, 7, 9, GFX_NOT_DEFINED);
// XIAO: Arduino_GC9A01   display(&bus, 4, 0, true);

// =============================================================================
// Pixel grid
// =============================================================================

const int PIXEL = 4;
const int PCX   = (240 / PIXEL) / 2;  // 30
const int PCY   = (240 / PIXEL) / 2;  // 30

// =============================================================================
// Eye geometry
// =============================================================================

const int OVAL_RX = 26;
const int OVAL_RY = 17;

// =============================================================================
// Palette
// =============================================================================

const uint16_t COLOR_BLACK        = BLACK;
const uint16_t COLOR_GLITCH_GREEN = GREEN;
uint16_t COLOR_CREAM, COLOR_PUPIL;
uint16_t scleraGrad[5];

// 6-band rainbow chromatic aberration
const int CHROMA_BANDS = 6;
uint16_t chromaColors[CHROMA_BANDS];

// =============================================================================
// Draw offsets
// =============================================================================

int gOffX = 0,  gOffY = 0;
int gIrisX = 0, gIrisY = 0;

// =============================================================================
// Pixel-grid draw primitives
// =============================================================================

void pFillEllipse(int cx, int cy, int rx, int ry, uint16_t color) {
  for (int dy = -ry; dy <= ry; dy++) {
    int hw = (int)(rx * sqrtf(fmaxf(0.0f, 1.0f - (float)dy*dy / ((float)ry*ry))));
    if (hw == 0) continue;
    canvas.fillRect((cx - hw)*PIXEL, (cy + dy)*PIXEL, (2*hw+1)*PIXEL, PIXEL, color);
  }
}

void pFillEllipseGrad(int cx, int cy, int rx, int ry, uint16_t *grad) {
  for (int dy = -ry; dy <= ry; dy++) {
    int hw = (int)(rx * sqrtf(fmaxf(0.0f, 1.0f - (float)dy*dy / ((float)ry*ry))));
    if (hw == 0) continue;
    for (int dx = -hw; dx <= hw; dx++) {
      float nx = (float)dx / rx, ny = (float)dy / ry;
      int band = (int)(sqrtf(nx*nx + ny*ny) * 5);
      if (band > 4) band = 4;
      canvas.fillRect((cx + dx)*PIXEL, (cy + dy)*PIXEL, PIXEL, PIXEL, grad[band]);
    }
  }
}

// Gradient ellipse with per-pixel alpha fade (0.0 – 1.0)
void pFillEllipseGradAlpha(int cx, int cy, int rx, int ry, uint16_t *grad, float alpha) {
  int ia = (int)(alpha * 256);
  for (int dy = -ry; dy <= ry; dy++) {
    int hw = (int)(rx * sqrtf(fmaxf(0.0f, 1.0f - (float)dy*dy / ((float)ry*ry))));
    if (hw == 0) continue;
    for (int dx = -hw; dx <= hw; dx++) {
      float nx = (float)dx / rx, ny = (float)dy / ry;
      int band = (int)(sqrtf(nx*nx + ny*ny) * 5);
      if (band > 4) band = 4;
      uint16_t c = grad[band];
      int r = ((((c >> 11) & 0x1F) * ia) >> 8);
      int g = ((((c >>  5) & 0x3F) * ia) >> 8);
      int b = (((c         & 0x1F) * ia) >> 8);
      canvas.fillRect((cx + dx)*PIXEL, (cy + dy)*PIXEL, PIXEL, PIXEL,
                      (uint16_t)((r << 11) | (g << 5) | b));
    }
  }
}

void pFillCircle(int cx, int cy, int r, uint16_t color) {
  for (int dy = -r; dy <= r; dy++) {
    int hw = (int)sqrtf(fmaxf(0.0f, (float)(r*r - dy*dy)));
    if (hw == 0) continue;
    canvas.fillRect((cx - hw)*PIXEL, (cy + dy)*PIXEL, (2*hw+1)*PIXEL, PIXEL, color);
  }
}

void pFillRect(int x, int y, int w, int h, uint16_t color) {
  canvas.fillRect(x*PIXEL, y*PIXEL, w*PIXEL, h*PIXEL, color);
}

void pDrawTopLid(int cx, int cy, int topLidY, uint16_t color) {
  for (int dy = -OVAL_RY; dy <= topLidY; dy++) {
    int hw = (int)(OVAL_RX * sqrtf(fmaxf(0.0f, 1.0f - (float)dy*dy / ((float)OVAL_RY*OVAL_RY))));
    if (hw == 0) continue;
    canvas.fillRect((cx - hw)*PIXEL, (cy + dy)*PIXEL, (2*hw+1)*PIXEL, PIXEL, color);
  }
}

void pDrawBotLid(int cx, int cy, int botLidY, uint16_t color) {
  for (int dy = botLidY; dy <= OVAL_RY; dy++) {
    int hw = (int)(OVAL_RX * sqrtf(fmaxf(0.0f, 1.0f - (float)dy*dy / ((float)OVAL_RY*OVAL_RY))));
    if (hw == 0) continue;
    canvas.fillRect((cx - hw)*PIXEL, (cy + dy)*PIXEL, (2*hw+1)*PIXEL, PIXEL, color);
  }
}

// =============================================================================
// Effects — time-fading trails + rainbow chromatic aberration
// =============================================================================

const int   TRAIL_LEN      = 3;
const float TRAIL_START_ALPHA = 0.65f;
const unsigned long TRAIL_FADE_MS = 800;

int           trailX[TRAIL_LEN]    = {0, 0, 0};
int           trailY[TRAIL_LEN]    = {0, 0, 0};
unsigned long trailTime[TRAIL_LEN] = {0, 0, 0};
float velX = 0.0f, velY = 0.0f;

bool anyTrailFading() {
  unsigned long now = millis();
  for (int i = 0; i < TRAIL_LEN; i++)
    if (now - trailTime[i] < TRAIL_FADE_MS) return true;
  return false;
}

void drawEffectLayers() {
  unsigned long now = millis();

  // Trails — oldest first, each fades from TRAIL_START_ALPHA to 0 over TRAIL_FADE_MS
  for (int i = TRAIL_LEN - 1; i >= 0; i--) {
    unsigned long age = now - trailTime[i];
    if (age >= TRAIL_FADE_MS) continue;
    float alpha = TRAIL_START_ALPHA * (1.0f - (float)age / TRAIL_FADE_MS);
    pFillEllipseGradAlpha(PCX + trailX[i], PCY + trailY[i], OVAL_RX, OVAL_RY, scleraGrad, alpha);
  }

  // Rainbow chromatic aberration — 6 bands at fixed ±1/±2/±3 px along velocity
  float speed = sqrtf(velX*velX + velY*velY);
  if (speed > 0.008f) {
    float ux = velX / speed, uy = velY / speed;
    const int offsets[CHROMA_BANDS] = { -3, -2, -1, 1, 2, 3 };
    for (int i = 0; i < CHROMA_BANDS; i++) {
      int ox = (int)roundf(ux * offsets[i]);
      int oy = (int)roundf(uy * offsets[i]);
      pFillEllipse(PCX + gOffX + ox, PCY + gOffY + oy, OVAL_RX, OVAL_RY, chromaColors[i]);
    }
  }
}

// =============================================================================
// Frame lifecycle
// =============================================================================

void beginFrame() {
  canvas.fillScreen(COLOR_BLACK);
  drawEffectLayers();
}

void endFrame() {
  canvas.flush();
}

void drawEye(int cx, int cy, int topLidY, int botLidY,
             int slitRx, int slitRy, int pOffX, int pOffY, int pR, uint16_t pColor) {
  int icx = cx + gIrisX, icy = cy + gIrisY;
  pFillEllipseGrad(cx, cy, OVAL_RX, OVAL_RY, scleraGrad);
  pFillEllipse(icx, icy, slitRx, slitRy, COLOR_BLACK);
  if (topLidY > -OVAL_RY) pDrawTopLid(cx, cy, topLidY, COLOR_BLACK);
  if (botLidY <  OVAL_RY) pDrawBotLid(cx, cy, botLidY, COLOR_BLACK);
  pFillCircle(icx + pOffX, icy + pOffY, pR, pColor);
}

// =============================================================================
// Mood state draw functions
// =============================================================================

void irisNeutral() {
  int cx = PCX + gOffX, cy = PCY + gOffY;
  beginFrame();
  drawEye(cx, cy, -OVAL_RY, OVAL_RY, 16, 7, 0, 0, 3, COLOR_PUPIL);
  endFrame();
}

void irisSquint() {
  int cx = PCX + gOffX, cy = PCY + gOffY;
  beginFrame();
  drawEye(cx, cy, -7, 7, 17, 5, 0, 0, 2, COLOR_PUPIL);
  endFrame();
}

void irisSurprised() {
  int cx = PCX + gOffX, cy = PCY + gOffY;
  beginFrame();
  drawEye(cx, cy, -OVAL_RY, OVAL_RY, 18, 10, 0, 0, 5, COLOR_PUPIL);
  endFrame();
}

void irisSuspicious() {
  int cx = PCX + gOffX, cy = PCY + gOffY;
  beginFrame();
  drawEye(cx, cy, -12, 12, 16, 6, 3, 2, 3, COLOR_PUPIL);
  endFrame();
}

void irisWideScan() {
  int cx = PCX + gOffX, cy = PCY + gOffY;
  beginFrame();
  drawEye(cx, cy, -OVAL_RY, OVAL_RY, 22, 4, 0, 0, 3, COLOR_PUPIL);
  endFrame();
}

void irisCute() {
  int cx = PCX + gOffX, cy = PCY + gOffY;
  int icx = cx + gIrisX, icy = cy + gIrisY;
  beginFrame();
  drawEye(cx, cy, -OVAL_RY, OVAL_RY, 16, 14, 0, -3, 8, COLOR_PUPIL);
  pFillCircle(icx + 6, icy - 10, 2, WHITE);
  pFillRect(icx - 4, icy - 11, 1, 1, WHITE);
  endFrame();
}


// =============================================================================
// Mood table
// =============================================================================

void (*moods[])() = { irisNeutral, irisSquint, irisSurprised, irisSuspicious, irisCute };
const char *moodNames[] = { "NEUTRAL", "SQUINT", "SURPRISED", "SUSPICIOUS", "CUTE" };
const int MOOD_COUNT = 5;
int currentMood = 0;

// =============================================================================
// Float animation
// =============================================================================

float floatX = 0.0f, floatY = 0.0f;
float targetX = 0.0f, targetY = 0.0f;
int   lastDrawX = 0, lastDrawY = 0;
unsigned long pauseUntil = 0;

const float BODY_EASE = 0.022f;
const float FLOAT_RX  = 4.0f;
const float FLOAT_RY  = 3.0f;

float irisFloatX = 0.0f, irisFloatY = 0.0f;
int   lastIrisX = 0, lastIrisY = 0;

const float IRIS_EASE = 0.07f;
const float IRIS_MAX  = 2.0f;

int trailPushCount = 0;

void pickTarget() {
  targetX = (random(201) - 100) / 100.0f * FLOAT_RX;
  targetY = (random(201) - 100) / 100.0f * FLOAT_RY;
}

void updateFloat() {
  // Trail fading must continue even while position is paused at a target
  if (millis() < pauseUntil) {
    if (anyTrailFading()) moods[currentMood]();
    return;
  }

  float prevX = floatX, prevY = floatY;
  floatX += (targetX - floatX) * BODY_EASE;
  floatY += (targetY - floatY) * BODY_EASE;
  velX = floatX - prevX;
  velY = floatY - prevY;

  float dx = targetX - floatX, dy = targetY - floatY;
  if (dx*dx + dy*dy < 0.01f) {
    pauseUntil = millis() + 800 + random(1500);
    pickTarget();
  }

  float itx = fmaxf(-IRIS_MAX, fminf(IRIS_MAX, +velX * 35.0f));
  float ity = fmaxf(-IRIS_MAX, fminf(IRIS_MAX, +velY * 35.0f));
  irisFloatX += (itx - irisFloatX) * IRIS_EASE;
  irisFloatY += (ity - irisFloatY) * IRIS_EASE;

  int ix  = (int)roundf(floatX);
  int iy  = (int)roundf(floatY);
  int iix = (int)roundf(irisFloatX);
  int iiy = (int)roundf(irisFloatY);

  bool bodyMoved = (ix != lastDrawX || iy != lastDrawY);
  bool irisMoved = (iix != lastIrisX || iiy != lastIrisY);

  if (bodyMoved) {
    trailPushCount++;
    if (trailPushCount >= 2) {
      trailPushCount = 0;
      trailX[2] = trailX[1]; trailY[2] = trailY[1]; trailTime[2] = trailTime[1];
      trailX[1] = trailX[0]; trailY[1] = trailY[0]; trailTime[1] = trailTime[0];
      trailX[0] = lastDrawX; trailY[0] = lastDrawY; trailTime[0] = millis();
    }
    lastDrawX = ix; lastDrawY = iy;
    gOffX = ix; gOffY = iy;
  }

  if (irisMoved) {
    lastIrisX = iix; lastIrisY = iiy;
    gIrisX = iix; gIrisY = iiy;
  }

  if (bodyMoved || irisMoved || anyTrailFading()) {
    moods[currentMood]();
  }
}

// =============================================================================
// Setup / loop
// =============================================================================

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, PIN_IRIS_RX, -1);
  pinMode(PIN_CYCLE_BTN, INPUT_PULLUP);

  canvas.begin();
  COLOR_CREAM = canvas.color565(238, 230, 196);
  COLOR_PUPIL = COLOR_CREAM;

  scleraGrad[0] = canvas.color565(252, 246, 218);
  scleraGrad[1] = canvas.color565(242, 235, 202);
  scleraGrad[2] = canvas.color565(228, 220, 186);
  scleraGrad[3] = canvas.color565(205, 197, 163);
  scleraGrad[4] = canvas.color565(178, 170, 136);

  chromaColors[0] = canvas.color565(160,  80, 220);  // violet  (trailing far)
  chromaColors[1] = canvas.color565( 80, 120, 255);  // blue
  chromaColors[2] = canvas.color565( 60, 210, 210);  // cyan
  chromaColors[3] = canvas.color565(220, 230,  80);  // yellow
  chromaColors[4] = canvas.color565(255, 150,  50);  // orange
  chromaColors[5] = canvas.color565(240,  70,  90);  // red     (leading far)

  // Init trail timestamps to far past so they don't draw on boot
  unsigned long t = millis();
  for (int i = 0; i < TRAIL_LEN; i++) trailTime[i] = t - TRAIL_FADE_MS;

  randomSeed(esp_random());
  pickTarget();

  irisNeutral();
  Serial.println("IRIS display init complete");
}

void loop() {
  if (digitalRead(PIN_CYCLE_BTN) == LOW) {
    currentMood = (currentMood + 1) % MOOD_COUNT;
    unsigned long t = millis();
    for (int i = 0; i < TRAIL_LEN; i++) { trailX[i] = gOffX; trailY[i] = gOffY; trailTime[i] = t - TRAIL_FADE_MS; }
    velX = velY = 0;
    irisFloatX = irisFloatY = 0;
    gIrisX = gIrisY = 0;
    trailPushCount = 0;
    moods[currentMood]();
    lastDrawX = gOffX; lastDrawY = gOffY;
    lastIrisX = lastIrisY = 0;
    while (digitalRead(PIN_CYCLE_BTN) == LOW) delay(10);
    delay(50);
  }

  updateFloat();
}
