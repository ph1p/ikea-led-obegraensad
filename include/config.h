#pragma once

#include <Arduino.h>
#include <string>
#include "constants.h"

#ifdef ENABLE_STORAGE
#include <Preferences.h>
#endif

class Config
{
private:
#ifdef ENABLE_STORAGE
  Preferences preferences;
  bool storageAvailable;
#endif
  
  // Current values with safe defaults
  String weatherLocation;
  String ntpServer;
  String tzInfo;
  bool autoStartSchedule;
  bool initialized;

  // MQTT / Home Assistant
  bool mqttEnabled;
  String mqttHost;
  uint16_t mqttPort;
  String mqttUser;
  String mqttPassword;
  String mqttTopic;
  bool mqttDiscovery;
  String mqttDiscoveryPrefix;

public:
  Config();
  
  // Initialize configuration - always succeeds with defaults if storage fails
  void begin();
  
  // Load from storage (safe - uses defaults on failure)
  void load();
  
  // Save to storage (safe - fails silently)
  void save();
  
  // Reset to hardcoded defaults
  void setDefaults();
  
  // Getters (always return valid values)
  String getWeatherLocation() const;
  String getNtpServer() const;
  String getTzInfo() const;
  bool getAutoStartSchedule() const;
  bool isInitialized() const { return initialized; }

  bool getMqttEnabled() const { return mqttEnabled; }
  String getMqttHost() const { return mqttHost; }
  uint16_t getMqttPort() const { return mqttPort; }
  String getMqttUser() const { return mqttUser; }
  String getMqttPassword() const { return mqttPassword; }
  // falls back to obegraensad/<last 6 hex digits of the MAC>
  String getMqttTopic() const;
  bool getMqttDiscovery() const { return mqttDiscovery; }
  String getMqttDiscoveryPrefix() const;
  
  // Setters with validation
  void setWeatherLocation(const String& location);
  void setNtpServer(const String& server);
  void setTzInfo(const String& tz);
  void setAutoStartSchedule(bool autoStart);
  
  // Export to JSON, the MQTT password is never exported
  String toJson() const;
  
  // Import from JSON (validates input)
  bool fromJson(const String& json);
};

extern Config config;
