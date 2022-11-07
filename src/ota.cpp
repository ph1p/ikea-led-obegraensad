#include "ota.h"

const char *otaUser = OTA_USERNAME;
const char *otaPassword = OTA_PASSWORD;

void onOTAStart()
{
  setMode(UPDATE);
  renderScreen(letterU);
}

void onOTAEnd()
{
  renderScreen(letterR);
  delay(1000);
  renderScreen(render_buffer);
}

void initOTA(AsyncWebServer &server)
{
  AsyncElegantOTA.begin(&server, otaUser, otaPassword);
  AsyncElegantOTA.onOTAStart(onOTAStart);
  AsyncElegantOTA.onOTAEnd(onOTAEnd);
}
