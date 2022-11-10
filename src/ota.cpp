#include "ota.h"

const char *otaUser = OTA_USERNAME;
const char *otaPassword = OTA_PASSWORD;

void onOTAStart()
{
  setMode(UPDATE);
  Screen.renderScreen(letterU);
}

void onOTAEnd()
{
  Screen.renderScreen(letterR);
  delay(1000);
  Screen.renderScreen(Screen.renderBuffer);
}

void initOTA(AsyncWebServer &server)
{
  AsyncElegantOTA.begin(&server, otaUser, otaPassword);
  AsyncElegantOTA.onOTAStart(onOTAStart);
  AsyncElegantOTA.onOTAEnd(onOTAEnd);
}
