#pragma once

#include "PluginManager.h"
#include "timing.h"

class SparkleFieldPlugin : public Plugin
{
private:
  NonBlockingDelay timer;
  uint8_t sparkles[16][16] = {};

public:
  void setup() override;
  void loop() override;
  const char *getName() const override;
};
