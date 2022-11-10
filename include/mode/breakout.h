#pragma once

#include <Arduino.h>
#include "mode/mode.h"
#include "led.h"

class Breakout
{
private:
  static const unsigned int DEBOUNCE_TIME = 100;
  static const unsigned int X_MAX = 16;
  static const unsigned int Y_MAX = 16;
  static const unsigned int BRICK_AMOUNT = X_MAX * 4;
  static const unsigned int BALL_DELAY_MAX = 250;
  static const unsigned int BALL_DELAY_MIN = 100;
  static const unsigned int BALL_DELAY_STEP = 5;
  static const unsigned int PADDLE_WIDTH = 4;
  static const unsigned int DIRECTION_NONE = 0;
  static const unsigned int DIRECTION_LEFT = 1;
  static const unsigned int DIRECTION_RIGHT = 2;
  static const unsigned int LED_TYPE_OFF = 0;
  static const unsigned int LED_TYPE_ON = 1;
  static const unsigned int GAME_STATE_RUNNING = 1;
  static const unsigned int GAME_STATE_END = 2;
  static const unsigned int GAME_STATE_LEVEL = 3;
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
  unsigned int ballDelay;
  unsigned int score;
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