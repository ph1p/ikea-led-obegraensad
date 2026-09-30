#pragma once

#include "PluginManager.h"
#include "timing.h"

class BigPongPlugin : public Plugin
{
private:
  static constexpr uint8_t X_MAX = 16;
  static constexpr uint8_t PADDLE_WIDTH = 4;
  static constexpr uint8_t PADDLE_TOP_Y = 0;
  static constexpr uint8_t PADDLE_BOTTOM_Y = 15;
  static constexpr uint8_t MID_LINE_Y = 7;
  static constexpr uint8_t BRIGHTNESS_FULL = 255;
  static constexpr uint8_t BRIGHTNESS_FAINT = 10;
  static constexpr float BASE_SPEED = 1.05f;
  static constexpr float MAX_SPEED_FACTOR = 1.5f;
  static constexpr float SPEED_UP = 1.05f;
  static constexpr float NUDGE = 0.2f;
  static constexpr uint8_t MOVE_INTERVAL_MS = 75;
  static constexpr uint8_t GOAL_BLINK_INTERVAL_MS = 150;
  static constexpr uint16_t SERVE_DELAY_MS = 500;
  static constexpr uint8_t GOAL_BLINK_TOGGLES = 8;
  static constexpr uint8_t SERVE_ANGLE_MAX_DEG = 25;
  static constexpr uint8_t AIM_ERROR_MAX = 1;
  static constexpr uint16_t PREDICT_MAX_STEPS = 512;

  enum GameState : uint8_t
  {
    RUNNING,
    GOAL_BLINK,
    SERVE_DELAY
  };

  enum Player : uint8_t
  {
    PLAYER_TOP,
    PLAYER_BOTTOM
  };

  NonBlockingDelay moveTimer;
  NonBlockingDelay seqTimer;

  GameState gameState = RUNNING;
  float bx = 7.5f;
  float by = 7.5f;
  float vx = 0.0f;
  float vy = BASE_SPEED;
  int8_t topPaddleX = (X_MAX - PADDLE_WIDTH) / 2;
  int8_t bottomPaddleX = (X_MAX - PADDLE_WIDTH) / 2;
  int8_t topTargetX = (X_MAX - PADDLE_WIDTH) / 2;
  int8_t bottomTargetX = (X_MAX - PADDLE_WIDTH) / 2;
  int8_t aimErrorTop = 0;
  int8_t aimErrorBottom = 0;
  Player scoringPlayer = PLAYER_TOP;
  int8_t serveDirY = 1;
  bool blinkVisible = true;
  uint8_t blinkToggles = 0;

  void tickGame();
  void updateAi();
  int8_t predictLanding(bool top) const;
  void bounceOffPaddle(bool top);
  void scaleBallSpeed();
  void serve(float dirY);
  void startGoal(Player scorer);
  void drawPaddleRow(int8_t x, uint8_t y);
  void render();
  int8_t rollAimError();
  static int8_t clampPaddle(int8_t value);

public:
  void setup() override;
  void loop() override;
  const char *getName() const override;
};
