#pragma once

#include "PluginManager.h"

#include "ArtnetWifi.h"

#define START_UNIVERSE 1

class ArtNetPlugin : public Plugin
{
private:
  ArtnetWifi artnet;

public:
  void setup() override;
  void teardown() override;
  void loop() override;
  const char* getName() const override;
  static void onDmxFrame(uint16_t universe, uint16_t length, uint16_t outgoing, uint8_t* data);
  void websocketHook(JsonDocument& request) override;
};
