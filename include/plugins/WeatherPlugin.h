#pragma once

#ifdef ESP32
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#endif
#ifdef ESP8266
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#endif
#include "PluginManager.h"
#include <ArduinoJson.h>
class WeatherPlugin : public Plugin
{
private:
  unsigned long lastUpdate = 0;
  HTTPClient http;
#ifdef ESP32
  WiFiClientSecure *secureClient = nullptr;
#endif
#ifdef ESP8266
  WiFiClient wiFiClient;
#endif

  // Cached weather data
  bool hasCachedData = false;
  int cachedTemperature = 0;
  int cachedWeatherIcon = 0;
  int cachedIconY = 1;
  int cachedTempY = 10;

  std::vector<int> thunderCodes = {200, 386, 389, 392, 395};
  std::vector<int> cloudyCodes = {119, 122};
  std::vector<int> partyCloudyCodes = {116};
  std::vector<int> clearCodes = {113};
  std::vector<int> fogCodes = {143, 248, 260};
  std::vector<int> rainCodes =
      {176, 293, 296, 299, 302, 305, 308, 311, 314, 353, 356, 359, 386, 389, 263, 266, 281, 284, 185};
  std::vector<int> snowCodes = {179, 227, 323, 326, 329, 332, 335, 338, 368, 371, 392, 395, 230, 350};

private:
  void drawWeather();

public:
  ~WeatherPlugin()
  {
#ifdef ESP32
    if (secureClient != nullptr)
    {
      delete secureClient;
      secureClient = nullptr;
    }
#endif
  }

  void update();
  void setup() override;
  void loop() override;
  void teardown() override;
  const char *getName() const override;
};
