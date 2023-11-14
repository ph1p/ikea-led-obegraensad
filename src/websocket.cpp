#include "PluginManager.h"

#ifdef ENABLE_SERVER

AsyncWebSocket ws("/ws");

void sendInfo()
{
  DynamicJsonDocument jsonDocument(6144);
  if (currentStatus == NONE)
  {
    for (int j = 0; j < ROWS * COLS; j++)
    {
      jsonDocument["data"][j] = Screen.getRenderBuffer()[j];
    }
  }
  jsonDocument["status"] = currentStatus;
  jsonDocument["plugin"] = pluginManager.getActivePlugin()->getId();
  jsonDocument["event"] = "info";
  jsonDocument["rotation"] = Screen.currentRotation;
  jsonDocument["brightness"] = Screen.getCurrentBrightness();

  JsonArray plugins = jsonDocument.createNestedArray("plugins");

  std::vector<Plugin *> &allPlugins = pluginManager.getAllPlugins();
  for (Plugin *plugin : allPlugins)
  {
    JsonObject object = plugins.createNestedObject();

    object["id"] = plugin->getId();
    object["name"] = plugin->getName();
  }

  String output;
  serializeJson(jsonDocument, output);
  ws.textAll(output);
}

void sendMinimalInfo()
{
  DynamicJsonDocument jsonDocument(6144);

  jsonDocument["status"] = currentStatus;
  jsonDocument["plugin"] = pluginManager.getActivePlugin()->getId();
  jsonDocument["event"] = "info";
  jsonDocument["rotation"] = Screen.currentRotation;
  jsonDocument["brightness"] = Screen.getCurrentBrightness();

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
          pluginManager.getActivePlugin()->websocketHook(wsRequest);

          const char *event = wsRequest["event"];

          if (!strcmp(event, "plugin"))
          {
            int pluginId = wsRequest["plugin"];

            pluginManager.setActivePluginById(pluginId);

            sendMinimalInfo();
          }
          else if (!strcmp(event, "persist-plugin"))
          {
            pluginManager.persistActivePlugin();
          }
          else if (!strcmp(event, "rotate"))
          {
            bool isRight = (bool)!strcmp(wsRequest["direction"], "right");

            Screen.currentRotation = isRight ? (Screen.currentRotation > 3 ? 1 : Screen.currentRotation + 1) : (Screen.currentRotation <= 0 ? 3 : Screen.currentRotation - 1);
          }
          else if (!strcmp(event, "info"))
          {
            sendInfo();
          }
          else if (!strcmp(event, "brightness"))
          {
            Screen.setBrightness(wsRequest["brightness"].as<uint8_t>());
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