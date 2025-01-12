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

#ifdef ENABLE_STORAGE
    storage.begin("led-wall", true);
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
        delay(10);
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

int modeButtonState = 0;
int lastModeButtonState = 1;

void PluginManager::runActivePlugin()
{
    static unsigned long lastButtonCheck = 0;
    const unsigned long buttonInterval = 20;
    unsigned long currentMillis = millis();

    if (currentStatus != LOADING &&
        currentMillis - lastButtonCheck >= buttonInterval)
    {
        lastButtonCheck = currentMillis;
        modeButtonState = digitalRead(PIN_BUTTON);

        if (modeButtonState != lastModeButtonState && modeButtonState == HIGH)
        {
            Scheduler.clearSchedule();
            activateNextPlugin();
        }
        lastModeButtonState = modeButtonState;
        currentStatus = NONE;
    }

    if (activePlugin && currentStatus != UPDATE &&
        currentStatus != LOADING && currentStatus != WSBINARY)
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
    sendMinimalInfo();
#endif
}