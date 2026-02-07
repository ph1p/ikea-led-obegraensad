#pragma once

#include "PluginManager.h"
#include "timing.h"

class CheckerboardPlugin : public Plugin
{
private:
  NonBlockingDelay timer;
  static constexpr uint8_t WIDTH = 16;
  static constexpr uint8_t HEIGHT = 16;
  static constexpr uint8_t SQUARE_SIZE = 2;
  
  uint8_t offset;
  bool inverted;
  uint8_t phase;

public:
  void setup() override;
  void loop() override;
  const char *getName() const override;
};
