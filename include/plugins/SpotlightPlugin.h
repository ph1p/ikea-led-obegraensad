#pragma once

#include "PluginManager.h"
#include "timing.h"

class SpotlightPlugin : public Plugin
{
private:
  static constexpr uint8_t X_MAX = 16;
  static constexpr uint8_t Y_MAX = 16;
  static constexpr float RADIUS = 4.5f;
  static constexpr uint8_t FRAME_INTERVAL_MS = 45;
  static constexpr uint8_t BRIGHTNESS_FULL = 255;
  static constexpr uint8_t POS_MIN = 4;
  static constexpr uint8_t POS_MAX = 11;

  NonBlockingDelay frameTimer;

  float hx = 7.5f;
  float hy = 7.5f;
  float vx = 0.32f;
  float vy = 0.21f;

  void rollVelocity();
  void updatePosition();
  void render();

public:
  void setup() override;
  void loop() override;
  const char *getName() const override;
};
