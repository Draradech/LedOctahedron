#include "stdinc.h"

#include <Arduino.h>
#include <I2SClocklessLedDriver.h>

struct Rgb { uint8_t r, g, b; };
static_assert(sizeof(Rgb) == 3, "Packed RGB pixels required");
static I2SClocklessLedDriver driver;
static bool driverReady = false;
#ifndef HPWIT_RENDER_STRESS_TEST
#define HPWIT_RENDER_STRESS_TEST 0
#endif
static uint32_t renderStressUs = 0;

#define NUM_STRIPS 8
#define NUM_LEDS (1440 / NUM_STRIPS)

Rgb leds[NUM_STRIPS][NUM_LEDS];
unsigned char animation;

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

static unsigned long next;
static bool validateFullFrameDma()
{
  if (!driver.enableDriver || driver.dmaBuffersTransposed == nullptr) return false;
  unsigned bytes = 0, eofCount = 0;
  for (int i = 0; i < NUM_LEDS + 2; ++i) {
    auto* desc = driver.dmaBuffersTransposed[i];
    auto* expectedNext = i < NUM_LEDS + 1 ? driver.dmaBuffersTransposed[i + 1] : nullptr;
    if (!desc || !desc->buffer || desc->next != expectedNext ||
        desc->dw0.length != desc->dw0.size ||
        desc->dw0.suc_eof != (i == NUM_LEDS + 1)) return false;
    bytes += desc->dw0.length;
    eofCount += desc->dw0.suc_eof;
    if (i == 0 || i == NUM_LEDS + 1) {
      const auto* data = (const uint8_t*)desc->buffer;
      for (unsigned j = 0; j < desc->dw0.length; ++j) if (data[j] != 0) return false;
    }
  }
  Serial.printf("HPWIT DMA: descriptors=%d bytes=%u EOF=%u\n", NUM_LEDS + 2, bytes, eofCount);
  return eofCount == 1;
}

static bool testFullFrameModes()
{
  // All-black tests: exercise completion/reuse without raising LED power.
  for (int i = 0; i < 3; ++i) {
    driver.showPixelsFirstTranspose(WAIT);
    if (driver.isDisplaying) return false;
  }
  leds[0][0] = Rgb{1, 2, 3};
  driver.showPixelsFirstTranspose(NO_WAIT);
  // Editing the source while DMA is active must not alter its staged pixel.
  uint8_t stagedPixel[3 * 8 * 2 * 3];
  memcpy(stagedPixel, driver.dmaBuffersTransposed[1]->buffer, sizeof(stagedPixel));
  leds[0][0] = Rgb{4, 5, 6};
  if (!driver.isDisplaying || memcmp(stagedPixel, driver.dmaBuffersTransposed[1]->buffer, sizeof(stagedPixel))) return false;
  driver.showPixelsFirstTranspose(NO_WAIT); // Deliberately wait on the previous frame.
  memset(leds, 0, sizeof(leds));
  waitLeds();
  if (driver.isDisplaying) return false;
  driver.showPixelsFromBuffer(LOOP);
  delay(25);
  if (!driver.isDisplaying) return false;
  const auto syncSemaphore = driver.semSync;
  driver.waitSync();
  driver.waitSync();
  if (driver.semSync != syncSemaphore) return false;
  driver.stopDisplayLoop();
  waitLeds();
  if (driver.isDisplaying) return false;
  driver.showPixelsFirstTranspose(WAIT); // Return from LOOP to single-shot mode.
  return !driver.isDisplaying;
}

void setupLeds(void)
{
  static uint8_t pins[] = {8, 9, 10, 11, 13, 14, 15, 16};
  driver.initled((uint8_t*)leds, pins, NUM_STRIPS, NUM_LEDS, ORDER_RGB);
  driver.setBrightness(255);
  if (!validateFullFrameDma() || !testFullFrameModes()) {
    Serial.println("HPWIT full-frame self-test FAILED; animation disabled, OTA available");
    return;
  }
  driverReady = true;
  Serial.printf("HPWIT: full-frame DMA NO_WAIT, %d lanes x %d pixels; self-test PASS\n", NUM_STRIPS, NUM_LEDS);
  next = micros();
  animation = 2;
}

void loopLeds(void)
{
  if (!driverReady) return;
  unsigned long now = micros();
  if(((signed long)(now - next)) > 0)
  {
#if HPWIT_RENDER_STRESS_TEST
    static const uint32_t testStart = millis();
    const uint32_t elapsed = millis() - testStart;
    renderStressUs = elapsed >= 5000 && elapsed < 25000 ? 5000 : 0;
#endif
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

void anim02(void)
{
  static unsigned char nextled = 0;
  unsigned char led = nextled;
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

void animationStep(void)
{
  const uint32_t renderStart = micros();
  const bool sourceOverlap = driver.isDisplaying;
  void (*anim_func)(void) = animations[animation];
  if (anim_func != NULL)
  {
    (*anim_func)();
  }
  while (micros() - renderStart < renderStressUs) {} // Finite CPU-load test only.
  const uint32_t renderUs = micros() - renderStart;
  const unsigned long showStart = micros();
  driver.showPixelsFirstTranspose(NO_WAIT);
  const unsigned long showUs = micros() - showStart;
  static unsigned long frames = 0, totalFrames = 0, maxShowUs = 0, lastReport = 0;
  static uint32_t maxRenderUs = 0, overlapFrames = 0;
  if (renderUs > maxRenderUs) maxRenderUs = renderUs;
  if (sourceOverlap) ++overlapFrames;
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
      "LED: total=%lu frames=%lu interval_ms=%lu max_show_us=%lu max_frame_us=%lu heap=%u render_us=%u overlap=%u stress_us=%u\n",
      totalFrames, frames, reportNow - lastReport, maxShowUs, maxFrameUs, ESP.getFreeHeap(), maxRenderUs, overlapFrames, renderStressUs);
    // Never stall animation when USB is disconnected or the reader falls behind.
    if (Serial && len > 0 && len < (int)sizeof(report) && Serial.availableForWrite() >= len)
      Serial.write((const uint8_t*)report, len);
    frames = maxShowUs = maxFrameUs = 0;
    maxRenderUs = overlapFrames = 0;
    lastReport = reportNow;
  }
}

void waitLeds(void)
{
  const uint32_t start = millis();
  while (driver.isDisplaying && millis() - start < 2000) delay(1);
}
