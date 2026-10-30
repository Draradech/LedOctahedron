#include "stdinc.h"

#include <Arduino.h>
#include <I2SClocklessLedDriver.h>

struct Rgb { uint8_t r, g, b; };
static_assert(sizeof(Rgb) == 3, "Packed RGB pixels required");
static I2SClocklessLedDriver driver;
static bool driverReady = false;

#define NUM_STRIPS 8
#define NUM_LEDS (1440 / NUM_STRIPS)
#define FPS 120u
#define FRAME_US (1000000u / FPS)

Rgb leds[NUM_STRIPS][NUM_LEDS];
uint8_t animation;

void animationStep(void);

void anim00(void);
void anim01(void);
void anim02(void);

void (*animations[10])(void) =
{
  &anim00,
  &anim01,
  &anim02,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

static uint32_t last;
void setupLeds(void)
{
  static uint8_t pins[] = {8, 9, 10, 11, 13, 14, 15, 16};
  driver.initled((uint8_t*)leds, pins, NUM_STRIPS, NUM_LEDS, ORDER_RGB);
  driver.setBrightness(255);
  if (!driver.enableDriver || driver.dmaBuffersTransposed == nullptr) {
    Serial.println("HPWIT initialization failed; animation disabled, OTA available");
    return;
  }
  driverReady = true;
  Serial.printf("HPWIT: full-frame DMA NO_WAIT, %d lanes x %d pixels\n", NUM_STRIPS, NUM_LEDS);
  last = micros();
  animation = 2;
}

void loopLeds(void)
{
  if (!driverReady) return;
  uint32_t now = micros();
  if (now - last > FRAME_US)
  {
    animationStep();
    last += FRAME_US;
    // drop accumulated deadlines after falling a whole frame behind
    if (now - last >= FRAME_US) last = now;
  }
}

void anim00(void)
{
  static uint8_t led = 0;
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
  static uint8_t led = 0;
  if(++led == 60) led = 0;
  leds[0][led] = Rgb{255, 255, 255};
  leds[1][led] = Rgb{255, 255, 255};
  leds[2][led] = Rgb{255, 255, 255};
  leds[3][led] = Rgb{255, 255, 255};
}

void anim02(void)
{
  static uint8_t nextled = 0;
  uint8_t led = nextled;
  leds[0][led] = Rgb{0, 0, 0};
  leds[1][led] = Rgb{0, 0, 0};
  leds[2][led] = Rgb{0, 0, 0};
  leds[3][led] = Rgb{0, 0, 0};
  if(++led == 60) led = 0;
  leds[0][led] = Rgb{255, 0, 0};
  leds[1][led] = Rgb{255, 0, 0};
  leds[2][led] = Rgb{255, 0, 0};
  leds[3][led] = Rgb{255, 0, 0};
  if(++led == 60) led = 0;
  leds[0][led] = Rgb{0, 255, 0};
  leds[1][led] = Rgb{0, 255, 0};
  leds[2][led] = Rgb{0, 255, 0};
  leds[3][led] = Rgb{0, 255, 0};
  if(++led == 60) led = 0;
  leds[0][led] = Rgb{0, 0, 255};
  leds[1][led] = Rgb{0, 0, 255};
  leds[2][led] = Rgb{0, 0, 255};
  leds[3][led] = Rgb{0, 0, 255};
  if(++nextled == 60) nextled = 0;
}

static void reportFrame(uint32_t showStart, uint32_t showEnd, uint32_t renderEnd)
{
  static uint32_t frames = 0, totalFrames = 0, lastReport = 0;
  static uint32_t maxRenderUs = 0, maxShowUs = 0;
  static uint32_t lastFrameUs = 0, maxFrameUs = 0;
  uint32_t renderUs = renderEnd - showEnd;
  uint32_t showUs = showEnd - showStart;
  if (renderUs > maxRenderUs) maxRenderUs = renderUs;
  if (showUs > maxShowUs) maxShowUs = showUs;
  if (lastFrameUs && showEnd - lastFrameUs > maxFrameUs)
  {
    maxFrameUs = showEnd - lastFrameUs;
  }
  lastFrameUs = showEnd;
  ++frames;
  ++totalFrames;
  uint32_t reportNow = millis();
  if (reportNow - lastReport >= 1000)
  {
    char report[300];
    int32_t len =
      snprintf(report, sizeof(report),
      "LED: total=%lu frames=%lu interval_ms=%lu max_show_us=%lu max_frame_us=%lu heap=%lu render_us=%lu\n",
      totalFrames, frames, reportNow - lastReport, maxShowUs, maxFrameUs, ESP.getFreeHeap(), maxRenderUs);

    // Don't stall animation when USB is disconnected or the reader falls behind.
    if (Serial && len > 0 && len < (int32_t)sizeof(report) && Serial.availableForWrite() >= len)
    {
      Serial.write(report, len);
    }
    frames = maxRenderUs = maxShowUs = maxFrameUs = 0;
    lastReport = reportNow;
  }
}

void animationStep(void)
{
  uint32_t showStart = micros();
  driver.showPixelsFirstTranspose(NO_WAIT);
  uint32_t showEnd = micros();
  void (*anim_func)(void) = animations[animation];
  if (anim_func != NULL)
  {
    (*anim_func)();
  }
  uint32_t renderEnd = micros();
  reportFrame(showStart, showEnd, renderEnd);
}

void waitLeds(void)
{
  uint32_t start = millis();
  while (driver.isDisplaying && millis() - start < 2000) delay(1);
}
