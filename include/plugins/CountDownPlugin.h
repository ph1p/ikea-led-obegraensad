#pragma once

#include "PluginManager.h"

class CountDownPlugin : public Plugin
{
private:
  struct tm localTimeInfo;
  struct tm targetTimeInfo;

  double previousSeconds;

public:
  void setup() override;
  void loop() override;
  const char *getName() const override;
};