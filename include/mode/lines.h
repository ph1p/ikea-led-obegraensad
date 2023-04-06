#pragma once

#include <Arduino.h>
#include "mode/mode.h"
class Lines
{
private:
  uint8_t count = 0;
  std::vector<std::vector<int>> frames = {
      {0xcc, 0xcc},
      {0x66, 0x66},
      {0x33, 0x33},
      {0x99, 0x99}};

public:
  void loop();
  void setup();
};
