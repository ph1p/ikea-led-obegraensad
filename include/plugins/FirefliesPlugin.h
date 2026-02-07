#pragma once

#include "PluginManager.h"
#include "timing.h"

class FirefliesPlugin : public Plugin
{
private:
  struct Firefly
  {
    float x;
    float y;
    float vx;
    float vy;
  };

  NonBlockingDelay timer;
  static constexpr uint8_t kFireflyCount = 10;
  Firefly fireflies[kFireflyCount];

public:
  void setup() override;
  void loop() override;
  const char *getName() const override;
};
