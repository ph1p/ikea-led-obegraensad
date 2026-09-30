#pragma once

#include "PluginManager.h"
#include "timing.h"

class BouncingBallPlugin : public Plugin
{
private:
  static constexpr uint8_t OBSTACLE_MAX = 14;
  static constexpr uint8_t GRID_WIDTH = 16;
  static constexpr uint8_t GRID_HEIGHT = 16;
  static constexpr uint8_t PHYSICS_INTERVAL_MS = 65;
  static constexpr uint8_t FADE_INTERVAL_MS = 40;
  static constexpr uint8_t SUBSTEPS = 3;
  static constexpr float GRAVITY = 0.06f;
  static constexpr float BOOSTED_GRAVITY = 0.10f;
  static constexpr unsigned long GRAVITY_BOOST_AFTER_MS = 8000;
  static constexpr float BALL_RADIUS = 0.5f;
  static constexpr float JITTER = 0.08f;
  static constexpr uint8_t MID_BRIGHTNESS = 90;
  static constexpr uint8_t FULL_BRIGHTNESS = 255;
  static constexpr uint8_t FADE_STEP = 35;
  static constexpr unsigned long LEVEL_TIMEOUT_MS = 20000;

  struct Obstacle
  {
    uint8_t x;
    uint8_t y;
    uint8_t len;
  };

  enum class GameState
  {
    Run,
    LevelEnd
  };

  NonBlockingDelay physicsTimer;
  NonBlockingDelay fadeTimer;
  GameState gameState = GameState::Run;
  float posX = 0.0f;
  float posY = 0.0f;
  float velX = 0.0f;
  float velY = 0.0f;
  Obstacle obstacles[BouncingBallPlugin::OBSTACLE_MAX] = {};
  uint8_t obstacleCount = 0;
  uint8_t obstacleBrightness = BouncingBallPlugin::MID_BRIGHTNESS;
  unsigned long levelStartTime = 0;

  void generateLevel();
  void spawnBall();
  void runPhysicsTick();
  void handleObstacleCollisions(uint8_t cellX, uint8_t cellY);
  void render();

public:
  void setup() override;
  void loop() override;
  const char *getName() const override;
};
