#include "stdinc.h"
#include <ArduinoOTA.h>

void setupOta(void)
{
  ArduinoOTA.setHostname("octahedron");

  ArduinoOTA.onStart([]() {
    waitLeds(); // Drain pending DMA before flash/cache operations.
  });

  ArduinoOTA.begin();
}

void loopOta(void)
{
  ArduinoOTA.handle();
}
