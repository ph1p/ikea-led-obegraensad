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
  byte lastDirection = 0; // 0=unset 1=up 2=right 3=down 4 =left

  std::vector<uint> position = {240,241,242};

  unsigned int dot;

  void initGame();
  void newDot();
  void findDirection();
  void moveSnake(uint newpos);
  void end();

public:
  void setup();
  void loop();
};
