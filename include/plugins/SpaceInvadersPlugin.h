#pragma once

#include "PluginManager.h"
#include "timing.h"

class SpaceInvadersPlugin : public Plugin
{
private:
  static constexpr uint8_t X_MAX = 16;
  static constexpr uint8_t ENEMY_ROWS = 3;
  static constexpr uint8_t ENEMY_COLS = 5;
  static constexpr uint8_t BASE_ROW_Y = 1;
  static constexpr uint8_t ROW_SPACING = 2;
  static constexpr uint8_t COL_START_X = 1;
  static constexpr uint8_t COL_SPACING = 3;
  static constexpr uint8_t SHIP_Y = 15;
  static constexpr uint8_t SHIP_START_X = (X_MAX - 1) / 2;
  static constexpr uint8_t BULLET_SPAWN_Y = SHIP_Y - 2;
  static constexpr uint8_t GAME_OVER_Y = 14;

  static constexpr uint8_t BRIGHTNESS_FULL = 255;
  static constexpr uint8_t BRIGHTNESS_HIGH = 191;
  static constexpr uint8_t BRIGHTNESS_MID = 150;

  static constexpr uint16_t MARCH_INTERVAL_MS = 900;
  static constexpr uint8_t MOVE_INTERVAL_MS = 100;
  static constexpr uint8_t BULLET_INTERVAL_MS = 60;
  static constexpr uint16_t WAVE_PAUSE_MS = 800;
  static constexpr uint8_t BLINK_INTERVAL_MS = 200;
  static constexpr uint8_t BLINK_TOGGLES = 8;

  enum GameState : uint8_t
  {
    RUNNING,
    GAME_OVER_BLINK,
    WAVE_DELAY
  };

  NonBlockingDelay marchTimer;
  NonBlockingDelay moveTimer;
  NonBlockingDelay bulletTimer;
  NonBlockingDelay seqTimer;

  GameState gameState = RUNNING;
  bool alive[ENEMY_ROWS][ENEMY_COLS];
  int8_t colOffset = 0;
  int8_t rowOffset = 0;
  int8_t marchDir = 1;
  uint16_t waveIntervalMs = MARCH_INTERVAL_MS;
  uint8_t enemiesAlive = 0;
  int8_t shipX = SHIP_START_X;
  int8_t targetX = SHIP_START_X;
  bool bulletActive = false;
  int8_t bulletX = 0;
  int8_t bulletY = 0;
  uint8_t wave = 1;
  bool blinkVisible = true;
  uint8_t blinkToggles = 0;

  void spawnWave();
  void updateFormation();
  void updateShip();
  void updateBullet();
  void startWaveClear();
  void startGameOver();
  int8_t enemyX(uint8_t col) const;
  int8_t enemyY(uint8_t row) const;
  void render();

public:
  void setup() override;
  void loop() override;
  const char *getName() const override;
};
