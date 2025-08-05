#pragma once

#include "constants.h"

#ifdef ENABLE_SERVER

#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>

#include "PluginManager.h"
#include "screen.h"
#include "secrets.h"
#include "signs.h"

void onOTAStart();
void onOTAProgress(size_t current, size_t final);
void onOTAEnd(bool success);
void initOTA(AsyncWebServer &server);

#endif
