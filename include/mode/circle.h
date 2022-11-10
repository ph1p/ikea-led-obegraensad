#pragma once

#include <Arduino.h>
#include <map>
#include "mode/mode.h"

class Circle
{
private:
  std::map<String, uint8_t[ROWS * COLS]> memo;
  void drawCircle(int x, int y, int radius, int onOff);
  uint8_t circleStep = 0;
  uint8_t innerCircleStep = 0;
  bool innerStarted = false;
  int everyN = 1;

public:
  void loop();
};