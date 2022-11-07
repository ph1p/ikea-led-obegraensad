#ifndef WEBSERVER_HEADER_H
#define WEBSERVER_HEADER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#include "webgui.h"

extern AsyncWebServer server;
void initWebServer();

#endif