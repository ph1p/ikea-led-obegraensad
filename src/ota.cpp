#include "ota.h"

#ifdef ENABLE_SERVER

const char *otaUser = OTA_USERNAME;
const char *otaPassword = OTA_PASSWORD;

void onOTAStart()
{
  currentStatus = UPDATE;

  std::vector<int> bits = Screen.readBytes(letterU);

  for (int i = 0; i < bits.size(); i++)
  {
    Screen.setPixelAtIndex(i, bits[i]);
  }
}

void onOTAEnd()
{
  std::vector<int> bits = Screen.readBytes(letterR);

  for (int i = 0; i < bits.size(); i++)
  {
    Screen.setPixelAtIndex(i, bits[i]);
  }

  currentStatus = NONE;

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
