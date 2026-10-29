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
  Serial.println("HPWIT test: 15-second OTA recovery window before LED initialization");
  const uint32_t recoveryStart = millis();
  while (millis() - recoveryStart < 15000) { loopOta(); delay(1); }
  setupLeds();
}

void loop(void)
{
  loopOta();
  loopLeds();
}
