#include "webhandler.h"
#include "messages.h"
#include "scheduler.h"
#include "websocket.h"

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

    // JSON response
    StaticJsonDocument<256> jsonResponse;
    jsonResponse["status"] = "success";
    jsonResponse["message"] = "Message received";

    String output;
    serializeJson(jsonResponse, output);
    request->send(200, "application/json", output);
}

// http://your-server/removemessage?id=42
void handleMessageRemove(AsyncWebServerRequest *request)
{
    int id = request->arg("id").toInt();
    Messages.remove(id);

    StaticJsonDocument<256> jsonResponse;
    jsonResponse["status"] = "success";
    jsonResponse["message"] = "Message removed";

    String output;
    serializeJson(jsonResponse, output);
    request->send(200, "application/json", output);
}

void handleSetPlugin(AsyncWebServerRequest *request)
{
    int id = request->arg("id").toInt();
    pluginManager.setActivePluginById(id);

    StaticJsonDocument<256> jsonResponse;

    if (pluginManager.getActivePlugin() && pluginManager.getActivePlugin()->getId() == id)
    {
        jsonResponse["status"] = "success";
        jsonResponse["message"] = "Plugin set successfully";
        String output;
        serializeJson(jsonResponse, output);
        request->send(200, "application/json", output);
    }
    else
    {
        jsonResponse["error"] = true;
        jsonResponse["errormessage"] = "Could not set plugin to id " + std::to_string(id);
        String output;
        serializeJson(jsonResponse, output);
        request->send(422, "application/json", output);
    }
}

void handleSetBrightness(AsyncWebServerRequest *request)
{
    int value = request->arg("value").toInt();

    StaticJsonDocument<256> jsonResponse;

    if (value < 0 || value > 255)
    {
        jsonResponse["error"] = true;
        jsonResponse["errormessage"] = "Invalid brightness value: " + std::to_string(value) + " - must be between 0 and 255.";
        String output;
        serializeJson(jsonResponse, output);
        request->send(422, "application/json", output);
        return;
    }

    Screen.setBrightness(value, true);

    jsonResponse["status"] = "success";
    jsonResponse["message"] = "Brightness set successfully";

    String output;
    serializeJson(jsonResponse, output);
    request->send(200, "application/json", output);
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
        StaticJsonDocument<256> jsonResponse;
        jsonResponse["error"] = true;
        jsonResponse["errormessage"] = e.what();
        String output;
        serializeJson(jsonResponse, output);
        request->send(500, "application/json", output);
    }
}

void handleGetInfo(AsyncWebServerRequest *request)
{
    DynamicJsonDocument jsonDocument(6144);
    jsonDocument["rows"] = ROWS;
    jsonDocument["cols"] = COLS;
    jsonDocument["status"] = currentStatus;
    jsonDocument["plugin"] = pluginManager.getActivePlugin()->getId();
    jsonDocument["rotation"] = Screen.currentRotation;
    jsonDocument["brightness"] = Screen.getCurrentBrightness();
    jsonDocument["scheduleActive"] = Scheduler.isActive;

    JsonArray scheduleArray = jsonDocument.createNestedArray("schedule");
    for (const auto &item : Scheduler.schedule)
    {
        JsonObject scheduleItem = scheduleArray.createNestedObject();
        scheduleItem["pluginId"] = item.pluginId;
        scheduleItem["duration"] = item.duration / 1000; // Convert milliseconds to seconds
    }

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
    jsonDocument.clear();

    request->send(200, "application/json", output);
}

void handleSetSchedule(AsyncWebServerRequest *request)
{
    bool scheduleIsSet = Scheduler.setScheduleByJSONString(request->arg("schedule"));

    StaticJsonDocument<256> jsonResponse;
    if (!scheduleIsSet)
    {
        jsonResponse["error"] = true;
        jsonResponse["message"] = "Schedule cannot be set";
        String output;
        serializeJson(jsonResponse, output);
        request->send(400, "application/json", output);
        return;
    }

    Scheduler.start();
    sendInfo();

    jsonResponse["status"] = "success";
    jsonResponse["message"] = "Schedule updated";
    String output;
    serializeJson(jsonResponse, output);
    request->send(200, "application/json", output);
}

void handleClearSchedule(AsyncWebServerRequest *request)
{
    Scheduler.clearSchedule(true);
    sendInfo();

    StaticJsonDocument<256> jsonResponse;
    jsonResponse["status"] = "success";
    jsonResponse["message"] = "Schedule cleared";

    String output;
    serializeJson(jsonResponse, output);
    request->send(200, "application/json", output);
}

void handleStopSchedule(AsyncWebServerRequest *request)
{
    StaticJsonDocument<256> jsonResponse;
    if (!Scheduler.schedule.empty())
    {
        Scheduler.stop();
        sendInfo();

        jsonResponse["status"] = "success";
        jsonResponse["message"] = "Schedule stopped";
        String output;
        serializeJson(jsonResponse, output);
        request->send(200, "application/json", output);
    }
    else
    {
        jsonResponse["error"] = true;
        jsonResponse["message"] = "No schedule found";
        String output;
        serializeJson(jsonResponse, output);
        request->send(404, "application/json", output);
    }
}

void handleStartSchedule(AsyncWebServerRequest *request)
{
    StaticJsonDocument<256> jsonResponse;
    if (!Scheduler.schedule.empty())
    {
        Scheduler.start();
        sendInfo();

        jsonResponse["status"] = "success";
        jsonResponse["message"] = "Schedule started";
        String output;
        serializeJson(jsonResponse, output);
        request->send(200, "application/json", output);
    }
    else
    {
        jsonResponse["error"] = true;
        jsonResponse["message"] = "No schedule found";
        String output;
        serializeJson(jsonResponse, output);
        request->send(404, "application/json", output);
    }
}

void handleClearStorage(AsyncWebServerRequest *request)
{
#ifdef ENABLE_STORAGE
    storage.begin("led-wall", false);
    storage.clear();
    storage.end();

    StaticJsonDocument<256> jsonResponse;
    jsonResponse["status"] = "success";
    jsonResponse["message"] = "Storage cleared";

    String output;
    serializeJson(jsonResponse, output);
    request->send(200, "application/json", output);
#endif
}
