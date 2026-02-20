#pragma once

#include "ESPAsyncWebServer.h"

// Helper functions for JSON responses
void sendJsonSuccess(AsyncWebServerRequest *request, const char *message);
void sendJsonError(AsyncWebServerRequest *request, int statusCode, const char *error);

void handleMessage(AsyncWebServerRequest *request);
void handleMessageRemove(AsyncWebServerRequest *request);
void handleGetInfo(AsyncWebServerRequest *request);
void handleSetPlugin(AsyncWebServerRequest *request);
void handleSetBrightness(AsyncWebServerRequest *request);
void handleGetData(AsyncWebServerRequest *request);
void handleSetSchedule(AsyncWebServerRequest *request);
void handleClearSchedule(AsyncWebServerRequest *request);
void handleStopSchedule(AsyncWebServerRequest *request);
void handleStartSchedule(AsyncWebServerRequest *request);
void handleClearStorage(AsyncWebServerRequest *request);
void handleGetConfig(AsyncWebServerRequest *request);
void handleSetConfigBody(AsyncWebServerRequest *request,
						 uint8_t *data,
						 size_t len,
						 size_t index,
						 size_t total);
void handleResetConfig(AsyncWebServerRequest *request);