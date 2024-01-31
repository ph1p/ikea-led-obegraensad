#pragma once

#include "screen.h"

void handleMessage(AsyncWebServerRequest *request);
void handleRemove(AsyncWebServerRequest *request);
void handleGetStatus(AsyncWebServerRequest *request);
void handleGetMetadata(AsyncWebServerRequest *request);
void handleSetPlugin(AsyncWebServerRequest *request);
void handleSetBrightness(AsyncWebServerRequest *request);
void handleGetData(AsyncWebServerRequest *request);