#pragma once

#include "PluginManager.h"
#include "timing.h"

class GameOfLifePlugin : public Plugin
{
private:
  static const uint8_t STATE_RUNNING = 1;
  static const uint8_t STATE_END = 2;
  static const uint8_t STATE_INIT = 3;
  static const uint8_t STATE_END_DELAY = 4;
  uint8_t state;
  uint8_t previous2[ROWS * COLS];
  uint8_t previous[ROWS * COLS];
  uint8_t buffer[ROWS * COLS];
  uint8_t updateCell(int row, int col);
  uint8_t countNeighbours(int row, int col);
  void next();
  void init();
  void show();
  uint16_t gol_delay = 150;

  NonBlockingDelay updateTimer;
  NonBlockingDelay initTimer;
  uint8_t initStep = 0;

public:
  void setup() override;
  void loop() override;
  const char *getName() const override;
  void websocketHook(DynamicJsonDocument &request) override;
};
