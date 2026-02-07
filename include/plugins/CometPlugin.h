#pragma once

#include "PluginManager.h"
#include "timing.h"

class CometPlugin : public Plugin
{
private:
  NonBlockingDelay timer;
  float x = 0.0f;
  float y = 0.0f;
  float vx = 0.8f;
  float vy = 0.5f;
  uint8_t trail[16][16] = {};

  void resetComet();

public:
  void setup() override;
  void loop() override;
  const char *getName() const override;
};
