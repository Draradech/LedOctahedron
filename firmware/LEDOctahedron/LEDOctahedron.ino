#include "stdinc.h"

void setup(void) 
{
  setupWifi();
  setupOta();
  setupLeds();
}

void loop(void)
{
  loopOta();
  loopLeds();
}
