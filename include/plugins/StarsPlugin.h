#pragma once

#include "PluginManager.h"

struct Star
{
  int x;
  int y;
  int brightness;
  unsigned long lastUpdateTime;
};
class StarsPlugin : public Plugin
{
private:
  int numStars = 25;
  Star stars[25];

public:
  void setup() override;
  void loop() override;
  void teardown() override;
  const char *getName() const override;
};
