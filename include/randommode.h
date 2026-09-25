#pragma once

#include "PluginManager.h"
#include <Arduino.h>

class RandomModeClass
{
private:
  RandomModeClass() = default;
  unsigned long lastSwitch = 0;

  bool isEligible(Plugin *plugin) const;
  void switchToRandomPlugin();
  void persist();

public:
  static constexpr uint8_t MIN_INTERVAL_MINUTES = 1;
  static constexpr uint8_t MAX_INTERVAL_MINUTES = 60;
  static constexpr uint8_t DEFAULT_INTERVAL_MINUTES = 5;

  static RandomModeClass &getInstance();

  RandomModeClass(const RandomModeClass &) = delete;
  RandomModeClass &operator=(const RandomModeClass &) = delete;

  bool isActive = false;
  uint8_t intervalMinutes = DEFAULT_INTERVAL_MINUTES;

  void init();
  void update();
  void setActive(bool active);
  void setInterval(int minutes);
};

extern RandomModeClass &RandomMode;
