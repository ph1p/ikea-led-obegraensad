#pragma once

#include "PluginManager.h"
#include <vector>
#include <Arduino.h>

struct ScheduleItem
{
  int pluginId;
  unsigned long duration; // Duration in milliseconds
};

class PluginScheduler
{
private:
  PluginScheduler() = default;
  unsigned long lastSwitch = 0;
  size_t currentIndex = 0;

public:
  static PluginScheduler &getInstance();

  PluginScheduler(const PluginScheduler &) = delete;
  PluginScheduler &operator=(const PluginScheduler &) = delete;

  bool isActive = false;
  std::vector<ScheduleItem> schedule;

  void addItem(int pluginId, unsigned long durationSeconds);
  void clearSchedule();
  void start();
  void stop();
  void update();
  void reset();

private:
  void switchToCurrentPlugin();
};

extern PluginScheduler &Scheduler;