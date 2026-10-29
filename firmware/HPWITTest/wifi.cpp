#include "stdinc.h"
#include <WiFi.h>

const char* ssid = "octahedron";
const char* passwd = "octahedronpw";

void setupWifi(void)
{
  WiFi.persistent(false);
  WiFi.useStaticBuffers(true);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, passwd);
}
