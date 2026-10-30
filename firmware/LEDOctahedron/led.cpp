#include "stdinc.h"

#include <Arduino.h>
#include <I2SClocklessLedDriver.h>

struct Rgb { uint8_t r, g, b; };
static_assert(sizeof(Rgb) == 3, "Packed RGB pixels required");
static I2SClocklessLedDriver driver;
static bool driverReady = false;

#define NUM_STRIPS 8
#define NUM_LEDS (1440 / NUM_STRIPS)
#define TEST_STRIPS 4
#define TEST_LEDS 60
#define FPS 120u
#define FRAME_US (1000000u / FPS)

static Rgb leds[NUM_STRIPS][NUM_LEDS];
static uint8_t animation;

void animationStep(void);

void anim00(void);
void anim01(void);
void anim02(void);

static void (*animations[])(void) =
{
  anim00,
  anim01,
  anim02
};

static uint32_t last;
void setupLeds(void)
{
  static uint8_t pins[] = {3, 5, 6, 7, 13, 14, 15, 16};
  driver.initled((uint8_t*)leds, pins, NUM_STRIPS, NUM_LEDS, ORDER_RGB);
  driver.setBrightness(255);
  if (!driver.enableDriver || driver.dmaBuffersTransposed == nullptr) {
    #if REPORT_MODE == REPORT_SERIAL
    Serial.println("HPWIT initialization failed; animation disabled, OTA available");
    #endif
    return;
  }
  driverReady = true;
  #if REPORT_MODE == REPORT_SERIAL
  Serial.printf("HPWIT: full-frame DMA NO_WAIT, %d lanes x %d pixels\n", NUM_STRIPS, NUM_LEDS);
  #endif
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
  for (uint8_t strip = 0; strip < TEST_STRIPS; ++strip)
  {
    leds[strip][led] = Rgb{0, 0, 0};
  }
  if(++led == TEST_LEDS) led = 0;
  for (uint8_t strip = 0; strip < TEST_STRIPS; ++strip)
  {
    leds[strip][led] = Rgb{255, 255, 255};
  }
}

void anim01(void)
{
  static uint8_t led = 0;
  if(++led == TEST_LEDS) led = 0;
  for (uint8_t strip = 0; strip < TEST_STRIPS; ++strip)
  {
    leds[strip][led] = Rgb{255, 255, 255};
  }
}

void anim02(void)
{
  static uint8_t nextled = 0;
  uint8_t led = nextled;
  for (uint8_t strip = 0; strip < TEST_STRIPS; ++strip)
  {
    leds[strip][led] = Rgb{0, 0, 0};
  }
  if(++led == TEST_LEDS) led = 0;
  for (uint8_t strip = 0; strip < TEST_STRIPS; ++strip)
  {
    leds[strip][led] = Rgb{255, 0, 0};
  }
  if(++led == TEST_LEDS) led = 0;
  for (uint8_t strip = 0; strip < TEST_STRIPS; ++strip)
  {
    leds[strip][led] = Rgb{0, 255, 0};
  }
  if(++led == TEST_LEDS) led = 0;
  for (uint8_t strip = 0; strip < TEST_STRIPS; ++strip)
  {
    leds[strip][led] = Rgb{0, 0, 255};
  }
  if(++nextled == TEST_LEDS) nextled = 0;
}

void animationStep(void)
{
  #if REPORT_MODE != REPORT_OFF
  uint32_t showStart = micros();
  #endif
  driver.showPixelsFirstTranspose(NO_WAIT);
  #if REPORT_MODE != REPORT_OFF
  uint32_t showEnd = micros();
  #endif
  animations[animation]();
  #if REPORT_MODE != REPORT_OFF
  uint32_t renderEnd = micros();
  reportFrame(showStart, showEnd, renderEnd);
  #endif
}

void waitLeds(void)
{
  uint32_t start = millis();
  while (driver.isDisplaying && millis() - start < 2000) delay(1);
}
