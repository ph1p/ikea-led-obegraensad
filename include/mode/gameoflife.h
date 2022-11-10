#pragma once

#include <Arduino.h>
#include "mode/mode.h"
#include "screen.h"
#include "constants.h"

class GameOfLife
{
private:
  uint8_t previous[256];
  uint8_t updateCell(int row, int col);
  uint8_t countNeighbours(int row, int col);
  void next();

public:
  void setup();
  void loop();
};
