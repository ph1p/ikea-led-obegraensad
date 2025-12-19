#include "scheduler.h"
#include "websocket.h"

PluginScheduler &PluginScheduler::getInstance()
{
  static PluginScheduler instance;
  return instance;
}

void PluginScheduler::addItem(int pluginId, unsigned long durationSeconds)
{
  ScheduleItem item = {
      .pluginId = pluginId,
      .duration = durationSeconds * 1000 // Convert to milliseconds
  };
  schedule.push_back(item);
}

void PluginScheduler::clearSchedule(bool emptyStorage)
{
  currentIndex = 0;
  isActive = false;
#ifdef ENABLE_STORAGE
  if (emptyStorage)
  {
    schedule.clear();
    storage.begin("led-wall");
    storage.putString("schedule", "");
    storage.putInt("scheduleactive", 0);
    storage.end();
  }
#else
  if (emptyStorage)
  {
    schedule.clear();
  }
#endif
}

void PluginScheduler::start()
{
  if (!schedule.empty())
  {
    currentIndex = 0;
    lastSwitch = millis();
    isActive = true;
    requestPersist();
    switchToCurrentPlugin();
  }
}

void PluginScheduler::stop()
{
  isActive = false;
  requestPersist();
}

void PluginScheduler::requestPersist()
{
  needsPersist = true;
  lastPersistRequest = millis();
}

void PluginScheduler::checkAndPersist()
{
#ifdef ENABLE_STORAGE
  if (needsPersist && (millis() - lastPersistRequest >= PERSIST_DELAY_MS))
  {
    storage.begin("led-wall", false);
    storage.putInt("scheduleactive", isActive ? 1 : 0);
    storage.end();
    needsPersist = false;
  }
#else
  needsPersist = false;
#endif
}

void PluginScheduler::update()
{
  checkAndPersist();

  if (!isActive || schedule.empty())
    return;

  unsigned long currentTime = millis();
  if (currentTime - lastSwitch >= schedule[currentIndex].duration)
  {
    currentIndex = (currentIndex + 1) % schedule.size();
    lastSwitch = currentTime;
    switchToCurrentPlugin();
  }
}

void PluginScheduler::switchToCurrentPlugin()
{
  if (currentIndex < schedule.size())
  {
    pluginManager.setActivePluginById(schedule[currentIndex].pluginId);
#ifdef ENABLE_SERVER
    sendInfo();
#endif
  }
}

void PluginScheduler::init()
{
#ifdef ENABLE_STORAGE
  storage.begin("led-wall", true);
  int storedActive = storage.getInt("scheduleactive", 0);
  bool scheduleIsSet = setScheduleByJSONString(storage.getString("schedule"));

  isActive = (storedActive == 1);
  storage.end();

  if (isActive && !schedule.empty())
  {
    currentIndex = 0;
    lastSwitch = millis();
    switchToCurrentPlugin();
  }
#endif
}

bool PluginScheduler::setScheduleByJSONString(String scheduleJson)
{
  if (scheduleJson.length() == 0)
  {
    return false;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, scheduleJson);

  if (error)
  {
    return false;
  }

  clearSchedule(true);

#ifdef ENABLE_STORAGE
  storage.begin("led-wall");
  storage.putString("schedule", scheduleJson);
  storage.end();
#endif

  clearSchedule();

  for (const auto &item : doc.as<JsonArray>())
  {
    if (item["pluginId"].is<int>() && item["duration"].is<unsigned long>())
    {
      int pluginId = item["pluginId"].as<int>();
      unsigned long duration = item["duration"].as<unsigned long>();
      addItem(pluginId, duration);
    }
  }
  return true;
}

PluginScheduler &Scheduler = PluginScheduler::getInstance();
