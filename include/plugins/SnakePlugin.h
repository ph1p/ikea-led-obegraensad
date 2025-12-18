#pragma once

#include "PluginManager.h"
#include "timing.h"

class SnakePlugin : public Plugin
{
private:
  static constexpr uint8_t LED_TYPE_OFF = 0;
  static constexpr uint8_t LED_TYPE_ON = 1;
  static constexpr uint8_t GAME_STATE_RUNNING = 1;
  static constexpr uint8_t GAME_STATE_END = 2;
  static constexpr uint8_t GAME_STATE_DEATH_ANIMATION = 3;

  static constexpr uint16_t SNAKE_DELAY_MS = 100;
  static constexpr uint16_t BLINK_SHORT_MS = 200;
  static constexpr uint16_t BLINK_LONG_MS = 500;

  unsigned char gameState;
  unsigned char lastDirection = 0; // 0=unset 1=up 2=right 3=down 4 =left

  std::vector<uint> position = {240, 241, 242};

  uint8_t dot;

  NonBlockingDelay moveTimer;
  NonBlockingDelay animationTimer;
  uint8_t animationStep = 0;

  void initGame();
  void newDot();
  void findDirection();
  void moveSnake(uint newpos);
  void end();
  void updateDeathAnimation();

public:
  void setup() override;
  void loop() override;
  const char *getName() const override;
};
