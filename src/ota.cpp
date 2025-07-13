#include "ota.h"

#ifdef ENABLE_SERVER

const char *otaUser = OTA_USERNAME;
const char *otaPassword = OTA_PASSWORD;

unsigned long ota_progress_millis = 0;

void onOTAStart()
{
  // Log when OTA has started
  Serial.println("OTA update started!");
  currentStatus = UPDATE;

  std::vector<int> bits = Screen.readBytes(letterU);

  for (int i = 0; i < bits.size(); i++)
  {
    Screen.setPixelAtIndex(i, bits[i]);
  }
  Screen.present();
}

void onOTAProgress(size_t current, size_t final)
{
  // Log every 1 second
  if (millis() - ota_progress_millis > 1000)
  {
    ota_progress_millis = millis();
    Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
  }
}

void onOTAEnd(bool success)
{
  // Log when OTA has finished
  if (success)
  {
    Serial.println("OTA update finished successfully!");
  }
  else
  {
    Serial.println("There was an error during OTA update!");
  }
  std::vector<int> bits = Screen.readBytes(letterR);

  for (int i = 0; i < bits.size(); i++)
  {
    Screen.setPixelAtIndex(i, bits[i]);
  }
  Screen.present();

  delay(1000);
  currentStatus = NONE;
  Screen.loadFromStorage();
  Screen.present();
}

void initOTA(AsyncWebServer &server)
{
  ElegantOTA.begin(&server); // Start ElegantOTA
  ElegantOTA.setAuth(otaUser, otaPassword);
  // ElegantOTA callbacks
  ElegantOTA.onStart(onOTAStart);
  ElegantOTA.onProgress(onOTAProgress);
  ElegantOTA.onEnd(onOTAEnd);
}

#endif
