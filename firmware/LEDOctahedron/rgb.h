#pragma once

#include <stdint.h>

struct RGB
{
  uint8_t r, g, b;

  static const RGB White;
  static const RGB Black;

  // FastLED rainbow HSV: all inputs 0..255; hue wraps around the color wheel.
  static RGB fromHSV(uint8_t h, uint8_t s, uint8_t v);
};

static_assert(sizeof(RGB) == 3, "Packed RGB pixels required");
