#include "messages.h"
#include <SPI.h>

using namespace std;

Messages_ &Messages_::getInstance()
{
    static Messages_ instance;
    return instance;
}

void Messages_::add(std::string text, int repeat, int id, int delay, std::vector<int> graph, int miny, int maxy)
{
    remove(id); // there should be only one message by id
    messages.emplace_back(Message{id, repeat, delay, text, graph, miny, maxy});
    previousMinute = -1; // force the message to be displayed immediately in the next loop. The http handler cannot run for too long
}

void Messages_::remove(int id)
{
    // Use std::remove_if to move the messages to be removed to the end of the vector
    auto it = std::remove_if(messages.begin(), messages.end(), [id](const Message &msg)
                             { return msg.id == id; });

    // Erase the messages marked for removal
    messages.erase(it, messages.end());
}

void Messages_::scroll()
{

    Screen.persist();

    for (auto it = messages.begin(); it != messages.end();)
    {

        // Print the text for each message
        if (it->text.length() > 0)
            Screen.scrollText(it->text.c_str(), it->delay);
        if (it->graph.size() > 0)
            Screen.scrollGraph(it->graph, it->miny, it->maxy, it->delay);

        // if the repeat is not -1, which is the flag for infinite repeat
        if (it->repeat != -1)
        {
            // Decrease repeat and remove if it becomes less than 0
            if (--(it->repeat) < 0)
            {
                it = messages.erase(it);
            }
            else
            {
                ++it;
            }
        }
        else
        {
            ++it;
        }
    }

    // restore old screen
    Screen.loadFromStorage();
}

void Messages_::scrollMessageEveryMinute()
{

    // Structure to hold time information
    struct tm timeinfo;

    // Check if the local time can be obtained
    if (getLocalTime(&timeinfo))
    {
        // Check if the current minute is different from the previous minute
        if (timeinfo.tm_min != previousMinute)
        {
            // Call the scroll function to display messages
            scroll();

            // Update the previous minute to the current minute
            previousMinute = timeinfo.tm_min;
        }

        if (timeinfo.tm_sec != previousSecond)
        {
            if (messages.size() > 0)
            {
                indicatorPixel = previousSecond & 0b00000001; // every other second, bit of a geek style
                Screen.setPixel(0, 0, indicatorPixel);
            }
            else
            { // if the indicator pixel was set, but no message exisits, reset it
                if (indicatorPixel > 0)
                {
                    indicatorPixel = 0;
                    Screen.setPixel(0, 0, indicatorPixel);
                }
            }

            previousSecond = timeinfo.tm_sec;
        }
    }
}

Messages_ &Messages = Messages.getInstance();

// http://your-server/message?text=Hello&repeat=3&id=42&graph=1,2,3,4
void handleMessage(AsyncWebServerRequest *request)
{

    // Extracting parameters from the URL
    std::string text = request->arg("text").c_str();
    int repeat = request->arg("repeat").toInt();
    int id = request->arg("id").toInt();
    int delay = request->arg("delay").toInt();
    int miny = request->arg("miny").toInt();
    int maxy = request->arg("maxy").toInt();

    // if no delay has been passed, use 50 ms
    if (delay <= 0)
        delay = 50;

    // default maxy to 15
    if (maxy == 0)
        maxy = 15;

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

    // Uncomment this to thee what charcode comes in
    //  for(int i = 0;i<text.size();i++)Screen.scrollText(std::to_string(text[i]));

    // Call the add function with the extracted parameters
    Messages.add(text, repeat, id, delay, graph, miny, maxy);

    // Send a response to the client
    request->send(200, "text/plain", "Message received");
}

// http://your-server/removemessage?id=42
void handleRemove(AsyncWebServerRequest *request)
{

    int id = request->arg("id").toInt();

    // Call the add function with the extracted parameters
    Messages.remove(id);

    // Send a response to the client
    request->send(200, "text/plain", "Message received");
}

void handleSetPlugin(AsyncWebServerRequest *request)
{
    // Extract the 'id' parameter from the request
    int id = request->arg("id").toInt();

    // Set the active plugin based on the provided ID
    pluginManager.setActivePluginById(id);

    // Check if the active plugin has been successfully set
    if (pluginManager.getActivePlugin() && pluginManager.getActivePlugin()->getId() == id)
    {
      // Send a success response to the client
      request->send(200, "text/plain");
    }
    else
    {
      StaticJsonDocument<256> jsonDocument;
      jsonDocument["error"] = true;
      jsonDocument["errormessage"] = "could not set plugin to id " + to_string(id);

      String output;
      serializeJson(jsonDocument, output);
      request->send(422, "application/json", output);
    }
}

void handleSetBrightness(AsyncWebServerRequest *request)
{
        // Extract the 'value' parameter from the request
    int value = request->arg("value").toInt();

    if(value < 0 || value > 255){
      // Send a error response to the client
      StaticJsonDocument<256> jsonDocument;
      jsonDocument["error"] = true;
      jsonDocument["errormessage"] = "invalid brightness value: " + to_string(value) + " - must be between 0 and 255.";

      String output;
      serializeJson(jsonDocument, output);
      request->send(422, "application/json", output);
      return;
    }

    Screen.setBrightness(value);
 
    request->send(200, "text/plain");
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

        // Send the raw data response to the client
        request->send(response);
    }
    catch (const std::exception& e) 
    {
        request->send(500, "text/plain", e.what());
    }
}

void handleGetStatus(AsyncWebServerRequest *request)
{
    DynamicJsonDocument jsonDocument(6144);
    jsonDocument["status"] = currentStatus;
    jsonDocument["plugin"] = pluginManager.getActivePlugin()->getId();
    jsonDocument["event"] = "info";
    jsonDocument["rotation"] = Screen.currentRotation;
    jsonDocument["brightness"] = Screen.getCurrentBrightness();
    jsonDocument["rows"] = ROWS;
    jsonDocument["cols"] = COLS;

    std::vector<Plugin *> &allPlugins = pluginManager.getAllPlugins();

    // Loop through each plugin and add its details to the JSON document
    for (Plugin *plugin : allPlugins)
    {
      JsonObject object = jsonDocument.createNestedObject();

      // Add plugin details to the JSON object
      object["id"] = plugin->getId();
      object["name"] = plugin->getName();
    }

    String output;
    serializeJson(jsonDocument, output);

    // Send the JSON response to the client
    request->send(200, "application/json", output);
}