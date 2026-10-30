#include "stdinc.h"

#if REPORT_MODE != REPORT_OFF
#include <Arduino.h>

#if REPORT_MODE == REPORT_UDP
#include <lwip/sockets.h>

static int32_t reportSocket = -1;
static sockaddr_in reportDestination = {};
#endif

void setupReports(void)
{
  #if REPORT_MODE == REPORT_UDP
  reportSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (reportSocket < 0) return;
  int32_t broadcast = 1;
  if (setsockopt(reportSocket, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0)
  {
    close(reportSocket);
    reportSocket = -1;
    return;
  }
  reportDestination.sin_family = AF_INET;
  reportDestination.sin_port = htons(REPORT_UDP_PORT);
  reportDestination.sin_addr.s_addr = inet_addr(REPORT_UDP_ADDRESS);
  #endif
}

void reportFrame(uint32_t showStart, uint32_t showEnd, uint32_t renderEnd)
{
  static uint32_t frames = 0, lastReport = 0;
  static uint32_t maxRenderUs = 0, maxShowUs = 0;
  static uint32_t lastFrameUs = 0, maxFrameUs = 0;
  uint32_t renderUs = renderEnd - showEnd;
  uint32_t showUs = showEnd - showStart;
  if (renderUs > maxRenderUs) maxRenderUs = renderUs;
  if (showUs > maxShowUs) maxShowUs = showUs;
  if (lastFrameUs && showEnd - lastFrameUs > maxFrameUs)
  {
    maxFrameUs = showEnd - lastFrameUs;
  }
  lastFrameUs = showEnd;
  ++frames;
  uint32_t reportNow = millis();
  if (reportNow - lastReport >= 1000)
  {
    char report[300];
    int32_t len =
      snprintf(report, sizeof(report),
      "LED: frames=%lu interval_ms=%lu max_show_us=%lu max_frame_us=%lu heap=%lu render_us=%lu\n",
      frames, reportNow - lastReport, maxShowUs, maxFrameUs, ESP.getFreeHeap(), maxRenderUs);

    #if REPORT_MODE == REPORT_SERIAL
    // Don't stall animation when USB is disconnected or the reader falls behind.
    if (Serial && len > 0 && len < sizeof(report) && Serial.availableForWrite() >= len)
    {
      Serial.write(report, len);
    }
    #elif REPORT_MODE == REPORT_UDP
    if (reportSocket >= 0 && len > 0 && len < sizeof(report))
    {
      // Best effort: no retries or error logging in the frame loop.
      sendto(reportSocket, report, len, MSG_DONTWAIT,
             (sockaddr*)&reportDestination, sizeof(reportDestination));
    }
    #endif
    frames = maxRenderUs = maxShowUs = maxFrameUs = 0;
    lastReport = reportNow;
  }
}

#endif
