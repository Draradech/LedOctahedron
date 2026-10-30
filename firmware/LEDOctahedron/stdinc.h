#pragma once

#include <stdint.h>

#define REPORT_OFF 0
#define REPORT_SERIAL 1
#define REPORT_UDP 2
#ifndef REPORT_MODE
#define REPORT_MODE REPORT_UDP
#endif

#define REPORT_UDP_ADDRESS "192.168.4.255"
#define REPORT_UDP_PORT 7777

#if REPORT_MODE != REPORT_OFF && REPORT_MODE != REPORT_SERIAL && REPORT_MODE != REPORT_UDP
#error "Invalid REPORT_MODE"
#endif

void setupOta(void);
void setupWifi(void);
void setupLeds(void);

void loopOta(void);
void loopLeds(void);

void waitLeds(void);

#if REPORT_MODE != REPORT_OFF
void setupReports(void);
void reportFrame(uint32_t showStart, uint32_t showEnd, uint32_t renderEnd);
#endif
