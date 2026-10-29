#include "stdinc.h"

#include <Arduino.h>
#include <I2SClocklessLedDriver.h>

struct Rgb { uint8_t r, g, b; };
static_assert(sizeof(Rgb) == 3, "Packed RGB pixels required");
static I2SClocklessLedDriver driver;

#define NUM_STRIPS 8
#define NUM_LEDS (1440 / NUM_STRIPS)

Rgb leds[NUM_STRIPS][NUM_LEDS];
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
  static uint8_t pins[] = {8, 9, 10, 11, 13, 14, 15, 16};
  driver.initled((uint8_t*)leds, pins, NUM_STRIPS, NUM_LEDS, ORDER_GRB);
  driver.setBrightness(255);
  Serial.printf("HPWIT: default streaming DMA, %d lanes x %d pixels\n", NUM_STRIPS, NUM_LEDS);
  next = micros();
  animation = 0;
}

void loopLeds(void)
{
  unsigned long now = micros();
  if(((signed long)(now - next)) > 0)
  {
    animationStep();
    // 120 Hz: 8333us. Recover small overruns without clock drift.
    next += 8333;
    // Drop accumulated deadlines only after falling a whole frame behind.
    if ((signed long)(micros() - next) >= 8333) next = micros();
  }
}

void anim00(void)
{
  static unsigned char led = 0;
  leds[0][led] = Rgb{0, 0, 0};
  leds[1][led] = Rgb{0, 0, 0};
  leds[2][led] = Rgb{0, 0, 0};
  leds[3][led] = Rgb{0, 0, 0};
  if(++led == 60) led = 0;
  leds[0][led] = Rgb{255, 255, 255};
  leds[1][led] = Rgb{255, 255, 255};
  leds[2][led] = Rgb{255, 255, 255};
  leds[3][led] = Rgb{255, 255, 255};
}

void anim01(void)
{
  static unsigned char led = 0;
  if(++led == 60) led = 0;
  leds[0][led] = Rgb{255, 255, 255};
  leds[1][led] = Rgb{255, 255, 255};
  leds[2][led] = Rgb{255, 255, 255};
  leds[3][led] = Rgb{255, 255, 255};
}

void animationStep(void)
{
  void (*anim_func)(void) = animations[animation];
  if (anim_func != NULL)
  {
    (*anim_func)();
  }
  const unsigned long showStart = micros();
  driver.showPixels(WAIT);
  const unsigned long showUs = micros() - showStart;
  static unsigned long frames = 0, totalFrames = 0, maxShowUs = 0, lastReport = 0;
  static unsigned long lastFrameUs = 0, maxFrameUs = 0;
  const unsigned long frameNow = micros();
  if (lastFrameUs && frameNow - lastFrameUs > maxFrameUs)
    maxFrameUs = frameNow - lastFrameUs;
  lastFrameUs = frameNow;
  ++frames;
  ++totalFrames;
  if (showUs > maxShowUs) maxShowUs = showUs;
  const unsigned long reportNow = millis();
  if (reportNow - lastReport >= 1000)
  {
    char report[300];
    const int len = snprintf(report, sizeof(report),
      "LED: total=%lu frames=%lu interval_ms=%lu max_show_us=%lu max_frame_us=%lu heap=%u\n",
      totalFrames, frames, reportNow - lastReport, maxShowUs, maxFrameUs, ESP.getFreeHeap());
    // Never stall animation when USB is disconnected or the reader falls behind.
    if (Serial && len > 0 && len < (int)sizeof(report) && Serial.availableForWrite() >= len)
      Serial.write((const uint8_t*)report, len);
    frames = maxShowUs = maxFrameUs = 0;
    lastReport = reportNow;
  }
}

void waitLeds(void)
{
  const uint32_t start = millis();
  while (driver.isDisplaying && millis() - start < 2000) delay(1);
}
