#ifndef OTA_HEADER_H
#define OTA_HEADER_H

#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

#include "led.h"
#include "mode/mode.h"
#include "secrets.h"
#include "signs.h"

void onOTAStart();

void onOTAEnd();
void initOTA(AsyncWebServer &server);

#endif