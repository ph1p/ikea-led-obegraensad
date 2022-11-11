#pragma once

#include "constants.h"

#ifdef ENABLE_SERVER
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

#include "storage.h"
#include "screen.h"
#include "mode/mode.h"

void sendStateAndInfo();
void sendModeToAllClients();
void initWebsocketServer(AsyncWebServer &server);
void cleanUpClients();

#ifdef WS_MAX_QUEUED_MESSAGES
#undef WS_MAX_QUEUED_MESSAGES
#define WS_MAX_QUEUED_MESSAGES 64
#endif

#endif
