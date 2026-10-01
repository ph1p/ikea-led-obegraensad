#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <atomic>
#include <string>
#include <vector>

#ifdef ESP32
#include <deque>
#include <mutex>
#endif

#include "screen.h"
#include "signs.h"
#include "websocket.h"

class Plugin
{
private:
  int id;

public:
  Plugin();

  virtual ~Plugin()
  {
  }

  virtual void teardown();
  virtual void websocketHook(JsonDocument &request);
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
  std::atomic<Plugin *> activePlugin{nullptr};
  int nextPluginId;
  int persistedPluginId = 1;

#ifdef ESP32
  // Switches and websocket hooks from the web server and button tasks are only
  // queued; the screen task applies them in runActivePlugin(). Otherwise those
  // tasks would wait on the plugin lock for a whole loop() (seconds for some
  // plugins) plus the plugin id display, stalling the web server meanwhile.
  static constexpr size_t MAX_PENDING_HOOKS = 32;

  std::atomic<bool> deferToScreenTask{false};
  std::atomic<TaskHandle_t> screenTask{nullptr};
  std::atomic<int> requestedPluginId{-1};
  std::mutex pendingHooksMutex;
  std::deque<JsonDocument> pendingHooks;

  void processPendingRequests();
#endif

  bool mustDefer() const;
  Plugin *findPlugin(int pluginId) const;
  void activatePlugin(Plugin *plugin);
  void renderPluginId(int pluginId);

public:
  PluginManager();

  int addPlugin(Plugin *plugin);
  void setActivePlugin(const char *pluginName);
  // Returns false if no plugin has that id. On ESP32, once enableScreenTask()
  // was called, other tasks only queue the switch for the screen task.
  bool setActivePluginById(int pluginId);
  void dispatchWebsocketHook(JsonDocument &request);
  // Call before the screen task starts calling runActivePlugin()
  void enableScreenTask();
  void runActivePlugin();
  void setupActivePlugin();
  void activateNextPlugin();
  void persistActivePlugin();
  void init();
  void activatePersistedPlugin();
  int getPersistedPluginId();
  Plugin *getActivePlugin() const;
  std::vector<Plugin *> &getAllPlugins();
  size_t getNumPlugins();
};

extern PluginManager pluginManager;
