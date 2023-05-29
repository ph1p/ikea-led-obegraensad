#pragma once

#include "constants.h"

#if defined(ENABLE_SERVER) || defined(ENABLE_ALEXA)
#define ESPALEXA_ASYNC
#include <Espalexa.h>
#include <ESPAsyncWebServer.h>

extern Espalexa espalexa;
void AlexaChanged(uint8_t brightness);

#endif