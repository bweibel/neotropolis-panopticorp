#pragma once
// NOTE: This file is duplicated in interior-hub/ and exterior-node/.
// If you change glitter parameters or colors, update BOTH copies.

// =============================================================================
// scenes.h — shared scene constants (interior-hub)
// =============================================================================

#include <FastLED.h>

// Scene indices
#define SCENE_OFF   0
#define SCENE_RED   1
#define SCENE_GREEN 2

// Base colors (tune to taste during testing)
const CRGB COLOR_RED   = CRGB(0xCC, 0x00, 0x00);
const CRGB COLOR_GREEN = CRGB(0x00, 0xCC, 0x00);

// Glitter animation parameters
const float GLITTER_DENSITY  = 0.05f;  // fraction of pixels selected each frame
const uint8_t BASE_BRIGHTNESS  = 217;  // 85% of 255
const uint8_t GLITTER_MIN      = 153;  // 60% of 255
const uint8_t GLITTER_MAX      = 255;  // 100% of 255
