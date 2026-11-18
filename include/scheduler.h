#pragma once

#include "PluginManager.h"
#include <Arduino.h>
#include <vector>

#ifdef ESP32
#include <mutex>
#endif

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

  bool needsPersist = false;
  unsigned long lastPersistRequest = 0;
  static constexpr unsigned long PERSIST_DELAY_MS = 2000;

  // Written by the web server task, read by the screen task and sendInfo()
  std::vector<ScheduleItem> schedule;
#ifdef ESP32
  mutable std::recursive_mutex scheduleMutex;
#endif

public:
  static PluginScheduler &getInstance();

  PluginScheduler(const PluginScheduler &) = delete;
  PluginScheduler &operator=(const PluginScheduler &) = delete;

  bool isActive = false;

  std::vector<ScheduleItem> getSchedule() const;
  bool hasSchedule() const;

  void addItem(int pluginId, unsigned long durationSeconds);
  void clearSchedule(bool emptyStorage = false);
  void start();
  void stop();
  void update();
  void init();
  bool setScheduleByJSONString(String scheduleJson);

private:
  void switchToCurrentPlugin();
  void requestPersist();
  void checkAndPersist();
};

extern PluginScheduler &Scheduler;