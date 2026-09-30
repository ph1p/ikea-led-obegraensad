#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <time.h>

class BrightnessSchedule
{
private:
  BrightnessSchedule() = default;

  static constexpr uint16_t DEFAULT_START_MINUTE = 22 * 60;
  static constexpr uint16_t DEFAULT_END_MINUTE = 7 * 60;
  static constexpr uint8_t DEFAULT_SCHEDULED_BRIGHTNESS = 20;

  bool enabled_ = false;
  uint16_t startMinute_ = DEFAULT_START_MINUTE;
  uint16_t endMinute_ = DEFAULT_END_MINUTE;
  uint8_t brightness_ = DEFAULT_SCHEDULED_BRIGHTNESS;
  bool active_ = false;
  bool initialized_ = false;

  void persist();
  static bool getSyncedLocalTime(struct tm &timeInfo);
  static bool parseTime(const String &value, uint16_t &minutes);
  static String formatTime(uint16_t minutes);

public:
  enum class ConfigureStatus
  {
    Success,
    InvalidConfiguration,
    IdenticalTimes,
  };

  static BrightnessSchedule &getInstance();

  BrightnessSchedule(const BrightnessSchedule &) = delete;
  BrightnessSchedule &operator=(const BrightnessSchedule &) = delete;

  void init();
  void update();
  ConfigureStatus configureFromJson(JsonVariantConst source);
  void writeToJson(JsonObject object) const;
};

extern BrightnessSchedule &ScheduledBrightness;
