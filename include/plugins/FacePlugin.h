#pragma once

#include "PluginManager.h"
#include "timing.h"

class FacePlugin : public Plugin
{
private:
  static constexpr uint8_t BRIGHT_FULL = 255;
  static constexpr uint8_t BRIGHT_MID = 140;
  static constexpr uint8_t TEAR_MAX = 4;

  enum Expression : uint8_t
  {
    EXPR_NEUTRAL,
    EXPR_LAUGH,
    EXPR_CRYING,
    EXPR_SHOCKED,
    EXPR_EXCITED,
    EXPR_COUNT
  };

  enum IdleMouth : uint8_t
  {
    MOUTH_LINE,
    MOUTH_SMILE,
    MOUTH_OPEN_SMALL,
    IDLE_MOUTH_COUNT
  };

  struct Tear
  {
    int8_t x;
    int8_t y;
  };

  NonBlockingDelay gazeTimer;
  NonBlockingDelay easeTimer;
  NonBlockingDelay blinkTimer;
  NonBlockingDelay blinkEndTimer;
  NonBlockingDelay exprTimer;
  NonBlockingDelay exprHoldTimer;
  NonBlockingDelay mouthTimer;
  NonBlockingDelay tearSpawnTimer;
  NonBlockingDelay tearFallTimer;

  int8_t gx = 0;
  int8_t gy = 0;
  int8_t targetGx = 0;
  int8_t targetGy = 0;
  unsigned long gazeInterval = 1500;
  unsigned long blinkInterval = 4000;
  unsigned long exprInterval = 6000;
  unsigned long holdInterval = 2000;
  unsigned long mouthInterval = 4000;
  bool blinking = false;
  bool gazeMoving = false;
  bool inExpression = false;
  bool tearLeftNext = true;
  Expression expression = EXPR_NEUTRAL;
  Expression lastExpression = EXPR_NEUTRAL;
  IdleMouth idleMouth = MOUTH_LINE;
  IdleMouth lastIdleMouth = MOUTH_LINE;
  Tear tears[FacePlugin::TEAR_MAX] = {};

  void updateGaze(bool allow);
  void updateBlink(bool allow);
  void updateExpression();
  void updateIdleMouth(bool allow);
  void updateTears(bool crying);
  void clearTears();
  void render();
  void drawEyes();
  void drawMouth();
  void drawTears();
  void drawBlock(int x, int y);
  void drawOutline4x4(int x, int y);
  void drawPlus(int cx, int cy);
  void setPx(int x, int y, uint8_t brightness);

public:
  void setup() override;
  void loop() override;
  const char *getName() const override;
};
