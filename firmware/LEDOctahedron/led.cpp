#include "stdinc.h"

#include <FastLED.h>

#define NUM_STRIPS 4
#define NUM_LEDS 360

CRGB leds[NUM_STRIPS][NUM_LEDS];
unsigned char animation;

void animationStep(void);

void anim00(void);
void anim01(void);

void (*animations[10])(void) =
{
  &anim00,
  &anim01,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

static unsigned long next;
void setupLeds(void)
{
  FastLED.addLeds<WS2812B, 8, RGB>(leds[0], NUM_LEDS);
  FastLED.addLeds<WS2812B, 9, RGB>(leds[1], NUM_LEDS);
  FastLED.addLeds<WS2812B, 10, RGB>(leds[2], NUM_LEDS);
  FastLED.addLeds<WS2812B, 11, RGB>(leds[3], NUM_LEDS);
  next = micros();
  animation = 0;
}

void loopLeds(void)
{
  unsigned long now = micros();
  if(((signed long)(now - next)) > 0)
  {
    animationStep();
    next += 20000;
  }
}

void anim00(void)
{
  static unsigned char led = 0;
  leds[0][led] = CRGB::Black;
  leds[1][led] = CRGB::Black;
  leds[2][led] = CRGB::Black;
  leds[3][led] = CRGB::Black;
  if(++led == 60) led = 0;
  leds[0][led] = CRGB(30, 0, 0);
  leds[1][led] = CRGB(0, 30, 0);
  leds[2][led] = CRGB(0, 0, 30);
  leds[3][led] = CRGB(255, 255, 255);
}

void anim01(void)
{
  static unsigned char led = 0;
  if(++led == 60) led = 0;
  leds[0][led] = CRGB::White;
  leds[1][led] = CRGB::White;
  leds[2][led] = CRGB::White;
  leds[3][led] = CRGB::White;
}

void animationStep(void)
{
  FastLED.setBrightness(255);
  void (*anim_func)(void) = animations[animation];
  if (anim_func != NULL)
  {
    (*anim_func)();
  }
  FastLED.show();
}