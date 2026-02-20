#include "PluginManager.h"
#include "scheduler.h"

#ifdef ENABLE_SERVER

AsyncWebSocket ws("/ws");

void sendInfo()
{
  JsonDocument jsonDocument;
  if (currentStatus == NONE)
  {
    for (int j = 0; j < ROWS * COLS; j++)
    {
      jsonDocument["data"][j] = Screen.getRenderBuffer()[j];
    }
  }

  jsonDocument["status"] = currentStatus;
  jsonDocument["plugin"] = pluginManager.getActivePlugin()->getId();
  jsonDocument["persist-plugin"] = pluginManager.getPersistedPluginId();
  jsonDocument["event"] = "info";
  jsonDocument["rotation"] = Screen.currentRotation;
  jsonDocument["brightness"] = Screen.getCurrentBrightness();
  jsonDocument["scheduleActive"] = Scheduler.isActive;

  JsonArray scheduleArray = jsonDocument["schedule"].to<JsonArray>();
  for (const auto &item : Scheduler.schedule)
  {
    JsonObject scheduleItem = scheduleArray.add<JsonObject>();
    scheduleItem["pluginId"] = item.pluginId;
    scheduleItem["duration"] = item.duration / 1000; // Convert milliseconds to seconds
  }

  JsonArray plugins = jsonDocument["plugins"].to<JsonArray>();

  std::vector<Plugin *> &allPlugins = pluginManager.getAllPlugins();
  for (Plugin *plugin : allPlugins)
  {
    JsonObject object = plugins.add<JsonObject>();

    object["id"] = plugin->getId();
    object["name"] = plugin->getName();
  }
  String output;
  serializeJson(jsonDocument, output);
  ws.textAll(output);
  jsonDocument.clear();
}

void sendWSMessage(String &message) {
  ws.textAll(message);
}

void onWsEvent(AsyncWebSocket *server,
               AsyncWebSocketClient *client,
               AwsEventType type,
               void *arg,
               uint8_t *data,
               size_t len)
{
  if (type == WS_EVT_CONNECT)
  {
    sendInfo();
  }

  if (type == WS_EVT_DATA)
  {
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len)
    {
      if (info->opcode == WS_BINARY && currentStatus == WSBINARY && info->len == 256)
      {
        Screen.setRenderBuffer(data, true);
      }
      else if (info->opcode == WS_TEXT)
      {
        data[len] = 0;

        JsonDocument wsRequest;
        DeserializationError error = deserializeJson(wsRequest, data);

        if (error)
        {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.f_str());
          return;
        }
        else
        {
          pluginManager.getActivePlugin()->websocketHook(wsRequest);

          const char *event = wsRequest["event"];

          if (!strcmp(event, "plugin"))
          {
            int pluginId = wsRequest["plugin"];

            Scheduler.clearSchedule();
            pluginManager.setActivePluginById(pluginId);
            sendInfo();
          }
          else if (!strcmp(event, "persist-plugin"))
          {
            pluginManager.persistActivePlugin();
            sendInfo();
          }
          else if (!strcmp(event, "rotate"))
          {
            bool isRight = (bool)!strcmp(wsRequest["direction"], "right");
            Screen.setCurrentRotation((Screen.currentRotation + (isRight ? 1 : 3)) % 4, true);
            sendInfo();
          }
          else if (!strcmp(event, "info"))
          {
            sendInfo();
          }
          else if (!strcmp(event, "brightness"))
          {
            uint8_t brightness = wsRequest["brightness"].as<uint8_t>();
            Screen.setBrightness(brightness, true);
            sendInfo();
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

#endif
