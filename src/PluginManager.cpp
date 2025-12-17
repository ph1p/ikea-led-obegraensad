#include "PluginManager.h"
#include "scheduler.h"

Plugin::Plugin() : id(-1) {}

void Plugin::setId(int id)
{
    this->id = id;
}

int Plugin::getId() const
{
    return id;
}

void Plugin::teardown() {}
void Plugin::loop() {}
void Plugin::websocketHook(DynamicJsonDocument &request) {}

PluginManager::PluginManager() : nextPluginId(1) {}

void PluginManager::init()
{
    Screen.clear();
    std::vector<Plugin *> &allPlugins = pluginManager.getAllPlugins();

    activatePersistedPlugin();
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
    if (activePlugin)
    {
        activePlugin->teardown();
        delay(100);
        activePlugin = nullptr;
    }

    for (Plugin *plugin : plugins)
    {
        if (strcmp(plugin->getName(), pluginName) == 0)
        {
            Screen.clear();
            activePlugin = plugin;
            activePlugin->setup();
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
    if (activePlugin)
    {
        activePlugin->setup();
    }
}

void PluginManager::runActivePlugin()
{
    if (activePlugin && currentStatus != UPDATE &&
        currentStatus != LOADING && currentStatus != WSBINARY && currentStatus != MESSAGES)
    {
        activePlugin->loop();
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