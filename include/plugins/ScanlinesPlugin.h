#pragma once

#include "PluginManager.h"
#include "timing.h"

class ScanlinesPlugin : public Plugin
{
private:
  NonBlockingDelay timer;
  float position = 0.0f;
  float speed = 0.4f;

public:
  void setup() override;
  void loop() override;
  const char *getName() const override;
};
