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

class WeatherPlugin : public Plugin
{
private:
  unsigned long lastUpdate = 0;
  HTTPClient http;

  std::vector<int> thunderCodes = {200, 386, 389, 392, 395};
  std::vector<int> cloudyCodes = {119, 122};
  std::vector<int> partyCloudyCodes = {116};
  std::vector<int> clearCodes = {113};
  std::vector<int> fogCodes = {143, 248, 260};
  std::vector<int> rainCodes = {
      176, 293, 296, 299, 302,
      305, 308, 311, 314, 353,
      356, 359, 386, 389, 263,
      266, 281, 284, 185};
  std::vector<int> snowCodes = {
      179, 227, 323, 326, 329,
      332, 335, 338, 368, 371,
      392, 395, 230, 350};

public:
  void update();
  void setup() override;
  void loop() override;
  const char *getName() const override;
};
