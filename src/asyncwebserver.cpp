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

  /*
  # Plugin Management API

  This API provides endpoints to manage plugins through an AsyncWebServer. Two main endpoints are available:

  1. **Get Plugin List:**
     - Endpoint: `/getpluginlist`
     - Method: `HTTP_GET`
     - Description: Retrieves a list of available plugins in JSON format.

  2. **Set Active Plugin:**
     - Endpoint: `/setplugin`
     - Method: `HTTP_GET`
     - Parameters:
        - `id` (integer): ID of the plugin to set as active.
     - Description: Sets an active plugin based on the provided ID.

  ## How to Use

  1. **Get Plugin List:**
     - Endpoint: `/getpluginlist`
     - Method: `HTTP_GET`
     - Example:
       ```javascript
       server.on("/getpluginlist", HTTP_GET, handleApiGetPluginList);
       ```

     - Response Format:
       ```json
       [
         {"id": 1, "name": "Plugin A"},
         {"id": 2, "name": "Plugin B"},
         // ... other plugins
       ]
       ```

  2. **Set Active Plugin:**
     - Endpoint: `/setplugin`
     - Method: `HTTP_GET`
     - Parameters:
       - `id` (integer): ID of the plugin to set as active.
     - Example:
       ```javascript
       server.on("/setplugin", HTTP_GET, handleApiSetPlugin);
       ```

     - Response Format:
       - Success (200):
         ```
         Plugin Set
         ```
       - Not Found (404):
         ```
         Plugin not found
         ```

  ## Important Notes

  - Ensure that the appropriate routes and handler functions are registered with your `AsyncWebServer`.

  - The `handleApiSetPlugin` endpoint requires the `id` parameter to be passed in the request URL.

  - The response for the "Get Plugin List" endpoint is a JSON array containing objects with `id` and `name` properties for each plugin.

  - Make sure to customize the code and endpoints according to your project requirements.

  ## Dependencies

  - [AsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer) - Ensure that you have this library installed to handle asynchronous HTTP requests on your server.

  */

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
      request->send(200, "text/plain", "Plugin Set");
    }
    else
    {
      // Send a not found response to the client
      request->send(404, "text/plain", "Plugin not found");
    }
  });
  server.begin();
}

#endif
