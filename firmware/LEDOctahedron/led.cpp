#include "stdinc.h"

#include <FastLED.h>
#include "fl/channels/channel_typed.h"
#include "platforms/esp/32/drivers/lcd_spi/bus_traits.h"
#include "platforms/esp/32/drivers/lcd_spi/lcd_spi_peripheral_esp.h"

#define NUM_STRIPS 8
#define NUM_LEDS (1440 / NUM_STRIPS)

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
  // On ESP32-S3, FLEX_IO instance 0 maps clockless LEDs to LCD_CLOCKLESS.
  static const int pins[] = {8, 9, 10, 11, 13, 14, 15, 16};
  // WS2815B: T0H=250ns, T0L=1000ns, T1H=650ns, T1L=600ns.
  // Avoid the library's slower TIMING_WS2815 preset (RESET=0).
  const fl::ChipsetTimingConfig timing(250, 400, 600, 300);
  for (int i = 0; i < NUM_STRIPS; ++i)
  {
    fl::ChannelConfigOf<fl::ClocklessChipset> cfg{
      fl::ClocklessChipset(pins[i], timing),
      fl::span<CRGB>(leds[i], NUM_LEDS), GRB};
    auto channel = fl::TypedChannel<
      fl::Bus::FLEX_IO, fl::ClocklessChipset, 0>::create(cfg);
    FastLED.add(channel);
    Serial.printf("LED channel %d: pin=%d created=%d bus=%s\n",
      i, pins[i], !!channel,
      fl::busDriverName(fl::Bus::FLEX_IO, 0, false));
  }

  // Encode each complete lane before starting one DMA transaction.
  auto lcdDriver = fl::BusTraits<fl::Bus::FLEX_IO, 0>::instancePtr();
  static_cast<fl::ChannelDriverLcdClockless*>(lcdDriver.get())
    ->setChunkInputBytesForTest(NUM_LEDS * 3);
  FastLED.setWaitSpinBudgetUs(6000);
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
  leds[0][led] = CRGB::Black;
  leds[1][led] = CRGB::Black;
  leds[2][led] = CRGB::Black;
  leds[3][led] = CRGB::Black;
  if(++led == 60) led = 0;
  leds[0][led] = CRGB::White;
  leds[1][led] = CRGB::White;
  leds[2][led] = CRGB::White;
  leds[3][led] = CRGB::White;
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
  const unsigned long showStart = micros();
  FastLED.show();
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
