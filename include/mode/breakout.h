#pragma once

#include <Arduino.h>
#include "mode/mode.h"
#include "screen.h"

class Breakout
{
private:
  static const uint8_t DEBOUNCE_TIME = 100;
  static const uint8_t X_MAX = 16;
  static const uint8_t Y_MAX = 16;
  static const uint8_t BRICK_AMOUNT = X_MAX * 4;
  static const uint8_t BALL_DELAY_MAX = 250;
  static const uint8_t BALL_DELAY_MIN = 100;
  static const uint8_t BALL_DELAY_STEP = 5;
  static const uint8_t PADDLE_WIDTH = 4;
  static const uint8_t DIRECTION_NONE = 0;
  static const uint8_t DIRECTION_LEFT = 1;
  static const uint8_t DIRECTION_RIGHT = 2;
  static const uint8_t LED_TYPE_OFF = 0;
  static const uint8_t LED_TYPE_ON = 1;
  static const uint8_t GAME_STATE_RUNNING = 1;
  static const uint8_t GAME_STATE_END = 2;
  static const uint8_t GAME_STATE_LEVEL = 3;
  struct Coords
  {
    byte x;
    byte y;
  };

  byte gameState;
  byte level;
  byte destroyedBricks;
  Coords paddle[Breakout::PADDLE_WIDTH];
  Coords bricks[Breakout::BRICK_AMOUNT];
  Coords ball;

  int ballMovement[2];
  uint8_t ballDelay;
  uint8_t score;
  unsigned long lastBallUpdate = 0;

  void resetLEDs();
  void initGame();
  void initBricks();
  void newLevel();
  void updateBall();
  void hitBrick(byte i);
  void checkPaddleCollision();
  void updatePaddle();
  void end();

public:
  void setup();
  void loop();
};
