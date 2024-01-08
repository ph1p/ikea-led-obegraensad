#include "asyncwebserver.h"

#ifdef ENABLE_SERVER

#ifdef ENABLE_REST
#include "restapi.h"
#endif

AsyncWebServer server(80);

void initWebServer()
{
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Accept, Content-Type, Authorization");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Credentials", "true");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");

  server.on("/", HTTP_GET, startGui);
  server.onNotFound([](AsyncWebServerRequest *request)
                    { request->send(404, "text/plain", "Page not found!"); });

  #ifdef ENABLE_REST
  server.on("/plugins", HTTP_GET, RestApi::getPluginList);
  #endif

  server.begin();
}

#endif
