// =============================================================================
// Trail fade test — single floating circle, time-based opacity trails
// Experiment branch: isolate fade logic before applying to full eye
// =============================================================================

#include <Arduino_GFX_Library.h>
#include <math.h>

const int PIN_CYCLE_BTN = 0;

Arduino_ESP32SPI bus(9, 10, 12, 11, GFX_NOT_DEFINED);
Arduino_GC9A01   display(&bus, 8, 0, true);
Arduino_Canvas   canvas(240, 240, &display);

// =============================================================================
// Grid — main circle snaps to this; trails and chroma bypass it
// =============================================================================

const int PIXEL = 4;
const int PCX   = (240 / PIXEL) / 2;  // 30
const int PCY   = (240 / PIXEL) / 2;  // 30

// Native pixel center of a logical grid coordinate
inline int toScreenX(int lx) { return lx * PIXEL + PIXEL / 2; }
inline int toScreenY(int ly) { return ly * PIXEL + PIXEL / 2; }

// =============================================================================
// Circle + chroma palette
// =============================================================================

const int   CIRC_R    = 8;               // logical radius
const int   CIRC_R_PX = CIRC_R * PIXEL;  // native radius for smooth circles

uint16_t    COLOR_CIRCLE;

const int CHROMA_BANDS = 2;
uint16_t  chromaColors[CHROMA_BANDS];  // cyan (trailing), red (leading)

// =============================================================================
// Trail — stored in display pixels so they can sit anywhere
// =============================================================================

const int           TRAIL_LEN      = 6;
const float         TRAIL_ALPHA    = 0.85f;
const unsigned long TRAIL_FADE_MS  = 1200;

struct TrailSlot { int px, py; unsigned long t; };
TrailSlot trail[TRAIL_LEN];

bool anyFading() {
  unsigned long now = millis();
  for (int i = 0; i < TRAIL_LEN; i++)
    if (now - trail[i].t < TRAIL_FADE_MS) return true;
  return false;
}

// =============================================================================
// Draw
// =============================================================================

// Pixelated circle — main shape only, stays on logical grid
void drawCirclePixel(int cx, int cy, int r, uint16_t color) {
  for (int dy = -r; dy <= r; dy++) {
    int hw = (int)sqrtf(fmaxf(0.0f, (float)(r*r - dy*dy)));
    if (hw == 0) continue;
    canvas.fillRect((cx - hw)*PIXEL, (cy + dy)*PIXEL, (2*hw+1)*PIXEL, PIXEL, color);
  }
}

// Trail circle: native position but 2px block scanlines — subpixel placement, chunky edge
void drawCircleTrail(int px, int py, int r, uint16_t color, float alpha) {
  const int BP = 2;
  int ia = (int)(alpha * 256);
  int rr = ((((color >> 11) & 0x1F) * ia) >> 8);
  int gg = ((((color >>  5) & 0x3F) * ia) >> 8);
  int bb = (((color         & 0x1F) * ia) >> 8);
  uint16_t c = (uint16_t)((rr << 11) | (gg << 5) | bb);
  for (int dy = -r; dy < r; dy += BP) {
    int hw = (int)sqrtf(fmaxf(0.0f, (float)(r*r - dy*dy)));
    hw = (hw / BP) * BP;  // snap width to block grid
    if (hw == 0) continue;
    canvas.fillRect(px - hw, py + dy, hw * 2, BP, c);
  }
}

void drawCircleNative(int px, int py, int r, uint16_t color, float alpha) {
  int ia = (int)(alpha * 256);
  int rr = ((((color >> 11) & 0x1F) * ia) >> 8);
  int gg = ((((color >>  5) & 0x3F) * ia) >> 8);
  int bb = (((color         & 0x1F) * ia) >> 8);
  canvas.fillCircle(px, py, r, (uint16_t)((rr << 11) | (gg << 5) | bb));
}

void drawCircleChroma(int px, int py, int r, uint16_t color, float alpha) {
  drawCircleNative(px, py, r, color, alpha);
}

const int   CHROMA_PX_OFFSETS[CHROMA_BANDS] = { -6, 5 };
const float CHROMA_ALPHAS[CHROMA_BANDS]     = { 0.85f, 0.85f };

void drawChroma(int px, int py, float nvx, float nvy) {
  for (int i = 0; i < CHROMA_BANDS; i++) {
    int off = CHROMA_PX_OFFSETS[i];
    int bx = px + (int)roundf(nvx * off);
    int by = py + (int)roundf(nvy * off);
    drawCircleChroma(bx, by, CIRC_R_PX, chromaColors[i], CHROMA_ALPHAS[i]);
  }
}

void drawFrame(int cx, int cy, float nvx, float nvy, bool moving) {
  canvas.fillScreen(BLACK);

  // Trails: smooth circles at native pixel positions, fading over time
  unsigned long now = millis();
  for (int i = TRAIL_LEN - 1; i >= 0; i--) {
    unsigned long age = now - trail[i].t;
    if (age >= TRAIL_FADE_MS) continue;
    float alpha = TRAIL_ALPHA * (1.0f - (float)age / TRAIL_FADE_MS);
    drawCircleTrail(trail[i].px, trail[i].py, CIRC_R_PX, COLOR_CIRCLE, alpha);
  }

  // Chroma fringe around current position
  if (moving) {
    int px = toScreenX(cx), py = toScreenY(cy);
    drawChroma(px, py, nvx, nvy);
  }

  // Main circle: pixelated, snapped to grid
  drawCirclePixel(cx, cy, CIRC_R, COLOR_CIRCLE);
  canvas.flush();
}

// =============================================================================
// Float animation
// =============================================================================

float floatX = 0.0f, floatY = 0.0f;
float targetX = 0.0f, targetY = 0.0f;
float prevFX  = 0.0f, prevFY  = 0.0f;
float velX    = 0.0f, velY    = 0.0f;
int   lastX = 0, lastY = 0;
unsigned long pauseUntil = 0;

const float EASE     = 0.32f;
const float RANGE_X  = 8.0f;
const float RANGE_Y  = 6.0f;

void pickTarget() {
  targetX = (random(201) - 100) / 100.0f * RANGE_X;
  targetY = (random(201) - 100) / 100.0f * RANGE_Y;
}

void updateFloat() {
  int cx = PCX + lastX, cy = PCY + lastY;

  if (millis() < pauseUntil) {
    velX = 0.0f; velY = 0.0f;
    if (anyFading()) drawFrame(cx, cy, 0, 0, false);
    return;
  }

  prevFX = floatX; prevFY = floatY;
  floatX += (targetX - floatX) * EASE;
  floatY += (targetY - floatY) * EASE;

  velX = floatX - prevFX;
  velY = floatY - prevFY;

  float dx = targetX - floatX, dy = targetY - floatY;
  if (dx*dx + dy*dy < 0.01f) {
    pauseUntil = millis() + 500 + random(1800);
    pickTarget();
  }

  int ix = (int)roundf(floatX);
  int iy = (int)roundf(floatY);

  if (ix != lastX || iy != lastY) {
    // Trail stored at native pixel position of the departing logical cell
    for (int i = TRAIL_LEN - 1; i > 0; i--) trail[i] = trail[i-1];
    trail[0] = { toScreenX(PCX + lastX), toScreenY(PCY + lastY), millis() };

    lastX = ix; lastY = iy;
    cx = PCX + ix; cy = PCY + iy;
  }

  float speed = sqrtf(velX*velX + velY*velY);
  bool moving = speed > 0.05f;
  float nvx = 0, nvy = 0;
  if (moving) { nvx = velX / speed; nvy = velY / speed; }

  if (moving || anyFading()) {
    drawFrame(cx, cy, nvx, nvy, moving);
  }
}

// =============================================================================
// Setup / loop
// =============================================================================

void setup() {
  Serial.begin(115200);
  pinMode(PIN_CYCLE_BTN, INPUT_PULLUP);

  canvas.begin();
  COLOR_CIRCLE = canvas.color565(238, 230, 196);

  // Chroma: pure cyan behind, pure red ahead
  chromaColors[0] = canvas.color565(  0, 255, 255);  // pure cyan (trailing)
  chromaColors[1] = canvas.color565(255,   0,  40);  // pure red  (leading)

  // All trail slots start fully expired
  unsigned long t = millis() - TRAIL_FADE_MS;
  for (int i = 0; i < TRAIL_LEN; i++)
    trail[i] = { toScreenX(PCX), toScreenY(PCY), t };

  randomSeed(esp_random());
  pickTarget();

  drawFrame(PCX, PCY, 0, 0, false);
  Serial.println("trail test ready");
}

void loop() {
  updateFloat();
}
