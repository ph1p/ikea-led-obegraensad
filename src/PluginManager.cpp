#include "PluginManager.h"

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
    for (Plugin *plugin : allPlugins)
    {
        Serial.print("init plugin: ");
        Serial.print(plugin->getName());
        Serial.print(" id: ");
        Serial.println(plugin->getId());
    }

#ifdef ENABLE_STORAGE
    storage.begin("led-wall", false);
    Serial.print("restore plugin: ");
    Serial.println(storage.getInt("current-plugin"));
    pluginManager.setActivePluginById(storage.getInt("current-plugin"));
    storage.end();
#endif

    if (!activePlugin)
    {
        pluginManager.setActivePluginById(1);
    }
}

void PluginManager::persistActivePlugin()
{
#ifdef ENABLE_STORAGE
    storage.begin("led-wall", false);
    if (activePlugin)
    {
        Serial.print("persist plugin: ");
        Serial.println(activePlugin->getId());
        storage.putInt("current-plugin", activePlugin->getId());
    }
    storage.end();
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
        activePlugin = nullptr;
    }

    for (Plugin *plugin : plugins)
    {
        if (strcmp(plugin->getName(), pluginName) == 0)
        {
            Serial.print("activate: ");
            Serial.println(plugin->getName());

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

int modeButtonState = 0;
int lastModeButtonState = 1;

void PluginManager::runActivePlugin()
{
    if (currentStatus != LOADING)
    {
        modeButtonState = digitalRead(PIN_BUTTON);
        if (modeButtonState != lastModeButtonState && modeButtonState == HIGH)
        {
            pluginManager.activateNextPlugin();
        }
        lastModeButtonState = modeButtonState;
        currentStatus = NONE;
    }
    if (activePlugin)
    {
        if (currentStatus != UPDATE && currentStatus != LOADING && currentStatus != WSBINARY)
        {
            activePlugin->loop();
        }
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
    Serial.print("next plugin: ");
    Serial.println(activePlugin->getId() + 1);

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
    sendMinimalInfo();
#endif
}