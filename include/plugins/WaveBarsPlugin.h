#pragma once

#include "PluginManager.h"
#include "timing.h"

class WaveBarsPlugin : public Plugin
{
private:
  NonBlockingDelay timer;
  float phase = 0.0f;

public:
  void setup() override;
  void loop() override;
  const char *getName() const override;
};
