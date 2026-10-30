#include "rgb.h"

// Adapted from FastLED's hsv2rgb_rainbow, src/hsv2rgb.cpp.hpp:
// https://github.com/FastLED/FastLED
/*
The MIT License (MIT)

Copyright (c) 2013 FastLED

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

const RGB RGB::White(255, 255, 255);
const RGB RGB::Black(0, 0, 0);

// FastLED's fixed scale8 and nonzero-preserving scale8_video semantics.
static uint8_t scale8(uint8_t value, uint8_t scale)
{
  return (uint16_t(value) * (uint16_t(scale) + 1)) >> 8;
}

static uint8_t scale8Video(uint8_t value, uint8_t scale)
{
  return ((uint16_t(value) * scale) >> 8) + (value && scale);
}

RGB RGB::fromHSV(uint8_t h, uint8_t s, uint8_t v)
{
  uint8_t offset = (h & 31) << 3;
  uint8_t third = scale8(offset, 85);
  uint8_t twoThirds = scale8(offset, 170);
  RGB rgb;
  // Default FastLED rainbow settings: moderate yellow boost, unscaled green.
  switch (h >> 5)
  {
    case 0: rgb = RGB(255 - third, third, 0); break;
    case 1: rgb = RGB(171, 85 + third, 0); break;
    case 2: rgb = RGB(171 - twoThirds, 170 + third, 0); break;
    case 3: rgb = RGB(0, 255 - third, third); break;
    case 4: rgb = RGB(0, 171 - twoThirds, 85 + twoThirds); break;
    case 5: rgb = RGB(third, 0, 255 - third); break;
    case 6: rgb = RGB(85 + third, 0, 171 - third); break;
    default: rgb = RGB(170 + third, 0, 85 - third); break;
  }

  if (s != 255)
  {
    if (s == 0) rgb = White;
    else
    {
      uint8_t desat = 255 - s;
      desat = scale8Video(desat, desat);
      uint8_t scale = 255 - desat;
      rgb.r = scale8(rgb.r, scale) + desat;
      rgb.g = scale8(rgb.g, scale) + desat;
      rgb.b = scale8(rgb.b, scale) + desat;
    }
  }
  if (v != 255)
  {
    v = scale8Video(v, v);
    rgb.r = scale8(rgb.r, v);
    rgb.g = scale8(rgb.g, v);
    rgb.b = scale8(rgb.b, v);
  }
  return rgb;
}
