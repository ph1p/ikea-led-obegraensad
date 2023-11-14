#pragma once

#include "PluginManager.h"

class FireworkPlugin : public Plugin
{
private:
  unsigned long previousMillis = 0;
  const long explosionDelay = 60;
  const long fadeDelay = 24;
  const long rocketDelay = 60;
  const long explosionDuration = 500;

  void drawExplosion(int x, int y, int maxRadius, int brightness);
  void explode(int x, int y);

public:
  void setup() override;
  void loop() override;
  const char *getName() const override;
};
