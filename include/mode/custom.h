#pragma once

#include <Arduino.h>
#include "mode/mode.h"

extern std::vector<std::vector<int>> customAnimationFrames;

class Custom
{
private:
  uint8_t step = 0;

public:
  void loop();
  void setup();
};