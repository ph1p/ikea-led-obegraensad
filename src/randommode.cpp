#include "randommode.h"
#include "scheduler.h"
#include "storage.h"
#include "websocket.h"

// Plugins that are not animations and therefore never picked by random mode.
// Additionally, every plugin whose name ends with "Clock" is excluded.
static const char *const EXCLUDED_PLUGINS[] = {"Draw", "DDP", "ArtNet", "Animation", "Weather"};

RandomModeClass &RandomModeClass::getInstance()
{
  static RandomModeClass instance;
  return instance;
}

bool RandomModeClass::isEligible(Plugin *plugin) const
{
  const char *name = plugin->getName();

  for (const char *excluded : EXCLUDED_PLUGINS)
  {
    if (strcmp(name, excluded) == 0)
    {
      return false;
    }
  }

  const size_t nameLength = strlen(name);
  const size_t suffixLength = strlen("Clock");
  if (nameLength >= suffixLength && strcmp(name + nameLength - suffixLength, "Clock") == 0)
  {
    return false;
  }

  return true;
}

void RandomModeClass::switchToRandomPlugin()
{
  Plugin *activePlugin = pluginManager.getActivePlugin();
  std::vector<int> candidates;

  for (Plugin *plugin : pluginManager.getAllPlugins())
  {
    if (plugin != activePlugin && isEligible(plugin))
    {
      candidates.push_back(plugin->getId());
    }
  }

  if (candidates.empty())
  {
    return;
  }

  pluginManager.setActivePluginById(candidates[random(candidates.size())]);
#ifdef ENABLE_SERVER
  sendInfo();
#endif
}

void RandomModeClass::persist()
{
#ifdef ENABLE_STORAGE
  storage.begin("led-wall", false);
  storage.putInt("random-active", isActive ? 1 : 0);
  storage.putInt("random-interval", intervalMinutes);
  storage.end();
#endif
}

void RandomModeClass::init()
{
#ifdef ENABLE_STORAGE
  storage.begin("led-wall", true);
  isActive = storage.getInt("random-active", 0) == 1;
  int storedInterval = storage.getInt("random-interval", DEFAULT_INTERVAL_MINUTES);
  storage.end();

  intervalMinutes = constrain(storedInterval, MIN_INTERVAL_MINUTES, MAX_INTERVAL_MINUTES);
#endif

  if (isActive)
  {
    lastSwitch = millis();
    switchToRandomPlugin();
  }
}

void RandomModeClass::update()
{
  if (!isActive)
  {
    return;
  }

  if (millis() - lastSwitch >= (unsigned long)intervalMinutes * 60000UL)
  {
    lastSwitch = millis();
    switchToRandomPlugin();
  }
}

void RandomModeClass::setActive(bool active)
{
  if (active == isActive)
  {
    return;
  }

  isActive = active;
  persist();

  if (isActive)
  {
    // random mode and the scheduler are mutually exclusive
    if (Scheduler.isActive)
    {
      Scheduler.stop();
    }
    lastSwitch = millis();
    switchToRandomPlugin();
  }
}

void RandomModeClass::setInterval(int minutes)
{
  uint8_t newInterval = constrain(minutes, MIN_INTERVAL_MINUTES, MAX_INTERVAL_MINUTES);
  if (newInterval == intervalMinutes)
  {
    return;
  }

  intervalMinutes = newInterval;
  lastSwitch = millis();
  persist();
}

RandomModeClass &RandomMode = RandomModeClass::getInstance();
