#pragma once

#include "PluginManager.h"
#include "timing.h"

class RainPlugin : public Plugin
{
private:
  NonBlockingDelay timer;
  static constexpr uint8_t NUM_DROPS = 10;
  static constexpr uint8_t X_MAX = 16;
  static constexpr uint8_t Y_MAX = 16;
  static constexpr uint8_t LED_TYPE_OFF = 0;
  static constexpr uint8_t LED_TYPE_ON = 1;
  struct Drop
  {
    unsigned char x;
    unsigned char y;
    bool visible;
  };

  Drop drops[RainPlugin::NUM_DROPS];

public:
  void setup() override;
  void loop() override;
  const char *getName() const override;
};
