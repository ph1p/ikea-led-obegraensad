#pragma once

#include "PluginManager.h"
#include "timing.h"

class AutoWalkerPlugin : public Plugin
{
private:
  static constexpr uint8_t X_MAX = 16;
  static constexpr uint8_t Y_MAX = 16;
  static constexpr uint16_t CELL_COUNT = (uint16_t)X_MAX * Y_MAX;
  static constexpr uint8_t STEP_INTERVAL_MS = 70;
  static constexpr uint16_t HOLD_DURATION_MS = 1200;
  static constexpr uint8_t FADE_INTERVAL_MS = 30;
  static constexpr uint8_t FADE_STEP = 17;
  static constexpr uint8_t BRIGHTNESS_WALKER = 255;
  static constexpr uint8_t BRIGHTNESS_VISIT_1 = 20;
  static constexpr uint8_t BRIGHTNESS_VISIT_2 = 90;
  static constexpr uint8_t BRIGHTNESS_VISIT_MAX = 200;
  static constexpr uint8_t WANDER_ONE_IN = 5;
  static constexpr uint8_t NEIGHBOR_COUNT = 4;

  enum WalkerState : uint8_t
  {
    RUN,
    HOLD,
    FADE
  };

  NonBlockingDelay stepTimer;
  NonBlockingDelay holdTimer;
  NonBlockingDelay fadeTimer;

  WalkerState state = RUN;
  uint8_t counts[Y_MAX][X_MAX] = {};
  uint8_t display[Y_MAX][X_MAX] = {};
  int8_t x = 0;
  int8_t y = 0;
  uint16_t visitedCells = 0;

  void resetRun();
  void pickRandomPosition();
  uint8_t brightnessForCount(uint8_t count) const;
  void step();
  void startHold();
  void fadeStep();
  void render();

public:
  void setup() override;
  void loop() override;
  const char *getName() const override;
};
