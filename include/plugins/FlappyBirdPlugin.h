#pragma once

#include "PluginManager.h"
#include "timing.h"

class FlappyBirdPlugin : public Plugin
{
private:
  static constexpr uint8_t X_MAX = 16;
  static constexpr uint8_t Y_MAX = 16;
  static constexpr uint8_t BIRD_X = 5;
  static constexpr float BIRD_START_Y = 7.5f;
  static constexpr float CEILING_Y = 0.0f;
  static constexpr float FLOOR_Y = 15.0f;
  static constexpr uint8_t GAP_HEIGHT = 5;
  static constexpr uint8_t GAP_TOP_MIN = 2;
  static constexpr uint8_t GAP_TOP_MAX = 9;
  static constexpr uint8_t PIPE_MAX = 8;
  static constexpr int8_t SPAWN_X = 15;
  static constexpr int8_t SPAWN_THRESHOLD_X = 7;
  static constexpr uint8_t BRIGHTNESS_FULL = 255;
  static constexpr uint8_t BRIGHTNESS_HIGH = 191;
  static constexpr float GRAVITY = 0.22f;
  static constexpr float FLAP_VY = -1.3f;
  static constexpr float VY_MIN = -2.2f;
  static constexpr float VY_MAX = 2.2f;
  static constexpr float BAND_UPPER_OFFSET = 1.6f;
  static constexpr float BAND_LOWER_OFFSET = 4.5f;
  static constexpr float RISE_SUPPRESS_VY = -0.3f;
  static constexpr float PIPE_FLOOR_RISK_Y = 14.0f;
  static constexpr float CRUISE_FLOOR_RISK_Y = 14.5f;
  static constexpr float CRUISE_FLAP_Y = 9.5f;
  static constexpr float CRUISE_HOLD_Y = 8.0f;
  static constexpr float CRUISE_FALL_VY = 1.0f;
  static constexpr int8_t NEAR_DX_MAX = 2;
  static constexpr uint8_t TICK_MS = 70;
  static constexpr uint8_t PIPE_TICK_MS = 180;
  static constexpr uint8_t BLINK_INTERVAL_MS = 200;
  static constexpr uint8_t BLINK_TOGGLES = 6;
  static constexpr uint16_t CRASH_PAUSE_MS = 900;
  static constexpr uint8_t PREDICT_TICKS = 5;

  enum GameState : uint8_t
  {
    RUNNING,
    CRASH_BLINK,
    CRASH_FALL,
    CRASH_PAUSE
  };

  struct Pipe
  {
    int8_t x;
    uint8_t gy;
  };

  NonBlockingDelay physicsTimer;
  NonBlockingDelay pipeTimer;
  NonBlockingDelay seqTimer;

  GameState gameState = RUNNING;
  Pipe pipes[PIPE_MAX];
  uint8_t pipeCount = 0;
  float by = BIRD_START_Y;
  float vy = 0.0f;
  bool blinkVisible = true;
  uint8_t blinkToggles = 0;

  void resetGame();
  void spawnPipe();
  void tickPipes();
  void tickPhysics();
  void updateAi();
  const Pipe *findControllingPipe() const;
  bool birdHitsPipe() const;
  void startCrash();
  void drawPipe(const Pipe &pipe);
  void render();
  static bool isSolidRow(const Pipe &pipe, int y);
  static float clampVy(float value);

public:
  void setup() override;
  void loop() override;
  const char *getName() const override;
};
