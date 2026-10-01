#pragma once

#include "PluginManager.h"

class AnimationPlugin : public Plugin
{
private:
  // Each frame costs ~150 bytes of heap; cap it so a client cannot exhaust memory.
  static constexpr size_t MAX_FRAMES = 128;

  size_t step = 0;
  std::vector<std::vector<int>> customAnimationFrames;
  int frameDelay = 400;

public:
  void setup() override;
  void loop() override;
  const char *getName() const override;
  void websocketHook(JsonDocument &request) override;
};
