#pragma once

#include "PluginManager.h"

class BigClockPlugin : public Plugin
{
private:
  struct tm timeinfo;

  int previousMinutes;
  int previousHour;
  std::vector<int> previousHH;
  std::vector<int> previousMM;
  bool previousLeadingZero;

public:
  void setup() override;
  void loop() override;
  const char *getName() const override;
};
