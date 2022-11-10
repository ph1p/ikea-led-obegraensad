#include "websocket.h"

AsyncWebSocket ws("/ws");

void sendStateAndInfo()
{
  DynamicJsonDocument jsonDocument(6144);
  for (int j = 0; j < sizeof(Screen.renderBuffer); j++)
  {
    jsonDocument["data"][j] = Screen.renderBuffer[j];
  }
  jsonDocument["mode"] = currentMode;
  jsonDocument["event"] = "info";
  jsonDocument["rotation"] = Screen.currentRotation;

  String output;
  serializeJson(jsonDocument, output);
  ws.textAll(output);
}

void sendModeToAllClients(MODE mode)
{
  DynamicJsonDocument jsonDocument(6144);

  jsonDocument["event"] = "mode";
  jsonDocument["mode"] = mode;
  jsonDocument["rotation"] = Screen.currentRotation;

  String output;
  serializeJson(jsonDocument, output);
  ws.textAll(output);
}

void onWsEvent(
    AsyncWebSocket *server,
    AsyncWebSocketClient *client,
    AwsEventType type,
    void *arg,
    uint8_t *data,
    size_t len)
{
  if (type == WS_EVT_CONNECT)
  {
    sendStateAndInfo();
  }

  if (type == WS_EVT_DATA)
  {
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
    {
      data[len] = 0;

      DynamicJsonDocument wsRequest(6144);
      DeserializationError error = deserializeJson(wsRequest, data);

      if (error)
      {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
      }
      else
      {
        const char *event = wsRequest["event"];

        if (!strcmp(event, "mode"))
        {
          setModeByString(wsRequest["mode"], sendModeToAllClients);
        }
        else if (!strcmp(event, "rotate"))
        {
          bool isRight = (bool)!strcmp(wsRequest["direction"], "right");
          Screen.rotate(
              Screen.positions, Screen.currentRotation + (90 * (isRight ? 1 : -1)));

          if (currentMode == NONE)
          {
            delay(10);
            Screen.renderScreen(Screen.renderBuffer);
          }
        }
        else if (!strcmp(event, "info"))
        {
          sendStateAndInfo();
        }

        if (currentMode == NONE)
        {
          if (!strcmp(event, "clear"))
          {
            Screen.clearScreenAndBuffer(Screen.renderBuffer);
          }
          else if (!strcmp(event, "led"))
          {
            Screen.setPixelAtIndex(Screen.renderBuffer, wsRequest["index"], wsRequest["status"]);
            Screen.renderScreen(Screen.renderBuffer);
          }
          else if (!strcmp(event, "screen"))
          {
            for (int i = 0; i < ROWS * COLS; i++)
            {
              Screen.renderBuffer[i] = wsRequest["data"][i];
            }
            delay(10);
            Screen.renderScreen(Screen.renderBuffer);
          }
          else if (!strcmp(event, "persist"))
          {
            storage.begin("led-wall", false);
            storage.putBytes("data", Screen.renderBuffer, sizeof(Screen.renderBuffer));
            storage.end();
          }
          else if (!strcmp(event, "load"))
          {
            Screen.clearScreenAndBuffer(Screen.renderBuffer);
            storage.begin("led-wall", false);
            storage.getBytes("data", Screen.renderBuffer, sizeof(Screen.renderBuffer));
            storage.end();
            delay(10);
            Screen.renderScreen(Screen.renderBuffer);
            sendStateAndInfo();
          }
        }
      }
    }
  }
}

void initWebsocketServer(AsyncWebServer &server)
{
  server.addHandler(&ws);
  ws.onEvent(onWsEvent);
}

void cleanUpClients()
{
  ws.cleanupClients();
}