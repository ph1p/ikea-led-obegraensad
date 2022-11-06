#ifndef OTA_HEADER_H
#define OTA_HEADER_H

#include <ESPAsyncWebServer.h>

void onOTAStart();

void onOTAEnd();
void initOTA(AsyncWebServer &server);

#endif