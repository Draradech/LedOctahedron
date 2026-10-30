#include "stdinc.h"
#include <esp_system.h>

void setup(void) 
{
  #if REPORT_MODE == REPORT_SERIAL
  Serial.begin(115200);
  #if ARDUINO_USB_CDC_ON_BOOT
  Serial.setTxTimeoutMs(0);
  #endif
  Serial.printf("\nOctahedron boot: reset reason=%d\n", esp_reset_reason());
  #endif

  setupWifi();
  #if REPORT_MODE != REPORT_OFF
  setupReports();
  #endif
  setupOta();
  setupLeds();
}

void loop(void)
{
  loopOta();
  loopLeds();
}
