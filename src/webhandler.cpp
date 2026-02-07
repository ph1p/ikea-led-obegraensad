#include "webhandler.h"
#include "config.h"
#include "messages.h"
#include "scheduler.h"
#include "websocket.h"

void sendJsonSuccess(AsyncWebServerRequest *request, const char *message)
{
  JsonDocument jsonResponse;
  jsonResponse["status"] = "success";
  jsonResponse["message"] = message;

  String output;
  serializeJson(jsonResponse, output);
  request->send(200, "application/json", output);
}

void sendJsonError(AsyncWebServerRequest *request, int statusCode, const char *error)
{
  JsonDocument jsonResponse;
  jsonResponse["error"] = true;
  jsonResponse["message"] = error;

  String output;
  serializeJson(jsonResponse, output);
  request->send(statusCode, "application/json", output);
}

// http://your-server/message?text=Hello&repeat=3&id=42&graph=1,2,3,4
void handleMessage(AsyncWebServerRequest *request)
{
  std::string text = request->arg("text").c_str();
  int repeat = request->arg("repeat").toInt();
  int id = request->arg("id").toInt();
  int delay = request->arg("delay").toInt();
  int miny = request->arg("miny").toInt();
  int maxy = request->arg("maxy").toInt();

  if (delay <= 0)
  {
    delay = 50;
  }

  if (maxy == 0)
  {
    maxy = 15;
  }

  // Extracting the 'graph' parameter as a comma-separated list of integers
  std::string graphParam = request->arg("graph").c_str();
  std::vector<int> graph;

  char *token = strtok(const_cast<char *>(graphParam.c_str()), ",");
  while (token != nullptr)
  {
    // Convert the substring to an integer and add it to the vector
    graph.push_back(std::stoi(token));
    token = strtok(nullptr, ",");
  }

  // Add the message
  Messages.add(text, repeat, id, delay, graph, miny, maxy);

  sendJsonSuccess(request, "Message received");
}

// http://your-server/removemessage?id=42
void handleMessageRemove(AsyncWebServerRequest *request)
{
  int id = request->arg("id").toInt();
  Messages.remove(id);

  sendJsonSuccess(request, "Message removed");
}

void handleSetPlugin(AsyncWebServerRequest *request)
{
  int id = request->arg("id").toInt();
  pluginManager.setActivePluginById(id);

  if (pluginManager.getActivePlugin() && pluginManager.getActivePlugin()->getId() == id)
  {
    sendJsonSuccess(request, "Plugin set successfully");
  }
  else
  {
    char errorMsg[64];
    snprintf(errorMsg, sizeof(errorMsg), "Could not set plugin to id %d", id);
    sendJsonError(request, 422, errorMsg);
  }
}

void handleSetBrightness(AsyncWebServerRequest *request)
{
  int value = request->arg("value").toInt();

  if (value < 0 || value > 255)
  {
    char errorMsg[80];
    snprintf(errorMsg,
             sizeof(errorMsg),
             "Invalid brightness value: %d - must be between 0 and 255.",
             value);
    sendJsonError(request, 422, errorMsg);
    return;
  }

  Screen.setBrightness(value, true);
  sendJsonSuccess(request, "Brightness set successfully");
}

void handleGetData(AsyncWebServerRequest *request)
{
  try
  {
    AsyncResponseStream *response = request->beginResponseStream("application/octet-stream");

    int currentpos_src = 0;
    for (int row = 0; row < ROWS; row++)
    {
      for (int col = 0; col < COLS; col++)
      {
        response->print(Screen.getRenderBuffer()[currentpos_src]);
        currentpos_src += 1;
      }
    }

    request->send(response);
  }
  catch (const std::exception &e)
  {
    sendJsonError(request, 500, e.what());
  }
}

void handleGetInfo(AsyncWebServerRequest *request)
{
  JsonDocument jsonDocument;
  jsonDocument["rows"] = ROWS;
  jsonDocument["cols"] = COLS;
  jsonDocument["status"] = currentStatus;
  jsonDocument["plugin"] = pluginManager.getActivePlugin()->getId();
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
  jsonDocument.clear();

  request->send(200, "application/json", output);
}

void handleSetSchedule(AsyncWebServerRequest *request)
{
  bool scheduleIsSet = Scheduler.setScheduleByJSONString(request->arg("schedule"));

  if (!scheduleIsSet)
  {
    sendJsonError(request, 400, "Schedule cannot be set");
    return;
  }

  Scheduler.start();
  sendInfo();

  sendJsonSuccess(request, "Schedule updated");
}

void handleClearSchedule(AsyncWebServerRequest *request)
{
  Scheduler.clearSchedule(true);
  sendInfo();

  sendJsonSuccess(request, "Schedule cleared");
}

void handleStopSchedule(AsyncWebServerRequest *request)
{
  if (!Scheduler.schedule.empty())
  {
    Scheduler.stop();
    sendInfo();
    sendJsonSuccess(request, "Schedule stopped");
  }
  else
  {
    sendJsonError(request, 404, "No schedule found");
  }
}

void handleStartSchedule(AsyncWebServerRequest *request)
{
  if (!Scheduler.schedule.empty())
  {
    Scheduler.start();
    sendInfo();
    sendJsonSuccess(request, "Schedule started");
  }
  else
  {
    sendJsonError(request, 404, "No schedule found");
  }
}

void handleClearStorage(AsyncWebServerRequest *request)
{
#ifdef ENABLE_STORAGE
  storage.begin("led-wall", false);
  storage.clear();
  storage.end();

  sendJsonSuccess(request, "Storage cleared");
#endif
}

void handleGetConfig(AsyncWebServerRequest *request)
{
  Serial.println("[WebHandler] GET /api/config");
  try {
    String json = config.toJson();
    Serial.println("[WebHandler] ============================================");
    Serial.println("[WebHandler] Current Configuration:");
    Serial.print("[WebHandler] Weather Location: ");
    Serial.println(config.getWeatherLocation());
    Serial.print("[WebHandler] NTP Server: ");
    Serial.println(config.getNtpServer());
    Serial.print("[WebHandler] Timezone: ");
    Serial.println(config.getTzInfo());
    Serial.print("[WebHandler] Auto-Start Schedule: ");
    Serial.println(config.getAutoStartSchedule() ? "enabled" : "disabled");
    Serial.println("[WebHandler] ============================================");
    request->send(200, "application/json", json);
  } catch (...) {
    Serial.println("[WebHandler] ERROR: Exception in handleGetConfig");
    sendJsonError(request, 500, "Error reading configuration");
  }
}

void handleSetConfigBody(AsyncWebServerRequest *request,
                         uint8_t *data,
                         size_t len,
                         size_t index,
                         size_t total)
{
  if (index == 0)
  {
    request->_tempObject = new String();
  }

  String *body = static_cast<String *>(request->_tempObject);
  if (!body)
  {
    sendJsonError(request, 500, "Internal buffer error");
    return;
  }

  if (index == 0)
  {
    body->reserve(total);
  }

  body->concat(reinterpret_cast<char *>(data), len);

  if (index + len != total)
  {
    return;
  }

  Serial.println("[WebHandler] POST /api/config (body)");
  Serial.print("[WebHandler] Received JSON: ");
  Serial.println(*body);

  try
  {
    if (body->length() == 0)
    {
      Serial.println("[WebHandler] ERROR: No JSON body");
      sendJsonError(request, 400, "No JSON body provided");
    }
    else if (config.fromJson(*body))
    {
      Serial.println("[WebHandler] JSON parsed successfully");
      config.save();
      Serial.println("[WebHandler] ============================================");
      Serial.println("[WebHandler] Configuration Updated:");
      Serial.print("[WebHandler] Weather Location: ");
      Serial.println(config.getWeatherLocation());
      Serial.print("[WebHandler] NTP Server: ");
      Serial.println(config.getNtpServer());
      Serial.print("[WebHandler] Timezone: ");
      Serial.println(config.getTzInfo());
      Serial.print("[WebHandler] Auto-Start Schedule: ");
      Serial.println(config.getAutoStartSchedule() ? "enabled" : "disabled");
      Serial.println("[WebHandler] ============================================");
      sendJsonSuccess(request, "Configuration saved successfully");
    }
    else
    {
      Serial.println("[WebHandler] ERROR: Invalid JSON format");
      sendJsonError(request, 400, "Invalid JSON format");
    }
  }
  catch (...)
  {
    Serial.println("[WebHandler] ERROR: Exception in handleSetConfigBody");
    sendJsonError(request, 500, "Error saving configuration");
  }

  delete body;
  request->_tempObject = nullptr;
}

void handleResetConfig(AsyncWebServerRequest *request)
{
  Serial.println("[WebHandler] POST /api/config/reset");
  try {
    config.setDefaults();
    config.save();
    Serial.println("[WebHandler] ============================================");
    Serial.println("[WebHandler] Configuration Reset to Defaults:");
    Serial.print("[WebHandler] Weather Location: ");
    Serial.println(config.getWeatherLocation());
    Serial.print("[WebHandler] NTP Server: ");
    Serial.println(config.getNtpServer());
    Serial.print("[WebHandler] Timezone: ");
    Serial.println(config.getTzInfo());
    Serial.print("[WebHandler] Auto-Start Schedule: ");
    Serial.println(config.getAutoStartSchedule() ? "enabled" : "disabled");
    Serial.println("[WebHandler] ============================================");
    sendJsonSuccess(request, "Configuration reset to defaults");
  } catch (...) {
    Serial.println("[WebHandler] ERROR: Exception in handleResetConfig");
    sendJsonError(request, 500, "Error resetting configuration");
  }
}
