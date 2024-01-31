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
  
  // Handle API request to get status
  server.on("/getStatus", HTTP_GET, handleGetStatus);

  // Handle API request to get status
  server.on("/getMetadata", HTTP_GET, handleGetMetadata);

  // Handle API request to set an active plugin by ID
  server.on("/setplugin", HTTP_PATCH, handleSetPlugin);

  // Handle API request to set the brightness (0..255);
  server.on("/setbrightness", HTTP_PATCH, handleSetBrightness);
  server.on("/getData", HTTP_GET, handleGetData);

  server.begin();
}

#endif
