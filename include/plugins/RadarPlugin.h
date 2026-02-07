#pragma once

#include "PluginManager.h"
#include "timing.h"

class RadarPlugin : public Plugin
{
private:
  NonBlockingDelay timer;
  static constexpr uint8_t WIDTH = 16;
  static constexpr uint8_t HEIGHT = 16;
  static constexpr uint8_t CENTER_X = 8;
  static constexpr uint8_t CENTER_Y = 8;
  static constexpr uint8_t NUM_BLIPS = 5;
  
  float sweepAngle;
  float sweepSpeed;
  
  struct Blip
  {
    uint8_t x;
    uint8_t y;
    uint8_t age;
    bool active;
  };
  
  Blip blips[NUM_BLIPS];

public:
  void setup() override;
  void loop() override;
  const char *getName() const override;
};
