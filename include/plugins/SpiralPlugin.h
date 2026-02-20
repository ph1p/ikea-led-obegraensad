#pragma once

#include "PluginManager.h"
#include "timing.h"

class SpiralPlugin : public Plugin
{
private:
  NonBlockingDelay timer;
  static constexpr uint8_t WIDTH = 16;
  static constexpr uint8_t HEIGHT = 16;
  static constexpr uint8_t CENTER_X = 8;
  static constexpr uint8_t CENTER_Y = 8;
  
  float angle;
  float radius;
  bool expanding;

public:
  void setup() override;
  void loop() override;
  const char *getName() const override;
};
