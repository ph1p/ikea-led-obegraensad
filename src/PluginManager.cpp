#include "PluginManager.h"
#include "scheduler.h"

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

  unsigned long startTime = millis();
  while (millis() - startTime < 800)
  {
    yield();
#ifdef ESP32
    vTaskDelay(pdMS_TO_TICKS(10));
#else
    delay(10);
#endif
  }
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
#ifdef ENABLE_STORAGE
  storage.begin("led-wall", true);
  persistedPluginId = storage.getInt("current-plugin", plugins.at(0)->getId());
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
  return deferToRenderTask && xTaskGetCurrentTaskHandle() != renderTask;
#else
  return false;
#endif
}

void PluginManager::activatePlugin(Plugin *plugin)
{
  currentStatus = LOADING; // Prevent plugin loop from drawing during ID display
  Plugin *previous = activePlugin;
  if (previous)
  {
    previous->teardown();
  }
  activePlugin = plugin;
  renderPluginId(plugin->getId());
  plugin->setup();
  currentStatus = NONE; // Allow plugin to start drawing

#ifdef ENABLE_SERVER
  sendInfo();
#endif
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

  Plugin *plugin = activePlugin;
  if (plugin)
  {
    plugin->websocketHook(request);
  }
}

void PluginManager::enableRenderTask()
{
#ifdef ESP32
  deferToRenderTask = true;
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

  // Hooks were queued before any switch that is still pending, so they belong
  // to the plugin that is active now.
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

void PluginManager::runActivePlugin()
{
#ifdef ESP32
  if (deferToRenderTask)
  {
    if (renderTask == nullptr)
    {
      renderTask = xTaskGetCurrentTaskHandle();
    }
    processPendingRequests();
  }
#endif

  Plugin *plugin = activePlugin;
  if (plugin && currentStatus != UPDATE && currentStatus != LOADING && currentStatus != WSBINARY)
  {
    plugin->loop();
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
  // Step from a switch that is still queued so repeated presses keep advancing
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
