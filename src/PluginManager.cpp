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
  const int firstPluginId = plugins.at(0)->getId();
#ifdef ENABLE_STORAGE
  storage.begin("led-wall", true);
  persistedPluginId = storage.getInt("current-plugin", firstPluginId);
  storage.end();

  if (setActivePluginById(persistedPluginId))
  {
    return;
  }
#endif
  setActivePluginById(firstPluginId);
}

void PluginManager::persistActivePlugin()
{
#ifdef ENABLE_STORAGE
  Plugin *plugin = activePlugin;
  storage.begin("led-wall", false);
  if (plugin)
  {
    persistedPluginId = plugin->getId();
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

Plugin *PluginManager::findPlugin(int pluginId) const
{
  for (Plugin *plugin : plugins)
  {
    if (plugin->getId() == pluginId)
    {
      return plugin;
    }
  }
  return nullptr;
}

bool PluginManager::mustDefer() const
{
#ifdef ESP32
  return deferToScreenTask && xTaskGetCurrentTaskHandle() != screenTask;
#else
  return false;
#endif
}

void PluginManager::activatePlugin(Plugin *plugin)
{
  {
    PLUGIN_GUARD;
    currentStatus = LOADING; // Prevent plugin loop from drawing during ID display
    Plugin *previous = activePlugin;
    if (previous)
    {
      previous->teardown();
    }
    // never null after boot: sendInfo() reads it from other tasks
    activePlugin = plugin;
    renderPluginId(plugin->getId());
    plugin->setup();
    currentStatus = NONE; // Allow plugin to start drawing
  }

#ifdef ENABLE_SERVER
  sendInfo();
#endif
}

void PluginManager::setActivePlugin(const char *pluginName)
{
  for (Plugin *plugin : plugins)
  {
    if (strcmp(plugin->getName(), pluginName) == 0)
    {
      setActivePluginById(plugin->getId());
      return;
    }
  }
}

bool PluginManager::setActivePluginById(int pluginId)
{
  Plugin *plugin = findPlugin(pluginId);
  if (!plugin)
  {
    return false;
  }

#ifdef ESP32
  if (mustDefer())
  {
    requestedPluginId = pluginId;
    return true;
  }
#endif

  activatePlugin(plugin);
  return true;
}

void PluginManager::dispatchWebsocketHook(JsonDocument &request)
{
#ifdef ESP32
  if (mustDefer())
  {
    std::lock_guard<std::mutex> lock(pendingHooksMutex);
    if (pendingHooks.size() >= MAX_PENDING_HOOKS)
    {
      Serial.println(F("PluginManager: websocket hook queue full, message dropped"));
      return;
    }
    pendingHooks.emplace_back(request);
    return;
  }
#endif

  PLUGIN_GUARD;
  Plugin *plugin = activePlugin;
  if (plugin)
  {
    plugin->websocketHook(request);
  }
}

void PluginManager::enableScreenTask()
{
#ifdef ESP32
  deferToScreenTask = true;
#endif
}

#ifdef ESP32
void PluginManager::processPendingRequests()
{
  std::deque<JsonDocument> hooks;
  {
    std::lock_guard<std::mutex> lock(pendingHooksMutex);
    hooks.swap(pendingHooks);
  }

  // hooks queued before a still pending switch belong to the current plugin
  for (JsonDocument &request : hooks)
  {
    Plugin *plugin = activePlugin;
    if (plugin)
    {
      plugin->websocketHook(request);
    }
  }

  const int pluginId = requestedPluginId.exchange(-1);
  if (pluginId >= 0)
  {
    Plugin *plugin = findPlugin(pluginId);
    if (plugin)
    {
      activatePlugin(plugin);
    }
  }
}
#endif

void PluginManager::setupActivePlugin()
{
  PLUGIN_GUARD;
  Plugin *plugin = activePlugin;
  if (plugin)
  {
    renderPluginId(plugin->getId());
    plugin->setup();
  }
}

void PluginManager::runActivePlugin()
{
  PLUGIN_GUARD;
#ifdef ESP32
  if (deferToScreenTask)
  {
    if (screenTask == nullptr)
    {
      screenTask = xTaskGetCurrentTaskHandle();
    }
    processPendingRequests();
  }
#endif

  Plugin *plugin = activePlugin;
  if (plugin && currentStatus != UPDATE && currentStatus != LOADING && currentStatus != WSBINARY)
  {
    plugin->loop();
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
  // step from a switch that is still queued so repeated presses keep advancing
  int currentId = -1;
#ifdef ESP32
  currentId = requestedPluginId;
#endif
  if (currentId < 0)
  {
    Plugin *plugin = activePlugin;
    currentId = plugin ? plugin->getId() : 0;
  }

  if (currentId >= 1 && currentId < (int)getNumPlugins())
  {
    setActivePluginById(currentId + 1);
  }
  else
  {
    setActivePluginById(1);
  }
}
