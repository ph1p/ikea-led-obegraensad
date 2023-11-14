#pragma once

#include <ArduinoJson.h>
#include <Arduino.h>
#include <vector>
#include <string>

#include "screen.h"
#include "signs.h"
#include "websocket.h"

class Plugin
{
private:
    int id;

public:
    Plugin();

    virtual ~Plugin() {}

    virtual void teardown();
    virtual void websocketHook(DynamicJsonDocument &request);
    virtual void setup() = 0;
    virtual void loop();
    virtual const char *getName() const = 0;

    void setId(int id);
    int getId() const;
};

class PluginManager
{
private:
    std::vector<Plugin *> plugins;
    Plugin *activePlugin;
    int nextPluginId;

public:
    PluginManager();

    int addPlugin(Plugin *plugin);
    void setActivePlugin(const char *pluginName);
    void setActivePluginById(int pluginId);
    void runActivePlugin();
    void setupActivePlugin();
    void activateNextPlugin();
    void persistActivePlugin();
    void init();
    Plugin *getActivePlugin() const;
    std::vector<Plugin *> &getAllPlugins();
    size_t getNumPlugins();
};

extern PluginManager pluginManager;