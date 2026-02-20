#include "config.h"
#include <ArduinoJson.h>

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
  Serial.println("[Config] ============================================");
}

void Config::setDefaults()
{
  // Use constants from constants.h as defaults
  weatherLocation = String(WEATHER_LOCATION);
  ntpServer = String(NTP_SERVER);
  tzInfo = String(TZ_INFO);
  autoStartSchedule = false;
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
    weatherLocation = preferences.getString("weatherLoc", String(WEATHER_LOCATION));
    ntpServer = preferences.getString("ntpServer", String(NTP_SERVER));
    tzInfo = preferences.getString("tzInfo", String(TZ_INFO));
    autoStartSchedule = preferences.getBool("autoSchedule", false);
    
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
  
  return true;
}

