// SensorBar — Seeed XIAO ESP32-S3 Sense
// PDM mic → amplitude envelope / FFT → WS2812B effect in scene color.
// Scene index received via UART from Interior Hub Serial2 TX shared line.
//
// Effects (stretch goal: selectable via serial):
//   0  PULSE_CENTER           — lit zone expands from center outward with amplitude
//   1  FREQ_SPECTRUM          — frequency spectrum, scene color, lows left / highs right
//   2  FREQ_SPECTRUM_GREEN
//   3  FREQ_SPECTRUM_BLUE
//   4  FREQ_SPECTRUM_RAINBOW
//   5  SPECTRUM_PB            — Pixelblaze-style: PI auto-gain, noise floor subtraction, fade trail

#include <FastLED.h>
#include <ESP_I2S.h>
#include <arduinoFFT.h>
#include "scenes.h"

// ---------------------------------------------------------------------------
// Pin constants
// ---------------------------------------------------------------------------
const int PIN_STRIP    =  1;  // D0 — WS2812B data
const int PIN_SCENE_RX = 44;  // D7 — Serial1 RX ← Hub Serial2 TX
const int PIN_SCENE_TX = 43;  // D6 — Serial1 TX (unused; required by begin())
const int PIN_PDM_DATA = 41;  // onboard PDM mic data
const int PIN_PDM_CLK  = 42;  // onboard PDM mic clock

// ---------------------------------------------------------------------------
// FastLED
// ---------------------------------------------------------------------------
const int NUM_LEDS = 150;
CRGB leds[NUM_LEDS];

const CRGB COLOR_BLUE = CRGB(0x00, 0x00, 0xCC);

// ---------------------------------------------------------------------------
// PDM microphone
// ---------------------------------------------------------------------------
I2SClass i2s;

const int MIC_BUF_LEN = 64;
int16_t micBuf[MIC_BUF_LEN];

int16_t readAmplitude() {
  size_t bytesRead = i2s.readBytes((char*)micBuf, sizeof(micBuf));
  if (bytesRead == 0) return 0;
  int samplesRead = bytesRead / sizeof(int16_t);
  int32_t peak = 0;
  for (int i = 0; i < samplesRead; i++) {
    int32_t s = abs((int32_t)micBuf[i]);
    if (s > peak) peak = s;
  }
  return (int16_t)min(peak, (int32_t)32767);
}

// ---------------------------------------------------------------------------
// Amplitude envelope (PULSE_CENTER)
// ---------------------------------------------------------------------------
float envelope = 0.0f;
int16_t lastRaw = 0;

const float ATTACK  = 0.8f;
const float DECAY   = 0.05f;
const int   AMP_MAX = 8000;

void updateEnvelope(int16_t rawPeak) {
  float target = (float)rawPeak / AMP_MAX;
  if (target > 1.0f) target = 1.0f;
  if (target > envelope)
    envelope += ATTACK * (target - envelope);
  else
    envelope -= DECAY;
  if (envelope < 0.0f) envelope = 0.0f;
}

// ---------------------------------------------------------------------------
// FFT — shared infrastructure
// ---------------------------------------------------------------------------
const int   FFT_SIZE    = 256;
const float SAMPLE_RATE = 16000.0f;
const int   FFT_BIN_MIN = 3;  // ≈188 Hz — skips DC and sub-bass noise floor

float   fftReal[FFT_SIZE];
float   fftImag[FFT_SIZE];
int16_t fftAccum[FFT_SIZE];
int     fftAccumIdx = 0;

ArduinoFFT<float> FFT = ArduinoFFT<float>(fftReal, fftImag, FFT_SIZE, SAMPLE_RATE);

// Accumulate samples via the same 64-sample chunks used by readAmplitude().
bool accumulateFftSamples() {
  size_t bytesRead = i2s.readBytes((char*)micBuf, sizeof(micBuf));
  int n = bytesRead / sizeof(int16_t);
  for (int i = 0; i < n && fftAccumIdx < FFT_SIZE; i++)
    fftAccum[fftAccumIdx++] = micBuf[i];
  if (fftAccumIdx >= FFT_SIZE) {
    fftAccumIdx = 0;
    return true;
  }
  return false;
}

// Raw FFT: DC removal → window → compute → magnitudes in fftReal[].
void computeFft() {
  int32_t sum = 0;
  for (int i = 0; i < FFT_SIZE; i++) sum += fftAccum[i];
  int16_t mean = (int16_t)(sum / FFT_SIZE);
  for (int i = 0; i < FFT_SIZE; i++) {
    fftReal[i] = (float)(fftAccum[i] - mean);
    fftImag[i] = 0.0f;
  }
  FFT.windowing(FFTWindow::Hann, FFTDirection::Forward);
  FFT.compute(FFTDirection::Forward);
  FFT.complexToMagnitude();
}

// ---------------------------------------------------------------------------
// FREQ_SPECTRUM band normalization (per-LED peak AGC)
// ---------------------------------------------------------------------------
float spectrumBands[NUM_LEDS];
float bandPeak[NUM_LEDS];

const float SPEC_ATTACK = 0.6f;
const float SPEC_DECAY  = 0.08f;
const float PEAK_DECAY  = 0.001f;

void updateSpectrumBands() {
  const float logMin = log((float)FFT_BIN_MIN);
  const float logMax = log((float)(FFT_SIZE / 2 - 1));
  for (int led = 0; led < NUM_LEDS; led++) {
    float t = (float)led / (NUM_LEDS - 1);
    int bin = (int)exp(logMin + t * (logMax - logMin));
    bin = max(FFT_BIN_MIN, min(bin, FFT_SIZE / 2 - 1));
    float mag = fftReal[bin];
    if (mag > bandPeak[led]) bandPeak[led] = mag;
    bandPeak[led] -= bandPeak[led] * PEAK_DECAY;
    if (bandPeak[led] < 1.0f) bandPeak[led] = 1.0f;
    float normalized = mag / bandPeak[led];
    if (normalized > 1.0f) normalized = 1.0f;
    if (normalized > spectrumBands[led])
      spectrumBands[led] += SPEC_ATTACK * (normalized - spectrumBands[led]);
    else
      spectrumBands[led] -= SPEC_DECAY;
    if (spectrumBands[led] < 0.0f) spectrumBands[led] = 0.0f;
  }
}

// ---------------------------------------------------------------------------
// SPECTRUM_PB — Pixelblaze-style effect
// PI auto-gain, noise-floor subtraction, fade trail, white-on-peak coloring.
// ---------------------------------------------------------------------------
const int PB_BANDS = 32;

// PI controller: keeps ~10% of pixels bright regardless of room volume.
// Sensitivity drives up when strip is too dark, down when too bright.
float pbPiIntegral   = 10.0f;
float pbSensitivity  = 10.0f;
float pbBrightFb     = 0.0f;   // sum of clamped pixel values from last frame

const float PB_KP          = 0.05f;
const float PB_KI          = 0.08f;
const float PB_PI_MIN      = 0.0f;
const float PB_PI_MAX      = 80.0f;
const float PB_TARGET_FILL = 0.10f;
const float PB_AVG_MS      = 1500.0f;
const float PB_FADE        = 0.85f;

float pbFreqData[PB_BANDS];  // current band magnitudes, 0-1 normalized per band
float pbBandPeak[PB_BANDS];  // per-band running peak (same AGC as FREQ_SPECTRUM)
float pbAverages[PB_BANDS];  // per-band noise floor (slow EMA)
float pbPixels[NUM_LEDS];    // pixel memory for fade trail
float pbT1 = 0.0f;           // slow time, cycles ~13s
float pbT2 = 0.0f;           // faster time, cycles ~8.5s

float pbCalcPI(float err) {
  pbPiIntegral = constrain(pbPiIntegral + err, PB_PI_MIN, PB_PI_MAX);
  return PB_KP * err + PB_KI * pbPiIntegral;
}

// Linear interpolation between adjacent band values (arrayLerp from Pixelblaze).
float pbArrayLerp(float *a, float idx) {
  int lo = (int)idx;
  int hi = min(lo + 1, PB_BANDS - 1);
  float r = idx - lo;
  return a[lo] * (1.0f - r) + a[hi] * r;
}

void effectSpectrumPB(uint32_t deltaMs) {
  bool newFft = accumulateFftSamples();
  if (newFft) {
    computeFft();

    // Map FFT bins to PB_BANDS using log scale, normalized per-band against
    // a running peak — gives each band a 0-1 range matching Pixelblaze's
    // pre-normalized frequencyData[], so high-frequency bands aren't drowned out.
    const float logMin = log((float)FFT_BIN_MIN);
    const float logMax = log((float)(FFT_SIZE / 2 - 1));
    for (int b = 0; b < PB_BANDS; b++) {
      float t = (float)b / (PB_BANDS - 1);
      int bin = (int)exp(logMin + t * (logMax - logMin));
      bin = max(FFT_BIN_MIN, min(bin, FFT_SIZE / 2 - 1));
      float mag = fftReal[bin];
      if (mag > pbBandPeak[b]) pbBandPeak[b] = mag;
      pbBandPeak[b] -= pbBandPeak[b] * PEAK_DECAY;
      if (pbBandPeak[b] < 1.0f) pbBandPeak[b] = 1.0f;
      pbFreqData[b] = mag / pbBandPeak[b];
    }

    // Advance time counters
    pbT1 = fmod(pbT1 + deltaMs / 13107.2f, 1.0f);
    pbT2 = fmod(pbT2 + deltaMs /  8519.7f, 1.0f);

    // Update per-band noise floor averages (tracks raw 0-1 signal, no sensitivity baked in)
    float dw = (float)deltaMs / PB_AVG_MS;
    for (int b = 0; b < PB_BANDS; b++) {
      pbAverages[b] = max(0.00001f, pbAverages[b] * (1.0f - dw) + pbFreqData[b] * dw);
    }

    // PI: use brightness feedback from the previous frame
    pbSensitivity = pbCalcPI(PB_TARGET_FILL - pbBrightFb / NUM_LEDS);
    pbBrightFb = 0.0f;
  }

  // Render every loop iteration using latest pbPixels[]
  FastLED.setBrightness(255);
  for (int i = 0; i < NUM_LEDS; i++) {
    float bandIdx = ((float)i / (NUM_LEDS - 1)) * (PB_BANDS - 1);

    float raw = pbArrayLerp(pbFreqData, bandIdx);
    float avg = pbArrayLerp(pbAverages, bandIdx);

    // How far above the noise floor, scaled by sensitivity. Squared for punchiness.
    float v = (raw - avg) * pbSensitivity;
    v = (v > 0.0f) ? v * v : 0.0f;

    pbPixels[i] = pbPixels[i] * PB_FADE + v;
    v = pbPixels[i];

    pbBrightFb += constrain(v, 0.0f, 1.0f);

    // Hue: frequency position + slow time drift
    float h = fmod(bandIdx / 60.0f + pbT1, 1.0f);
    // Saturation drops toward white as pixel brightens — loud peaks flash white
    float s = constrain(1.0f - v, 0.0f, 1.0f);
    float bri = constrain(v, 0.0f, 1.0f);

    leds[i] = CHSV((uint8_t)(h * 255), (uint8_t)(s * 255), (uint8_t)(bri * 255));
  }
  FastLED.show();
}

// ---------------------------------------------------------------------------
// Effects dispatch
// ---------------------------------------------------------------------------
#define EFFECT_PULSE_CENTER        0
#define EFFECT_FREQ_SPECTRUM       1
#define EFFECT_FREQ_SPECTRUM_GREEN 2
#define EFFECT_FREQ_SPECTRUM_BLUE  3
#define EFFECT_FREQ_SPECTRUM_RAINBOW 4
#define EFFECT_SPECTRUM_PB         5

uint8_t currentEffect = EFFECT_SPECTRUM_PB;  // boot default

void effectPulseCenter(CRGB color) {
  lastRaw = readAmplitude();
  updateEnvelope(lastRaw);
  const float SOFT_EDGE = 10.0f;
  float litRadius = envelope * (NUM_LEDS / 2);
  FastLED.setBrightness(255);
  for (int i = 0; i < NUM_LEDS; i++) {
    float d = abs(i - (NUM_LEDS / 2.0f - 0.5f));
    float t = (litRadius - d) / SOFT_EDGE;
    if (t <= 0.0f) { leds[i] = CRGB::Black; continue; }
    if (t >  1.0f) t = 1.0f;
    leds[i] = color;
    leds[i].nscale8((uint8_t)(t * 255));
  }
  FastLED.show();
}

// colorMode: 0=scene, 1=green, 2=blue, 3=rainbow
void effectFreqSpectrum(CRGB sceneColor, uint8_t colorMode) {
  if (accumulateFftSamples()) {
    computeFft();
    updateSpectrumBands();
  }
  FastLED.setBrightness(255);
  for (int i = 0; i < NUM_LEDS; i++) {
    uint8_t bri = (uint8_t)(spectrumBands[i] * 255);
    if (colorMode == 3) {
      uint8_t hue = map(i, 0, NUM_LEDS - 1, 0, 170);
      leds[i] = CHSV(hue, 255, bri);
    } else {
      CRGB c = (colorMode == 1) ? COLOR_GREEN
             : (colorMode == 2) ? COLOR_BLUE
             : sceneColor;
      leds[i] = c;
      leds[i].nscale8(bri);
    }
  }
  FastLED.show();
}

void runEffect(CRGB color, uint32_t deltaMs) {
  switch (currentEffect) {
    case EFFECT_FREQ_SPECTRUM:           effectFreqSpectrum(color, 0); break;
    case EFFECT_FREQ_SPECTRUM_GREEN:     effectFreqSpectrum(color, 1); break;
    case EFFECT_FREQ_SPECTRUM_BLUE:      effectFreqSpectrum(color, 2); break;
    case EFFECT_FREQ_SPECTRUM_RAINBOW:   effectFreqSpectrum(color, 3); break;
    case EFFECT_SPECTRUM_PB:             effectSpectrumPB(deltaMs);    break;
    case EFFECT_PULSE_CENTER:
    default:                             effectPulseCenter(color);     break;
  }
}

// ---------------------------------------------------------------------------
// Serial scene listener
// ---------------------------------------------------------------------------
uint8_t currentScene = SCENE_RED;  // visible on boot without serial input

void parseSceneByte(char c, char *buf, uint8_t &bufIdx) {
  if (c == '\n') {
    buf[bufIdx] = '\0';
    if (bufIdx > 0) {
      int scene = atoi(buf);
      if (scene >= SCENE_OFF && scene <= SCENE_GREEN)
        currentScene = (uint8_t)scene;
    }
    bufIdx = 0;
  } else if (c != '\r' && bufIdx < 7) {
    buf[bufIdx++] = c;
  }
}

void readSerial() {
  static char buf1[8]; static uint8_t idx1 = 0;
  static char buf0[8]; static uint8_t idx0 = 0;
  while (Serial1.available()) parseSceneByte(Serial1.read(), buf1, idx1);
  while (Serial.available())  parseSceneByte(Serial.read(),  buf0, idx0);
}

// ---------------------------------------------------------------------------
// Strip update
// ---------------------------------------------------------------------------
void updateStrip(uint32_t deltaMs) {
  if (currentScene == SCENE_OFF) {
    FastLED.clear(true);
    lastRaw = 0;
    envelope = 0.0f;
    return;
  }
  CRGB color = (currentScene == SCENE_RED) ? COLOR_RED : COLOR_GREEN;
  runEffect(color, deltaMs);
}

// ---------------------------------------------------------------------------
// Setup / loop
// ---------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, PIN_SCENE_RX, PIN_SCENE_TX);

  i2s.setPinsPdmRx(PIN_PDM_CLK, PIN_PDM_DATA);
  i2s.begin(I2S_MODE_PDM_RX, 16000, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO);
  i2s.setTimeout(0);

  FastLED.addLeds<WS2812B, PIN_STRIP, GRB>(leds, NUM_LEDS);
  FastLED.clear(true);

  memset(spectrumBands, 0, sizeof(spectrumBands));
  memset(pbPixels,      0, sizeof(pbPixels));
  memset(pbFreqData,    0, sizeof(pbFreqData));
  for (int i = 0; i < NUM_LEDS;  i++) bandPeak[i]   = 1.0f;
  for (int i = 0; i < PB_BANDS;  i++) pbAverages[i]  = 0.00001f;
  for (int i = 0; i < PB_BANDS;  i++) pbBandPeak[i]  = 1.0f;

  Serial.println("SensorBar init complete");
}

// Cycle through spectrum effects for testing. Set to 0 to disable.
const uint32_t COLOR_CYCLE_MS = 5000;

void loop() {
  static uint32_t lastMs = 0;
  static uint32_t lastCycle = 0;
  uint32_t now = millis();
  uint32_t deltaMs = now - lastMs;
  lastMs = now;

  if (COLOR_CYCLE_MS > 0 && now - lastCycle >= COLOR_CYCLE_MS) {
    lastCycle = now;
    currentEffect++;
    if (currentEffect > EFFECT_SPECTRUM_PB) currentEffect = EFFECT_FREQ_SPECTRUM;
    Serial.printf("→ effect %d\n", currentEffect);
  }

  readSerial();
  updateStrip(deltaMs);

  static uint32_t lastPrint = 0;
  if (now - lastPrint >= 200) {
    lastPrint = now;
    Serial.printf("scene=%d effect=%d sens=%.1f brightFb=%.2f\n",
      currentScene, currentEffect, pbSensitivity, pbBrightFb / NUM_LEDS);
  }
}
