#pragma once

#include <Arduino.h>
#include "mode/mode.h"
#include "screen.h"

class Snake
{
private:
  static const unsigned int LED_TYPE_OFF = 0;
  static const unsigned int LED_TYPE_ON = 1;
  static const unsigned int GAME_STATE_RUNNING = 1;
  static const unsigned int GAME_STATE_END = 2;

  byte gameState;

  std::vector<int> position = {240,241,242};
  unsigned int dot;

  void initGame();
  void initSnake();
  void newDot();
  void findDirection();
  void moveSnake(int newpos);
  void end();

public:
  void setup();
  void loop();
};