#include "stdinc.h"
#include <esp_system.h>

void setup(void) 
{
  Serial.begin(115200);
#if ARDUINO_USB_CDC_ON_BOOT
  Serial.setTxTimeoutMs(0);
#endif
  Serial.printf("\nOctahedron boot: reset reason=%d\n", (int)esp_reset_reason());
  setupWifi();
  setupOta();
  setupLeds();
}

void loop(void)
{
  loopOta();
  loopLeds();
}
