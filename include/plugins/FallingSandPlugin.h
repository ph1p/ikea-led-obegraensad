#pragma once

#include "PluginManager.h"
#include "timing.h"

class FallingSandPlugin : public Plugin
{
private:
  static constexpr uint8_t GRID_SIZE = 16;
  static constexpr uint8_t MAX_AMOUNT = 255;
  static constexpr uint8_t TICK_INTERVAL_MS = 50;
  static constexpr uint8_t SPAWN_INTERVAL_MS = 180;
  static constexpr uint8_t DRAIN_INTERVAL_MS = 80;
  static constexpr uint8_t BRIGHTNESS_FULL = 255;
  static constexpr uint8_t SPAWN_COL_START = 7;
  static constexpr uint8_t SPAWN_COL_MIN = 1;
  static constexpr uint8_t SPAWN_COL_MAX = 14;
  static constexpr uint16_t WANDER_MIN_MS = 600;
  static constexpr uint16_t WANDER_MAX_MS = 1200;

  enum SandState : uint8_t
  {
    SPAWNING,
    DRAINING
  };

  NonBlockingDelay tickTimer;
  NonBlockingDelay spawnTimer;
  NonBlockingDelay drainTimer;
  NonBlockingDelay wanderTimer;

  SandState state = SPAWNING;
  uint8_t amount[GRID_SIZE][GRID_SIZE];
  uint8_t spawnCol = SPAWN_COL_START;
  uint16_t wanderIntervalMs = WANDER_MIN_MS;

  void resetGrid();
  void wanderSpawnColumn();
  void spawnSand();
  void applyPhysics();
  void drainStep();
  void render();
  bool topRowOccupied() const;
  uint16_t totalAmount() const;

public:
  void setup() override;
  void loop() override;
  const char *getName() const override;
};
