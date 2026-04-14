#include "stdinc.h"
#include <ArduinoOTA.h>

static bool done = false;
static int16_t rebootDelay = 500;

void setupOta(void)
{
  ArduinoOTA.setHostname("octahedron");

  ArduinoOTA.onStart([]() {
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
