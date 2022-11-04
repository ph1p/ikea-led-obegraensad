#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

#include "ws.h"
#include "storage.h"
#include "led.h"
#include "mode/mode.h"
#include "constants.h"

AsyncWebSocket ws("/ws");

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
  }
  else if (type == WS_EVT_DISCONNECT)
  {
  }
  else if (type == WS_EVT_DATA)
  {
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
    {
      data[len] = 0;

      StaticJsonDocument<4132> ws_request;
      DeserializationError error = deserializeJson(ws_request, data);

      if (error)
      {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
      }
      else
      {
        const char *event = ws_request["event"];

        if (!strcmp(event, "mode"))
        {
          if (ws_request["m"] == "stars")
          {
            setMode(STARS);
          }
          else if (ws_request["m"] == "lines")
          {
            setMode(LINES);
          }
          else if (ws_request["m"] == "breakout")
          {
            setMode(BREAKOUT);
          }
          else if (ws_request["m"] == "gameoflife")
          {
            setMode(GAMEOFLIFE);
          }
          else
          {
            setMode(NONE);
          }
        }

        if (current_mode == NONE)
        {
          if (!strcmp(event, "init"))
          {
            DynamicJsonDocument doc(4132);
            JsonObject root = doc.to<JsonObject>();
            JsonArray d = root.createNestedArray("data");
            for (int j = 0; j < sizeof(render_buffer); j++)
            {
              d.add(render_buffer[j]);
            }
            root["event"] = "init";

            char buffer[4132];
            size_t len = serializeJson(root, buffer);
            client->text(buffer);
          }
          else if (!strcmp(event, "clear"))
          {
            memset(render_buffer, 0, sizeof(render_buffer));
            renderScreen(render_buffer);
          }
          else if (!strcmp(event, "led"))
          {
            setPixelAtIndex(render_buffer, ws_request["i"], ws_request["s"]);
            renderScreen(render_buffer);
          }
          else if (!strcmp(event, "persist"))
          {
            storage.begin("led-wall", false);
            storage.putBytes("data", render_buffer, sizeof(render_buffer));
            storage.end();
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