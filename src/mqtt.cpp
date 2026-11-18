#include "mqtt.h"

#ifdef ENABLE_MQTT

#include <ArduinoJson.h>
#include <PubSubClient.h>

#ifdef ESP32
#include <WiFi.h>
#endif
#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif

#include "PluginManager.h"
#include "brightness_schedule.h"
#include "config.h"
#include "messages.h"
#include "scheduler.h"
#include "screen.h"
#include "timing.h"
#include "websocket.h"

namespace
{
WiFiClient netClient;
PubSubClient client(netClient);

// discovery configs carry the whole plugin list in the select options
constexpr uint16_t MQTT_BUFFER_SIZE = 2048;
constexpr unsigned long MIN_BACKOFF_MS = 5000;
constexpr unsigned long MAX_BACKOFF_MS = 60000;
constexpr unsigned long STATE_POLL_MS = 250;
constexpr unsigned long STATE_REFRESH_MS = 60000;

// settings of the current connection, kept to clean up after a change
String baseTopic;
String discoveryPrefix;
String nodeId;
String host;
bool discoveryPublished = false;

volatile bool reconfigureRequested = false;
unsigned long lastAttempt = 0;
unsigned long backoff = 0;
bool firstAttempt = true;

struct State
{
  bool power;
  uint8_t brightness;
  int pluginId;
  bool schedule;
  bool night;
  int rotation;
};
State lastState;
bool stateValid = false;
unsigned long lastStatePublish = 0;

const char *const ROTATIONS[] = {"0", "90", "180", "270"};

struct Entity
{
  const char *component;
  const char *object;
};

// every entity published by publishDiscovery(), also used to remove them again
const Entity ENTITIES[] = {
    {"light", "display"},
    {"select", "plugin"},
    {"button", "next_plugin"},
    {"switch", "plugin_schedule"},
    {"switch", "scheduled_brightness"},
    {"select", "rotation"},
    {"text", "message"},
    {"sensor", "wifi_signal"},
    {"sensor", "ip_address"},
};

String macSuffix()
{
  String mac = WiFi.macAddress();
  mac.replace(":", "");
  mac.toLowerCase();
  return mac.substring(6);
}

String discoveryTopic(const Entity &entity)
{
  return discoveryPrefix + "/" + entity.component + "/" + nodeId + "/" + entity.object + "/config";
}

bool nightScheduleEnabled()
{
  JsonDocument doc;
  ScheduledBrightness.writeToJson(doc.to<JsonObject>());
  return doc["enabled"] | false;
}

State readState()
{
  Plugin *plugin = pluginManager.getActivePlugin();
  return State{
      Screen.isPowered(),
      Screen.getBaseBrightness(),
      plugin ? plugin->getId() : -1,
      Scheduler.isActive,
      nightScheduleEnabled(),
      Screen.currentRotation,
  };
}

bool sameState(const State &a, const State &b)
{
  return a.power == b.power && a.brightness == b.brightness && a.pluginId == b.pluginId &&
         a.schedule == b.schedule && a.night == b.night && a.rotation == b.rotation;
}

void publishState(const State &state)
{
  JsonDocument doc;
  // "state", "brightness" and "color_mode" are read by the JSON schema light
  doc["state"] = state.power ? "ON" : "OFF";
  doc["brightness"] = state.brightness;
  doc["color_mode"] = "brightness";
  Plugin *plugin = pluginManager.getActivePlugin();
  doc["plugin"] = plugin ? plugin->getName() : "";
  doc["pluginId"] = state.pluginId;
  doc["schedule"] = state.schedule ? "ON" : "OFF";
  doc["nightSchedule"] = state.night ? "ON" : "OFF";
  doc["rotation"] = ROTATIONS[state.rotation & 0x03];
  doc["rssi"] = WiFi.RSSI();
  doc["ip"] = WiFi.localIP().toString();

  String payload;
  serializeJson(doc, payload);
  client.publish((baseTopic + "/state").c_str(), payload.c_str(), true);

  lastState = state;
  stateValid = true;
  lastStatePublish = millis();
}

void addDevice(JsonDocument &doc)
{
  JsonObject device = doc["dev"].to<JsonObject>();
  device["ids"].to<JsonArray>().add(nodeId);
  device["name"] = "OBEGRÄNSAD LED";
  device["mf"] = "IKEA (modded)";
  device["mdl"] = "OBEGRÄNSAD";
  device["cu"] = "http://" + WiFi.localIP().toString() + "/";
  JsonArray connections = device["cns"].to<JsonArray>();
  JsonArray mac = connections.add<JsonArray>();
  mac.add("mac");
  mac.add(WiFi.macAddress());
}

// fills the fields every entity shares, abbreviations as documented by HA
void baseEntity(JsonDocument &doc, const Entity &entity, const char *name)
{
  doc["~"] = baseTopic;
  doc["name"] = name;
  doc["uniq_id"] = nodeId + "_" + entity.object;
  doc["obj_id"] = "obegraensad_" + String(entity.object);
  doc["avty_t"] = "~/status";
  addDevice(doc);
}

void publishConfig(const Entity &entity, JsonDocument &doc)
{
  String payload;
  serializeJson(doc, payload);
  if (!client.publish(discoveryTopic(entity).c_str(), payload.c_str(), true))
  {
    Serial.printf("[MQTT] discovery for %s too large (%u bytes)\n", entity.object, payload.length());
  }
}

void publishDiscovery()
{
  {
    JsonDocument doc;
    baseEntity(doc, ENTITIES[0], "Display");
    doc["schema"] = "json";
    doc["stat_t"] = "~/state";
    doc["cmd_t"] = "~/light/set";
    doc["brightness"] = true;
    doc["bri_scl"] = 255;
    doc["sup_clrm"].to<JsonArray>().add("brightness");
    doc["icon"] = "mdi:led-strip-variant";
    publishConfig(ENTITIES[0], doc);
  }
  {
    JsonDocument doc;
    baseEntity(doc, ENTITIES[1], "Plugin");
    doc["stat_t"] = "~/state";
    doc["val_tpl"] = "{{ value_json.plugin }}";
    doc["cmd_t"] = "~/plugin/set";
    doc["icon"] = "mdi:view-grid";
    JsonArray options = doc["ops"].to<JsonArray>();
    for (Plugin *plugin : pluginManager.getAllPlugins())
    {
      options.add(plugin->getName());
    }
    publishConfig(ENTITIES[1], doc);
  }
  {
    JsonDocument doc;
    baseEntity(doc, ENTITIES[2], "Next plugin");
    doc["cmd_t"] = "~/next/set";
    doc["icon"] = "mdi:skip-next";
    publishConfig(ENTITIES[2], doc);
  }
  {
    JsonDocument doc;
    baseEntity(doc, ENTITIES[3], "Plugin schedule");
    doc["stat_t"] = "~/state";
    doc["val_tpl"] = "{{ value_json.schedule }}";
    doc["cmd_t"] = "~/schedule/set";
    doc["icon"] = "mdi:calendar-clock";
    publishConfig(ENTITIES[3], doc);
  }
  {
    JsonDocument doc;
    baseEntity(doc, ENTITIES[4], "Scheduled brightness");
    doc["stat_t"] = "~/state";
    doc["val_tpl"] = "{{ value_json.nightSchedule }}";
    doc["cmd_t"] = "~/night/set";
    doc["icon"] = "mdi:weather-night";
    doc["ent_cat"] = "config";
    publishConfig(ENTITIES[4], doc);
  }
  {
    JsonDocument doc;
    baseEntity(doc, ENTITIES[5], "Rotation");
    doc["stat_t"] = "~/state";
    doc["val_tpl"] = "{{ value_json.rotation }}";
    doc["cmd_t"] = "~/rotation/set";
    doc["icon"] = "mdi:screen-rotation";
    doc["ent_cat"] = "config";
    JsonArray options = doc["ops"].to<JsonArray>();
    for (const char *rotation : ROTATIONS)
    {
      options.add(rotation);
    }
    publishConfig(ENTITIES[5], doc);
  }
  {
    JsonDocument doc;
    baseEntity(doc, ENTITIES[6], "Message");
    doc["cmd_t"] = "~/message/set";
    doc["max"] = 255;
    doc["icon"] = "mdi:message-text";
    publishConfig(ENTITIES[6], doc);
  }
  {
    JsonDocument doc;
    baseEntity(doc, ENTITIES[7], "WiFi signal");
    doc["stat_t"] = "~/state";
    doc["val_tpl"] = "{{ value_json.rssi }}";
    doc["dev_cla"] = "signal_strength";
    doc["unit_of_meas"] = "dBm";
    doc["stat_cla"] = "measurement";
    doc["ent_cat"] = "diagnostic";
    publishConfig(ENTITIES[7], doc);
  }
  {
    JsonDocument doc;
    baseEntity(doc, ENTITIES[8], "IP address");
    doc["stat_t"] = "~/state";
    doc["val_tpl"] = "{{ value_json.ip }}";
    doc["icon"] = "mdi:ip-network";
    doc["ent_cat"] = "diagnostic";
    publishConfig(ENTITIES[8], doc);
  }
  discoveryPublished = true;
}

// empty retained configs make Home Assistant delete the entities
void removeDiscovery()
{
  for (const Entity &entity : ENTITIES)
  {
    client.publish(discoveryTopic(entity).c_str(), "", true);
  }
  discoveryPublished = false;
}

bool payloadIs(const String &payload, const char *value)
{
  return payload.equalsIgnoreCase(value);
}

void handleLight(const String &payload)
{
  JsonDocument doc;
  if (deserializeJson(doc, payload))
  {
    // plain ON / OFF is accepted too
    if (payloadIs(payload, "ON") || payloadIs(payload, "OFF"))
    {
      Screen.setPower(payloadIs(payload, "ON"));
    }
    return;
  }

  if (doc["brightness"].is<int>())
  {
    const int brightness = constrain(doc["brightness"].as<int>(), 0, 255);
    if (brightness == 0)
    {
      Screen.setPower(false);
    }
    else
    {
      Screen.setBaseBrightness(brightness, true);
      ScheduledBrightness.update();
    }
  }

  const char *state = doc["state"] | "";
  if (!strcasecmp(state, "ON"))
  {
    Screen.setPower(true);
  }
  else if (!strcasecmp(state, "OFF"))
  {
    Screen.setPower(false);
  }
}

void handlePlugin(const String &payload)
{
  int pluginId = -1;
  const long numeric = payload.toInt();
  if (numeric > 0 && String(numeric) == payload)
  {
    pluginId = numeric;
  }
  else
  {
    for (Plugin *plugin : pluginManager.getAllPlugins())
    {
      if (payload.equalsIgnoreCase(plugin->getName()))
      {
        pluginId = plugin->getId();
        break;
      }
    }
  }

  if (pluginId < 0)
  {
    Serial.printf("[MQTT] unknown plugin \"%s\"\n", payload.c_str());
    return;
  }

  // a manual pick stops the schedule, like the button and the Web GUI do
  if (Scheduler.isActive)
  {
    Scheduler.stop();
  }
  // the switch itself sends info, on ESP32 it is only queued here
  pluginManager.setActivePluginById(pluginId);
}

void handleNightSchedule(bool enabled)
{
  JsonDocument doc;
  ScheduledBrightness.writeToJson(doc.to<JsonObject>());
  doc["enabled"] = enabled;
  ScheduledBrightness.configureFromJson(doc.as<JsonVariantConst>());
}

void handleRotation(const String &payload)
{
  for (int i = 0; i < 4; i++)
  {
    if (payload == ROTATIONS[i])
    {
      Screen.setCurrentRotation(i, true);
      return;
    }
  }
}

void handleMessage(const String &payload)
{
  if (!payload.startsWith("{"))
  {
    if (payload.length() > 0)
    {
      Messages.add(payload.c_str());
    }
    return;
  }

  JsonDocument doc;
  if (deserializeJson(doc, payload))
  {
    Serial.println("[MQTT] invalid message JSON");
    return;
  }

  std::vector<int> graph;
  for (JsonVariantConst value : doc["graph"].as<JsonArrayConst>())
  {
    graph.push_back(value.as<int>());
  }

  int delay = doc["delay"] | 50;
  int miny = doc["miny"] | 0;
  int maxy = doc["maxy"] | 15;
  // same limits as GET /api/message, scrollGraph() divides by the range
  if (delay <= 0)
  {
    delay = 50;
  }
  if (maxy < miny)
  {
    Serial.println("[MQTT] maxy must be >= miny");
    return;
  }

  Messages.add(doc["text"] | "", doc["repeat"] | 0, doc["id"] | 0, delay, graph, miny, maxy);
}

void onMessage(char *topic, uint8_t *data, unsigned int length)
{
  String payload;
  payload.reserve(length);
  for (unsigned int i = 0; i < length; i++)
  {
    payload += static_cast<char>(data[i]);
  }
  payload.trim();

  String topicString(topic);
  if (topicString == discoveryPrefix + "/status")
  {
    // Home Assistant restarted, it may have lost the retained configs
    if (payloadIs(payload, "online") && config.getMqttDiscovery())
    {
      publishDiscovery();
      stateValid = false;
    }
    return;
  }

  if (!topicString.startsWith(baseTopic + "/"))
  {
    return;
  }
  const String command = topicString.substring(baseTopic.length() + 1);

  if (command == "light/set")
  {
    handleLight(payload);
  }
  else if (command == "plugin/set")
  {
    handlePlugin(payload);
    return;
  }
  else if (command == "next/set")
  {
    if (Scheduler.isActive)
    {
      Scheduler.stop();
    }
    pluginManager.activateNextPlugin();
    return;
  }
  else if (command == "schedule/set")
  {
    if (payloadIs(payload, "ON") && Scheduler.hasSchedule())
    {
      Scheduler.start();
    }
    else if (payloadIs(payload, "OFF"))
    {
      Scheduler.stop();
    }
  }
  else if (command == "night/set")
  {
    handleNightSchedule(payloadIs(payload, "ON"));
  }
  else if (command == "rotation/set")
  {
    handleRotation(payload);
  }
  else if (command == "message/set")
  {
    handleMessage(payload);
    return;
  }
  else if (command == "message/remove")
  {
    Messages.remove(payload.toInt());
    return;
  }
  else
  {
    return;
  }

  // keep the Web GUI in sync, the MQTT state follows from the next poll
  sendInfo();
}

void disconnect()
{
  if (client.connected())
  {
    if (discoveryPublished && (!config.getMqttEnabled() || !config.getMqttDiscovery() ||
                               config.getMqttTopic() != baseTopic ||
                               config.getMqttDiscoveryPrefix() != discoveryPrefix))
    {
      removeDiscovery();
    }
    client.publish((baseTopic + "/status").c_str(), "offline", true);
    client.disconnect();
  }
  discoveryPublished = false;
  stateValid = false;
}

void connect()
{
  baseTopic = config.getMqttTopic();
  discoveryPrefix = config.getMqttDiscoveryPrefix();
  nodeId = "obegraensad_" + macSuffix();
  // PubSubClient keeps the pointer, the String has to stay alive
  host = config.getMqttHost();

  client.setServer(host.c_str(), config.getMqttPort());

  const String clientId = "obegraensad-" + macSuffix();
  const String statusTopic = baseTopic + "/status";
  const String user = config.getMqttUser();
  const String password = config.getMqttPassword();

  Serial.printf("[MQTT] connecting to %s:%u\n", host.c_str(), config.getMqttPort());
  const bool connected =
      client.connect(clientId.c_str(),
                     user.length() > 0 ? user.c_str() : nullptr,
                     user.length() > 0 ? password.c_str() : nullptr,
                     statusTopic.c_str(),
                     1,
                     true,
                     "offline");

  if (!connected)
  {
    Serial.printf("[MQTT] connection failed, state %d\n", client.state());
    return;
  }

  Serial.println("[MQTT] connected");
  client.publish(statusTopic.c_str(), "online", true);
  client.subscribe((baseTopic + "/+/set").c_str());
  client.subscribe((baseTopic + "/message/remove").c_str());

  if (config.getMqttDiscovery())
  {
    client.subscribe((discoveryPrefix + "/status").c_str());
    publishDiscovery();
  }
  stateValid = false;
}
} // namespace

void initMqtt()
{
  client.setBufferSize(MQTT_BUFFER_SIZE);
  client.setCallback(onMessage);
  client.setSocketTimeout(5);
}

void mqttReconfigure()
{
  reconfigureRequested = true;
}

bool mqttIsConnected()
{
  return client.connected();
}

void mqttLoop()
{
  if (reconfigureRequested)
  {
    reconfigureRequested = false;
    disconnect();
    backoff = 0;
    firstAttempt = true;
  }

  if (!config.getMqttEnabled() || config.getMqttHost().length() == 0 ||
      WiFi.status() != WL_CONNECTED)
  {
    return;
  }

  if (!client.connected())
  {
    // connect() blocks for up to a few seconds, back off while the broker is down
    const unsigned long now = millis();
    if (!firstAttempt && now - lastAttempt < backoff)
    {
      return;
    }
    firstAttempt = false;
    lastAttempt = now;
    connect();
    backoff = client.connected() ? 0 : min(max(backoff * 2, MIN_BACKOFF_MS), MAX_BACKOFF_MS);
    if (!client.connected())
    {
      return;
    }
  }

  client.loop();

  static NonBlockingDelay pollTimer;
  if (pollTimer.isReady(STATE_POLL_MS))
  {
    const State state = readState();
    if (!stateValid || !sameState(state, lastState) ||
        millis() - lastStatePublish >= STATE_REFRESH_MS)
    {
      publishState(state);
    }
  }
}

#endif
