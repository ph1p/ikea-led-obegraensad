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
  // On ESP32 plugins run in their own task (see screenDrawingTask in main.cpp),
  // while plugin switches and websocket hooks are triggered from the web server,
  // the button handler and the scheduler. Those callers only queue the request;
  // the drawing task applies it between two loop() calls, so a plugin is never
  // set up, torn down or hooked while its loop() is running.
  static constexpr size_t MAX_PENDING_HOOKS = 32;

  std::atomic<bool> deferToRenderTask{false};
  std::atomic<TaskHandle_t> renderTask{nullptr};
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
  // Returns false if no plugin has that id. On ESP32, once enableRenderTask()
  // was called, the switch happens asynchronously in the drawing task.
  bool setActivePluginById(int pluginId);
  void dispatchWebsocketHook(JsonDocument &request);
  // Must be called before the drawing task starts calling runActivePlugin().
  void enableRenderTask();
  void runActivePlugin();
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
