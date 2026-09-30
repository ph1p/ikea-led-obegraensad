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
  unsigned long updateInterval = 1000UL * 60 * 30;
  HTTPClient http;
#ifdef ESP32
  WiFiClientSecure *secureClient = nullptr;
#endif
#ifdef ESP8266
  WiFiClient wiFiClient;
#endif

  // Coordinates the forecast endpoint needs, looked up once per configured city
  String resolvedLocation;
  float latitude = 0;
  float longitude = 0;

  // Cached weather data
  bool hasCachedData = false;
  int cachedTemperature = 0;
  int cachedWeatherIcon = 0;
  int cachedIconY = 1;
  int cachedTempY = 10;

  // WMO weather codes as returned by Open-Meteo
  std::vector<int> thunderCodes = {95, 96, 99};
  std::vector<int> cloudyCodes = {3};
  std::vector<int> partyCloudyCodes = {2};
  std::vector<int> clearCodes = {0, 1};
  std::vector<int> fogCodes = {45, 48};
  std::vector<int> rainCodes = {51, 53, 55, 56, 57, 61, 63, 65, 66, 67, 80, 81, 82};
  std::vector<int> snowCodes = {71, 73, 75, 77, 85, 86};

private:
  void drawWeather();
  void drawError();
  bool resolveLocation(const String &location);
  bool fetchJson(const String &url, JsonDocument &doc);

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

  bool update();
  void setup() override;
  void loop() override;
  void teardown() override;
  const char *getName() const override;
};
