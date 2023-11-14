#pragma once

#include "PluginManager.h"

class FireworkPlugin : public Plugin
{
private:
  void drawExplosion(int x, int y, int maxRadius, int brightness);
  void explode(int x, int y);

public:
  void setup() override;
  void loop() override;
  const char *getName() const override;
};
