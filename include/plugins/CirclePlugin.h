#pragma once

#include "PluginManager.h"
#include "timing.h"

class CirclePlugin : public Plugin
{
private:
  uint8_t circleStep = 0;
  NonBlockingDelay timer;

public:
  void setup() override;
  void loop() override;
  const char *getName() const override;
};
