#include "restapi.h"

#ifdef ENABLE_REST

const char* RestApi::jsonContentType = "application/json";

void RestApi::getPluginList(AsyncWebServerRequest *request)
{
    AsyncWebServerResponse *response = request->beginResponse(200, RestApi::jsonContentType, "Clock; Binary Clock; Cookies");
    //response->addHeader("Content-Encoding", "gzip");
    request->send(response);
}

#endif // ENABLE_REST