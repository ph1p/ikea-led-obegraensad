#pragma once

#ifdef ESP32
#include <HTTPClient.h>
#endif
#ifdef ESP8266
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#endif
#include <ArduinoJson.h>
#include "PluginManager.h"

class WeatherPlugin : public Plugin
{
private:
  unsigned long lastUpdate = 0;
  HTTPClient http;

public:
  void update();
  void setup() override;
  void loop() override;
  const char *getName() const override;
};
