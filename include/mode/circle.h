#pragma once

#include <Arduino.h>
#include <map>
#include "mode/mode.h"
#include "signs.h"

class Circle
{
private:
  uint8_t circleStep = 0;

public:
  void loop();
  void setup();
};