#pragma once

#include "PluginManager.h"
#include "timing.h"

class LinesPlugin : public Plugin
{
private:
  uint8_t count = 0;
  NonBlockingDelay timer;

public:
  void setup() override;
  void loop() override;
  const char *getName() const override;
};
