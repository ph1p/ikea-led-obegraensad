#pragma once

#include "PluginManager.h"
#include "timing.h"

class MeteorShowerPlugin : public Plugin
{
private:
  struct Meteor
  {
    float x;
    float y;
    float vx;
    float vy;
  };

  NonBlockingDelay timer;
  static constexpr uint8_t kMeteorCount = 6;
  Meteor meteors[kMeteorCount];

  void resetMeteor(Meteor &meteor);

public:
  void setup() override;
  void loop() override;
  const char *getName() const override;
};
