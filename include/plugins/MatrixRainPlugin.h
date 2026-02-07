#pragma once

#include "PluginManager.h"
#include "timing.h"

class MatrixRainPlugin : public Plugin
{
private:
  NonBlockingDelay timer;
  static constexpr uint8_t NUM_COLUMNS = 16;
  static constexpr uint8_t HEIGHT = 16;
  static constexpr uint8_t MAX_TRAIL_LENGTH = 8;
  
  struct Column
  {
    int8_t y;
    uint8_t speed;
    uint8_t length;
    bool active;
  };

  Column columns[NUM_COLUMNS];

public:
  void setup() override;
  void loop() override;
  const char *getName() const override;
};
