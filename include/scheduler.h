#pragma once

#include "PluginManager.h"
#include <Arduino.h>
#include <vector>

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
  void clearSchedule(bool emptyStorage = false);
  void start();
  void stop();
  void update();
  void init();
  bool setScheduleByJSONString(String scheduleJson);

private:
  void switchToCurrentPlugin();
};

extern PluginScheduler &Scheduler;