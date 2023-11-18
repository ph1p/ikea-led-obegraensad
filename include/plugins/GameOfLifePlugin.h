#pragma once

#include "PluginManager.h"

class GameOfLifePlugin : public Plugin
{
private:
  uint8_t previous[ROWS * COLS];
  uint8_t buffer[ROWS * COLS];
  uint8_t updateCell(int row, int col);
  uint8_t countNeighbours(int row, int col);
  void next();

public:
  void setup() override;
  void loop() override;
  const char *getName() const override;
};
