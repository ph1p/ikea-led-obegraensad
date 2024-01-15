#pragma once

#include "constants.h"

#ifdef ENABLE_SERVER

#include <ESPAsyncWebServer.h>
void startGui(AsyncWebServerRequest *request);
#endif
