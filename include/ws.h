#ifndef WS_HEADER_H
#define WS_HEADER_H

#include <ESPAsyncWebServer.h>

void sendStateAndInfo(AsyncWebSocketClient *client);
void sendModeToAllClients();
void initWebsocketServer(AsyncWebServer &server);
void cleanUpClients();

#endif