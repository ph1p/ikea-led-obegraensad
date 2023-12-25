#pragma once

#include "PluginManager.h"

class PongClockPlugin : public Plugin
{
private:
  static const uint8_t X_MAX = 16;
  static const uint8_t Y_MAX = 16;
  static const uint8_t LED_TYPE_OFF = 0;
  static const uint8_t LED_TYPE_ON = 1;
  static const uint8_t SPEED = 100;

  struct tm timeinfo;

  uint8_t
  paddleIndex = 230,
  ballX = 112,
  ballY = 208,
  ballIndex = 216,
  ballBrightness = 255,
  ballBrightnessStep = -20,
  
  pongPaddleLeftY = 0,
  pongPaddleRightY = 0,
  pongPaddleLeftStart = 0,
  pongPaddleRightStart = 0,
  pongPaddleLeftTarget = 0,
  pongPaddleRightTarget = 0;

  bool
  pongScoredHour = false,
  pongScoredMinute = false,
  pongCelebrate = false,
  pongclock = false, // pong clock mode
  pongReset = false,
  pongBallDirection = 0;

  unsigned long
  pongCelebrationEnd = 0,
  pongShowtime = 0,
  nextUpdateMillis = 0;

  int
  previousMinutes,
  previousHour,
  ballAngle = 0,
  fadeLength = 0,
  current_minute = 0,
  current_hour = 0;


  void drawCharacter(int x, int y, std::vector<int> bits, int bitCount, uint8_t brightness = 255);
  byte getScreenIndex(byte x, byte y);
  void swapXdirection();
  void swapYdirection();
  float degToRad(float deg);
  int pong_predict_y(int x, int y, int angle);
  long realRandom(int max);
  int realRandom(int min, int max);
  void reset();
  void drawDigits();


public:
  void setup() override;
  void loop() override;
  const char *getName() const override;
};