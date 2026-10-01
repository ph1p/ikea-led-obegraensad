#include "PluginManager.h"
#include "brightness_schedule.h"
#include "scheduler.h"

#include <algorithm>
#include <vector>
#ifdef ESP32
#include <mutex>
#endif

#ifdef ENABLE_SERVER

AsyncWebSocket ws("/ws");

// IDs of clients that receive live frames. Clients are subscribed on connect
// and can opt out with {"event":"live-preview","enabled":false}. Written from
// the websocket event handler, read from loop(), hence the mutex.
std::vector<uint32_t> liveClients;
// set when a client subscribes, so it gets the current frame even if the
// screen does not change
volatile bool liveFrameForced = false;
#ifdef ESP32
std::mutex liveClientsLock;
#define LIVE_CLIENTS_GUARD std::lock_guard<std::mutex> guard(liveClientsLock)
#else
// ESP8266 runs websocket events and loop() on the same thread
#define LIVE_CLIENTS_GUARD
#endif

void setLiveClient(uint32_t id, bool enabled)
{
  LIVE_CLIENTS_GUARD;
  auto it = std::find(liveClients.begin(), liveClients.end(), id);
  if (enabled && it == liveClients.end())
  {
    liveClients.push_back(id);
    liveFrameForced = true;
  }
  else if (!enabled && it != liveClients.end())
  {
    liveClients.erase(it);
  }
}

void sendInfo()
{
  JsonDocument jsonDocument;
  JsonArray data = jsonDocument["data"].to<JsonArray>();
  const uint8_t *buffer = Screen.getRenderBuffer();
  for (int j = 0; j < ROWS * COLS; j++)
  {
    data.add(buffer[j]);
  }

  jsonDocument["status"] = currentStatus;
  jsonDocument["plugin"] = pluginManager.getActivePlugin()->getId();
  jsonDocument["persist-plugin"] = pluginManager.getPersistedPluginId();
  jsonDocument["event"] = "info";
  jsonDocument["rotation"] = Screen.currentRotation;
  jsonDocument["baseBrightness"] = Screen.getBaseBrightness();
  jsonDocument["brightness"] = Screen.getCurrentBrightness();
  JsonObject brightnessSchedule = jsonDocument["brightnessSchedule"].to<JsonObject>();
  ScheduledBrightness.writeToJson(brightnessSchedule);
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

// sends the presented frame to subscribed clients, but only when it changed
void sendLiveFrame()
{
  static uint32_t lastSentFrame = 0;

  uint8_t frame[ROWS * COLS];
  const uint32_t frameCounter = Screen.copyPresentedFrame(frame);
  if (frameCounter == lastSentFrame && !liveFrameForced)
  {
    return;
  }

  // copied out instead of sending under the lock: the websocket library calls
  // setLiveClient() while holding its own lock, sending takes that lock too
  uint32_t ids[DEFAULT_MAX_WS_CLIENTS];
  size_t count = 0;
  {
    LIVE_CLIENTS_GUARD;
    for (uint32_t id : liveClients)
    {
      if (count < sizeof(ids) / sizeof(ids[0]))
      {
        ids[count++] = id;
      }
    }
  }
  if (count == 0)
  {
    return;
  }
  lastSentFrame = frameCounter;
  liveFrameForced = false;

  // one buffer shared by all clients instead of a copy per client
  AsyncWebSocketSharedBuffer buffer =
      std::make_shared<std::vector<uint8_t>>(frame, frame + sizeof(frame));
  for (size_t i = 0; i < count; i++)
  {
    if (ws.availableForWrite(ids[i]))
    {
      ws.binary(ids[i], buffer);
    }
  }
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
    setLiveClient(client->id(), true);
    sendInfo();
  }

  if (type == WS_EVT_DISCONNECT)
  {
    setLiveClient(client->id(), false);
  }

  if (type == WS_EVT_DATA)
  {
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len)
    {
      if (info->opcode == WS_BINARY && currentStatus == WSBINARY && info->len == TOTAL_PIXELS)
      {
        Screen.setRenderBuffer(data, true);
      }
      else if (info->opcode == WS_TEXT)
      {
        JsonDocument wsRequest;
        DeserializationError error = deserializeJson(wsRequest, data, len);

        if (error)
        {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.f_str());
          return;
        }
        else
        {
          // Every handler below and the plugin hooks dispatch on "event", so
          // drop anything without it instead of passing nullptr to strcmp().
          if (!wsRequest["event"].is<const char *>())
          {
            Serial.println(F("websocket message without \"event\" ignored"));
            return;
          }

          Plugin *activePlugin = pluginManager.getActivePlugin();
          if (activePlugin)
          {
            activePlugin->websocketHook(wsRequest);
          }

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
            bool isRight = !strcmp(wsRequest["direction"] | "", "right");
            Screen.setCurrentRotation((Screen.currentRotation + (isRight ? 1 : 3)) % 4, true);
            sendInfo();
          }
          else if (!strcmp(event, "info"))
          {
            sendInfo();
          }
          else if (!strcmp(event, "live-preview"))
          {
            setLiveClient(client->id(), wsRequest["enabled"] | true);
          }
          else if (!strcmp(event, "brightness"))
          {
            uint8_t brightness = wsRequest["brightness"].as<uint8_t>();
            Screen.setBaseBrightness(brightness, true);
            ScheduledBrightness.update();
            sendInfo();
          }
          else if (!strcmp(event, "brightness-schedule"))
          {
            ScheduledBrightness.configureFromJson(wsRequest.as<JsonVariantConst>());
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
