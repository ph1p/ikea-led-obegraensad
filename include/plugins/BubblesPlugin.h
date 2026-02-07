#pragma once

#include "PluginManager.h"
#include "timing.h"

class BubblesPlugin : public Plugin
{
private:
  struct Bubble
  {
    float x;
    float y;
    float radius;
    float speed;
    uint8_t brightness;
  };

  NonBlockingDelay timer;
  static constexpr uint8_t kBubbleCount = 6;
  Bubble bubbles[kBubbleCount];

  void resetBubble(Bubble &bubble);

public:
  void setup() override;
  void loop() override;
  const char *getName() const override;
};
