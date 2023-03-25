
#pragma once

#include "constants.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>

#ifdef ENABLE_SERVER
#include "screen.h"
#include "mode/mode.h"

extern unsigned long lastUpdate;

extern WiFiClient client;
extern HTTPClient http;

void weatherSetup();
void weatherLoop();
void weatherUpdate();

#endif