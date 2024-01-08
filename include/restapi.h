#pragma once

#include "constants.h"
#include <ESPAsyncWebServer.h>

class RestApi
{
    private:
        static const char* jsonContentType;

    public:
        static void getPluginList(AsyncWebServerRequest *request);
};