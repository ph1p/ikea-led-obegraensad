#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#include "webgui.h"

extern AsyncWebServer server;
void initWebServer();
