#pragma once

#ifdef ESP32
#include <HTTPClient.h>
#endif
#ifdef ESP8266
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#endif
#include <ArduinoJson.h>
#include "PluginManager.h"
#include "secrets.h"

class WeatherPlugin : public Plugin
{
private:
  long lastUpdate = 0;
  bool tick = true;
  bool weatherSettingsLoaded = false;
  int weatherTime; //number of hours or days in the future
  int weatherMode; //0 for current, 1 for hourly, 2 for daily
  HTTPClient http;

  std::vector<int> thunderCodes = {95,96,99};
  std::vector<int> cloudyCodes = {3};
  std::vector<int> partyCloudyCodes = {1,2};
  std::vector<int> clearCodes = {0};
  std::vector<int> fogCodes = {45,48};
  std::vector<int> rainCodes = {51,53,55,56,57,61,63,65,66,67,80,81,82};
  std::vector<int> snowCodes = {71,73,75,77,85,86};

public:
  void mqttHook(String topic, int message);
  void getUpdate();
  void parseUpdate();
  void setup() override;
  void loop() override;
  const char *getName() const override;
  void websocketHook(DynamicJsonDocument &request) override;
};
