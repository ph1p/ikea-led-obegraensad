#pragma once

#include "PluginManager.h"
#include "timing.h"
#include <cmath>
#include <cstdlib>

class BlobPlugin : public Plugin
{
public:
  static constexpr float aspect_ratio = 1.5f;
  static constexpr uint8_t X_MAX = 16;
  static constexpr uint8_t Y_MAX = static_cast<uint8_t>(X_MAX * aspect_ratio);

  static constexpr uint8_t NUM_BALLS = 5;

  BlobPlugin();
  virtual ~BlobPlugin()
  {
  }

  void setup() override;
  void loop() override;
  const char* getName() const override;

private:
  struct Ball
  {
    float x, y;
    float vx, vy;
  };

  Ball balls[NUM_BALLS];

  static constexpr float RADIUS = 7.0f;
  static constexpr float RADIUS_SQ = RADIUS * RADIUS;
  static constexpr float SPEED = 0.2f;
  static constexpr float CAP_VALUE = 3.0f;
  static constexpr float GAMMA = 0.7f;
  static constexpr uint16_t UPDATE_INTERVAL_MS = 50;

  uint8_t previousBrightness[ROWS * COLS];
  NonBlockingDelay updateTimer;

  float attenuationSquared(float d_sq, float radius_sq) const;
  uint8_t toneMap(float v) const;
  void updatePositions();
  void render();
};
