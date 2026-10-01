#include "PluginManager.h"
#include "scheduler.h"
#ifdef ESP32
#include <mutex>

// plugins are switched from the web, button and scheduler tasks while the
// screen task runs loop(), so never tear down a plugin in the middle of it
static std::recursive_mutex pluginLock;
#define PLUGIN_GUARD std::lock_guard<std::recursive_mutex> guard(pluginLock)
#else
#define PLUGIN_GUARD
#endif

Plugin::Plugin() : id(-1)
{
}

void Plugin::setId(int id)
{
  this->id = id;
}

int Plugin::getId() const
{
  return id;
}

void Plugin::teardown()
{
}
void Plugin::loop()
{
}
void Plugin::websocketHook(JsonDocument &request)
{
}

PluginManager::PluginManager() : nextPluginId(1)
{
}

void PluginManager::init()
{
  Screen.clear();
  std::vector<Plugin *> &allPlugins = pluginManager.getAllPlugins();

  activatePersistedPlugin();
}

void PluginManager::renderPluginId(int pluginId)
{
  if (Scheduler.isActive)
  {
    return;
  }

  Screen.clear();

  std::vector<int> digits;

  if (pluginId >= 10)
  {
    digits.push_back((pluginId - pluginId % 10) / 10);
    digits.push_back(pluginId % 10);
  }
  else
  {
    digits.push_back(pluginId);
  }

  if (pluginId >= 10)
  {
    Screen.drawNumbers(3, 6, digits, MAX_BRIGHTNESS);
  }
  else
  {
    Screen.drawNumbers(6, 6, digits, MAX_BRIGHTNESS);
  }

  Screen.presentAndWait(800);
}

void PluginManager::activatePersistedPlugin()
{
  std::vector<Plugin *> &allPlugins = pluginManager.getAllPlugins();
#ifdef ENABLE_STORAGE
  storage.begin("led-wall", true);
  persistedPluginId = storage.getInt("current-plugin", allPlugins.at(0)->getId());
  pluginManager.setActivePluginById(persistedPluginId);
  storage.end();
#endif
  if (!activePlugin)
  {
    pluginManager.setActivePluginById(allPlugins.at(0)->getId());
  }
}

void PluginManager::persistActivePlugin()
{
#ifdef ENABLE_STORAGE
  storage.begin("led-wall", false);
  if (activePlugin)
  {
    persistedPluginId = activePlugin->getId();
    storage.putInt("current-plugin", persistedPluginId);
  }
  storage.end();
#endif
}

int PluginManager::getPersistedPluginId()
{
  std::vector<Plugin *> &allPlugins = pluginManager.getAllPlugins();
#ifdef ENABLE_STORAGE
  storage.begin("led-wall", true);
  persistedPluginId = storage.getInt("current-plugin", allPlugins.at(0)->getId());
  storage.end();
  return persistedPluginId;
#else
  return -1;
#endif
}

int PluginManager::addPlugin(Plugin *plugin)
{

  plugin->setId(nextPluginId++);
  plugins.push_back(plugin);
  return plugin->getId();
}

void PluginManager::setActivePlugin(const char *pluginName)
{
  PLUGIN_GUARD;
  if (activePlugin)
  {
    activePlugin->teardown();
    activePlugin = nullptr;
  }

  for (Plugin *plugin : plugins)
  {
    if (strcmp(plugin->getName(), pluginName) == 0)
    {
      currentStatus = LOADING; // Prevent plugin loop from drawing during ID display
      activePlugin = plugin;
      renderPluginId(activePlugin->getId());
      activePlugin->setup();
      currentStatus = NONE; // Allow plugin to start drawing
      break;
    }
  }
}

void PluginManager::setActivePluginById(int pluginId)
{
  for (Plugin *plugin : plugins)
  {
    if (plugin->getId() == pluginId)
    {
      setActivePlugin(plugin->getName());
    }
  }
}

void PluginManager::setupActivePlugin()
{
  PLUGIN_GUARD;
  if (activePlugin)
  {
    renderPluginId(activePlugin->getId());
    activePlugin->setup();
  }
}

void PluginManager::runActivePlugin()
{
  PLUGIN_GUARD;
  if (activePlugin && currentStatus != UPDATE && currentStatus != LOADING &&
      currentStatus != WSBINARY)
  {
    activePlugin->loop();
  }
  // present under the lock: a plugin switch from another task clears and
  // redraws the buffer, presenting in between flashes a torn frame.
  // while LOADING someone else (plugin id, scrolling message) owns the buffer
  // and presents its own finished frames
  if (currentStatus != LOADING)
  {
    Screen.present();
  }
}

Plugin *PluginManager::getActivePlugin() const
{
  return activePlugin;
}

std::vector<Plugin *> &PluginManager::getAllPlugins()
{
  return plugins;
}

size_t PluginManager::getNumPlugins()
{
  return plugins.size();
}

void PluginManager::activateNextPlugin()
{
  if (activePlugin)
  {
    if (activePlugin->getId() <= getNumPlugins() - 1)
    {
      setActivePluginById(activePlugin->getId() + 1);
    }
    else
    {
      setActivePluginById(1);
    }
  }
  else
  {
    setActivePluginById(1);
  }
#ifdef ENABLE_SERVER
  sendInfo();
#endif
}