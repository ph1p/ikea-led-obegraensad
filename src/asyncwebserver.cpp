#include "asyncwebserver.h"
#include "messages.h"

#ifdef ENABLE_SERVER

AsyncWebServer server(80);

void initWebServer()
{
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Accept, Content-Type, Authorization");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Credentials", "true");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");

  server.on("/", HTTP_GET, startGui);
  server.onNotFound([](AsyncWebServerRequest *request)
                    { request->send(404, "text/plain", "Page not found!"); });

  // Route to handle  http://your-server/message?text=Hello&repeat=3&id=42&delay=30&graph=1,2,3,4&miny=0&maxy=15
  server.on("/message", HTTP_GET, handleMessage);
  server.on("/removemessage", HTTP_GET, handleRemove);


  // Handle API request to retrieve plugin list
  server.on("/getpluginlist", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    // Create a JSON document with sufficient capacity
    DynamicJsonDocument jsonDocument(6144);

    // Get a reference to all plugins
    std::vector<Plugin *> &allPlugins = pluginManager.getAllPlugins();

    // Loop through each plugin and add its details to the JSON document
    for (Plugin *plugin : allPlugins)
    {
      JsonObject object = jsonDocument.createNestedObject();

      // Add plugin details to the JSON object
      object["id"] = plugin->getId();
      object["name"] = plugin->getName();
    }

    // Serialize JSON document to a string
    String output;
    serializeJson(jsonDocument, output);

    // Send the JSON response to the client
    request->send(200, "application/json", output);
  });

  // Handle API request to set an active plugin by ID
  server.on("/setplugin", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    // Extract the 'id' parameter from the request
    int id = request->arg("id").toInt();

    // Set the active plugin based on the provided ID
    pluginManager.setActivePluginById(id);

    // Check if the active plugin has been successfully set
    if (pluginManager.getActivePlugin() && pluginManager.getActivePlugin()->getId() == id)
    {
      // Send a success response to the client
      request->send(200, "application/json", R"({"success": "Plugin Set"})");
    }
    else
    {
      // Send a not found response to the client
      request->send(422, "application/json", R"({"error": "Plugin Not Found"})");
    }
  });

    // Handle API request to set the brightness (0..255);
  server.on("/setbrightness", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    // Extract the 'value' parameter from the request
    int value = request->arg("value").toInt();

    if(value < 0 || value > 255){
      // Send a error response to the client
      request->send(422, "application/json", R"({"error": "Invalid Brightness Value"})");
      return;
    }

    Screen.setBrightness(value);
 
    request->send(200, "application/json", R"({"success": "Brightness Set"})");

  });

  server.begin();
}

#endif
