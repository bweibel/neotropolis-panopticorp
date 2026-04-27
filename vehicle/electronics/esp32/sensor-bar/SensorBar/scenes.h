#pragma once
// NOTE: This file is duplicated in interior-hub/, exterior-node/, and sensor-bar/.
// If you change scene indices or colors, update ALL copies.

// =============================================================================
// scenes.h — shared scene constants (sensor-bar)
// =============================================================================

#include <FastLED.h>

// Scene indices
#define SCENE_OFF   0
#define SCENE_RED   1
#define SCENE_GREEN 2

// Base colors (tune to taste during testing)
const CRGB COLOR_RED   = CRGB(0xCC, 0x00, 0x00);
const CRGB COLOR_GREEN = CRGB(0x00, 0xCC, 0x00);
