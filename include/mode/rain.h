#pragma once

#include "mode/mode.h"

class Rain
{
private:
  static const uint8_t NUM_DROPS = 10;
  static const uint8_t X_MAX = 16;
  static const uint8_t Y_MAX = 16;
  static const uint8_t LED_TYPE_OFF = 0;
  static const uint8_t LED_TYPE_ON = 1;
  struct Drop
  {
    byte x;
    byte y;
    bool visible;
  };

  Drop drops[Rain::NUM_DROPS];

public:
  void loop();
  void setup();
};
