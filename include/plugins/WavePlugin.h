#pragma once

#include "PluginManager.h"
#include "timing.h"

class WavePlugin : public Plugin
{
private:
  NonBlockingDelay timer;
  static constexpr uint8_t WIDTH = 16;
  static constexpr uint8_t HEIGHT = 16;
  
  float phase;
  float waveSpeed;

public:
  void setup() override;
  void loop() override;
  const char *getName() const override;
};
