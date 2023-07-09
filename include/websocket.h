#pragma once

#include "constants.h"

#ifdef ENABLE_SERVER
#include <ESPAsyncWebServer.h>

#include "storage.h"

void onWsEvent(
    AsyncWebSocket *server,
    AsyncWebSocketClient *client,
    AwsEventType type,
    void *arg,
    uint8_t *data,
    size_t len);
void sendInfo();
void sendMinimalInfo();
void initWebsocketServer(AsyncWebServer &server);
void cleanUpClients();

#ifdef WS_MAX_QUEUED_MESSAGES
#undef WS_MAX_QUEUED_MESSAGES
#define WS_MAX_QUEUED_MESSAGES 64
#endif

#endif
