#include "ota.h"

#ifdef ENABLE_SERVER

const char *otaUser = OTA_USERNAME;
const char *otaPassword = OTA_PASSWORD;

void onOTAStart()
{
  setMode(UPDATE);
  Screen.setRenderBuffer(letterU);
  Screen.render();
}

void onOTAEnd()
{
  Screen.setRenderBuffer(letterR);
  Screen.render();
  delay(1000);
  Screen.loadFromStorage();
}

void initOTA(AsyncWebServer &server)
{
  AsyncElegantOTA.begin(&server, otaUser, otaPassword);
  AsyncElegantOTA.onOTAStart(onOTAStart);
  AsyncElegantOTA.onOTAEnd(onOTAEnd);
}

#endif
