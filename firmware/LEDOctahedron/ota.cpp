#include "stdinc.h"
#include <ArduinoOTA.h>
#include <FastLED.h>

static bool done = false;
static int16_t rebootDelay = 500;

void setupOta(void)
{
  ArduinoOTA.setHostname("octahedron");

  ArduinoOTA.onStart([]() {
    FastLED.wait(2000); // Drain pending DMA before flash/cache operations.
  });

  ArduinoOTA.onEnd([]() {
    done = true;
  });

  ArduinoOTA.begin();
}

void loopOta(void)
{
  ArduinoOTA.handle();
  if(done)
  {
    if(--rebootDelay == 0)
    {
      ESP.restart();
    }
  }
}
