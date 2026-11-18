#include "config.h"
#include "storage.h"
#include <ArduinoJson.h>

#ifdef ESP32
#include <WiFi.h>
#endif
#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif

static constexpr uint16_t DEFAULT_MQTT_PORT = 1883;
static const char *DEFAULT_DISCOVERY_PREFIX = "homeassistant";

// MQTT topics must not contain wildcards, and HA object ids are built from them
static bool isValidTopic(const String &topic)
{
  if (topic.length() == 0 || topic.length() >= 100 || topic.startsWith("/") || topic.endsWith("/"))
  {
    return false;
  }
  for (size_t i = 0; i < topic.length(); i++)
  {
    const char c = topic[i];
    if (c == '#' || c == '+' || c == ' ')
    {
      return false;
    }
  }
  return true;
}

Config config;


Config::Config()
{
  initialized = false;
#ifdef ENABLE_STORAGE
  storageAvailable = false;
#endif
  setDefaults();
}


void Config::begin()
{
  Serial.println("[Config] Initializing configuration system...");
  
  // Set safe defaults first
  setDefaults();
  
#ifdef ENABLE_STORAGE
  // Try to initialize storage
  try {
    if (preferences.begin("config", false)) {
      storageAvailable = true;
      Serial.println("[Config] Storage initialized successfully");
      
      // Try to load saved configuration
      load();
    } else {
      Serial.println("[Config] Warning: Could not initialize storage, using defaults");
      storageAvailable = false;
    }
  } catch (...) {
    Serial.println("[Config] Error: Exception during storage init, using defaults");
    storageAvailable = false;
  }
#else
  Serial.println("[Config] Storage disabled, using defaults");
#endif
  
  initialized = true;
  
  Serial.println("[Config] ============================================");
  Serial.println("[Config] Current Configuration:");
  Serial.print("[Config] Weather Location: ");
  Serial.println(weatherLocation);
  Serial.print("[Config] NTP Server: ");
  Serial.println(ntpServer);
  Serial.print("[Config] Timezone: ");
  Serial.println(tzInfo);
  Serial.print("[Config] Auto-Start Schedule: ");
  Serial.println(autoStartSchedule ? "enabled" : "disabled");
  Serial.print("[Config] MQTT: ");
  Serial.println(mqttEnabled ? mqttHost + ":" + String(mqttPort) : String("disabled"));
  Serial.println("[Config] ============================================");
}

void Config::setDefaults()
{
  // Use constants from constants.h as defaults
  weatherLocation = String(WEATHER_LOCATION);
  ntpServer = String(NTP_SERVER);
  tzInfo = String(TZ_INFO);
  autoStartSchedule = false;

  mqttEnabled = false;
  mqttHost = "";
  mqttPort = DEFAULT_MQTT_PORT;
  mqttUser = "";
  mqttPassword = "";
  mqttTopic = "";
  mqttDiscovery = true;
  mqttDiscoveryPrefix = DEFAULT_DISCOVERY_PREFIX;
}

void Config::load()
{
#ifdef ENABLE_STORAGE
  if (!storageAvailable) {
    Serial.println("[Config] Storage not available, keeping defaults");
    return;
  }
  
  try {
    // Load with fallback to defaults
    weatherLocation = getStoredString(preferences, "weatherLoc", WEATHER_LOCATION);
    ntpServer = getStoredString(preferences, "ntpServer", NTP_SERVER);
    tzInfo = getStoredString(preferences, "tzInfo", TZ_INFO);
    autoStartSchedule = preferences.getBool("autoSchedule", false);

    mqttEnabled = preferences.getBool("mqttEnabled", false);
    mqttHost = getStoredString(preferences, "mqttHost");
    mqttPort = preferences.getUShort("mqttPort", DEFAULT_MQTT_PORT);
    mqttUser = getStoredString(preferences, "mqttUser");
    mqttPassword = getStoredString(preferences, "mqttPass");
    mqttTopic = getStoredString(preferences, "mqttTopic");
    mqttDiscovery = preferences.getBool("mqttDisc", true);
    mqttDiscoveryPrefix = getStoredString(preferences, "mqttDiscPfx", DEFAULT_DISCOVERY_PREFIX);
    
    Serial.println("[Config] Configuration loaded from storage");
  } catch (...) {
    Serial.println("[Config] Error loading config, using defaults");
    setDefaults();
  }
#endif
}

void Config::save()
{
#ifdef ENABLE_STORAGE
  if (!storageAvailable) {
    Serial.println("[Config] Storage not available, cannot save");
    return;
  }
  
  try {
    preferences.putString("weatherLoc", weatherLocation);
    preferences.putString("ntpServer", ntpServer);
    preferences.putString("tzInfo", tzInfo);
    preferences.putBool("autoSchedule", autoStartSchedule);

    preferences.putBool("mqttEnabled", mqttEnabled);
    preferences.putString("mqttHost", mqttHost);
    preferences.putUShort("mqttPort", mqttPort);
    preferences.putString("mqttUser", mqttUser);
    preferences.putString("mqttPass", mqttPassword);
    preferences.putString("mqttTopic", mqttTopic);
    preferences.putBool("mqttDisc", mqttDiscovery);
    preferences.putString("mqttDiscPfx", mqttDiscoveryPrefix);
    
    Serial.println("[Config] Configuration saved");
  } catch (...) {
    Serial.println("[Config] Error: Could not save configuration");
  }
#else
  Serial.println("[Config] Storage disabled, cannot save");
#endif
}


String Config::getWeatherLocation() const
{
  return weatherLocation.length() > 0 ? weatherLocation : String(WEATHER_LOCATION);
}

String Config::getNtpServer() const
{
  return ntpServer.length() > 0 ? ntpServer : String(NTP_SERVER);
}

String Config::getTzInfo() const
{
  return tzInfo.length() > 0 ? tzInfo : String(TZ_INFO);
}

bool Config::getAutoStartSchedule() const
{
  return autoStartSchedule;
}

String Config::getMqttTopic() const
{
  if (mqttTopic.length() > 0)
  {
    return mqttTopic;
  }
  String mac = WiFi.macAddress();
  mac.replace(":", "");
  mac.toLowerCase();
  return "obegraensad/" + mac.substring(6);
}

String Config::getMqttDiscoveryPrefix() const
{
  return mqttDiscoveryPrefix.length() > 0 ? mqttDiscoveryPrefix : String(DEFAULT_DISCOVERY_PREFIX);
}

void Config::setWeatherLocation(const String& location)
{
  if (location.length() > 0 && location.length() < 100) {
    weatherLocation = location;
  }
}


void Config::setNtpServer(const String& server)
{
  if (server.length() > 0 && server.length() < 100) {
    ntpServer = server;
  }
}

void Config::setTzInfo(const String& tz)
{
  if (tz.length() > 0 && tz.length() < 100) {
    tzInfo = tz;
  }
}

void Config::setAutoStartSchedule(bool autoStart)
{
  autoStartSchedule = autoStart;
}


String Config::toJson() const
{
  JsonDocument doc;
  doc["weatherLocation"] = weatherLocation;
  doc["ntpServer"] = ntpServer;
  doc["tzInfo"] = tzInfo;
  doc["autoStartSchedule"] = autoStartSchedule;
  doc["mqttEnabled"] = mqttEnabled;
  doc["mqttHost"] = mqttHost;
  doc["mqttPort"] = mqttPort;
  doc["mqttUser"] = mqttUser;
  doc["mqttPasswordSet"] = mqttPassword.length() > 0;
  doc["mqttTopic"] = getMqttTopic();
  doc["mqttDiscovery"] = mqttDiscovery;
  doc["mqttDiscoveryPrefix"] = getMqttDiscoveryPrefix();
  
  String output;
  serializeJson(doc, output);
  return output;
}


bool Config::fromJson(const String& json)
{
  if (json.length() == 0) {
    Serial.println("[Config] Empty JSON");
    return false;
  }
  
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, json);
  
  if (error) {
    Serial.print("[Config] JSON parse error: ");
    Serial.println(error.c_str());
    return false;
  }
  
  // Validate and set each field
  if (doc["weatherLocation"].is<String>()) {
    String loc = doc["weatherLocation"].as<String>();
    if (loc.length() > 0 && loc.length() < 100) {
      weatherLocation = loc;
    }
  }
  
  if (doc["ntpServer"].is<String>()) {
    String ntp = doc["ntpServer"].as<String>();
    if (ntp.length() > 0 && ntp.length() < 100) {
      ntpServer = ntp;
    }
  }
  
  if (doc["tzInfo"].is<String>()) {
    String tz = doc["tzInfo"].as<String>();
    if (tz.length() > 0 && tz.length() < 100) {
      tzInfo = tz;
    }
  }
  
  if (doc["autoStartSchedule"].is<bool>()) {
    autoStartSchedule = doc["autoStartSchedule"].as<bool>();
  }

  if (doc["mqttEnabled"].is<bool>())
  {
    mqttEnabled = doc["mqttEnabled"].as<bool>();
  }

  if (doc["mqttHost"].is<String>())
  {
    String host = doc["mqttHost"].as<String>();
    host.trim();
    if (host.length() < 100)
    {
      mqttHost = host;
    }
  }

  if (doc["mqttPort"].is<int>())
  {
    const int port = doc["mqttPort"].as<int>();
    if (port > 0 && port <= 65535)
    {
      mqttPort = port;
    }
  }

  if (doc["mqttUser"].is<String>() && doc["mqttUser"].as<String>().length() < 100)
  {
    mqttUser = doc["mqttUser"].as<String>();
  }

  // absent means keep the stored password, the GUI never receives it
  if (doc["mqttPassword"].is<String>() && doc["mqttPassword"].as<String>().length() < 100)
  {
    mqttPassword = doc["mqttPassword"].as<String>();
  }

  if (doc["mqttTopic"].is<String>())
  {
    String topic = doc["mqttTopic"].as<String>();
    topic.trim();
    // empty resets to the MAC based default
    if (topic.length() == 0 || isValidTopic(topic))
    {
      mqttTopic = topic;
    }
  }

  if (doc["mqttDiscovery"].is<bool>())
  {
    mqttDiscovery = doc["mqttDiscovery"].as<bool>();
  }

  if (doc["mqttDiscoveryPrefix"].is<String>())
  {
    String prefix = doc["mqttDiscoveryPrefix"].as<String>();
    prefix.trim();
    if (prefix.length() == 0 || isValidTopic(prefix))
    {
      mqttDiscoveryPrefix = prefix;
    }
  }
  
  return true;
}

