#include "brightness_schedule.h"

#include "screen.h"
#include "storage.h"
#include <time.h>

BrightnessSchedule &BrightnessSchedule::getInstance()
{
  static BrightnessSchedule instance;
  return instance;
}

bool BrightnessSchedule::parseTime(const String &value, uint16_t &minutes)
{
  if (value.length() != 5 || value[2] != ':' || value[0] < '0' || value[0] > '9' || value[1] < '0' ||
      value[1] > '9' || value[3] < '0' || value[3] > '9' || value[4] < '0' || value[4] > '9')
  {
    return false;
  }

  const uint8_t hours = (value[0] - '0') * 10 + (value[1] - '0');
  const uint8_t mins = (value[3] - '0') * 10 + (value[4] - '0');
  if (hours > 23 || mins > 59)
  {
    return false;
  }

  minutes = hours * 60 + mins;
  return true;
}

String BrightnessSchedule::formatTime(uint16_t minutes)
{
  char value[6];
  snprintf(value, sizeof(value), "%02u:%02u", minutes / 60, minutes % 60);
  return String(value);
}

void BrightnessSchedule::init()
{
#ifdef ENABLE_STORAGE
  storage.begin("led-wall", true);
  enabled_ = storage.getBool("bsEnabled", false);
  startMinute_ = storage.getUShort("bsStart", DEFAULT_START_MINUTE);
  endMinute_ = storage.getUShort("bsEnd", DEFAULT_END_MINUTE);
  brightness_ = storage.getUChar("bsBrightness", DEFAULT_SCHEDULED_BRIGHTNESS);
  storage.end();

  if (startMinute_ >= 24 * 60 || endMinute_ >= 24 * 60 || startMinute_ == endMinute_)
  {
    enabled_ = false;
    startMinute_ = DEFAULT_START_MINUTE;
    endMinute_ = DEFAULT_END_MINUTE;
    brightness_ = DEFAULT_SCHEDULED_BRIGHTNESS;
  }
#endif

  initialized_ = true;
  update();
}

void BrightnessSchedule::persist()
{
#ifdef ENABLE_STORAGE
  storage.begin("led-wall", false);
  storage.putBool("bsEnabled", enabled_);
  storage.putUShort("bsStart", startMinute_);
  storage.putUShort("bsEnd", endMinute_);
  storage.putUChar("bsBrightness", brightness_);
  storage.end();
#endif
}

BrightnessSchedule::ConfigureStatus BrightnessSchedule::configureFromJson(JsonVariantConst source)
{
  if (!source["enabled"].is<bool>() || !source["startTime"].is<const char *>() ||
      !source["endTime"].is<const char *>() || !source["brightness"].is<int>())
  {
    return ConfigureStatus::InvalidConfiguration;
  }

  const int brightness = source["brightness"].as<int>();
  uint16_t startMinute;
  uint16_t endMinute;
  if (brightness < 0 || brightness > MAX_BRIGHTNESS ||
      !parseTime(source["startTime"].as<String>(), startMinute) ||
      !parseTime(source["endTime"].as<String>(), endMinute))
  {
    return ConfigureStatus::InvalidConfiguration;
  }

  if (startMinute == endMinute)
  {
    return ConfigureStatus::IdenticalTimes;
  }

  enabled_ = source["enabled"].as<bool>();
  startMinute_ = startMinute;
  endMinute_ = endMinute;
  brightness_ = static_cast<uint8_t>(brightness);
  persist();
  update();
  return ConfigureStatus::Success;
}

bool BrightnessSchedule::getSyncedLocalTime(struct tm &timeInfo)
{
  const time_t now = time(nullptr);
  localtime_r(&now, &timeInfo);
  // Same "time is synchronized" heuristic the Arduino cores use in
  // getLocalTime(): struct tm stores years since 1900, and an unsynced SNTP
  // clock reports 1970. Unlike getLocalTime() this does not block.
  return timeInfo.tm_year > (2016 - 1900);
}

void BrightnessSchedule::update()
{
  if (!initialized_)
  {
    return;
  }

  bool shouldBeActive = false;
  struct tm timeinfo;
  if (enabled_ && getSyncedLocalTime(timeinfo))
  {
    const uint16_t currentMinute = timeinfo.tm_hour * 60 + timeinfo.tm_min;
    shouldBeActive = startMinute_ < endMinute_
                         ? currentMinute >= startMinute_ && currentMinute < endMinute_
                         : currentMinute >= startMinute_ || currentMinute < endMinute_;
  }

  active_ = shouldBeActive;
  const uint8_t targetBrightness = active_ ? brightness_ : Screen.getBaseBrightness();
  if (Screen.getCurrentBrightness() != targetBrightness)
  {
    Screen.setDisplayedBrightness(targetBrightness);
  }
}

void BrightnessSchedule::writeToJson(JsonObject object) const
{
  object["enabled"] = enabled_;
  object["startTime"] = formatTime(startMinute_);
  object["endTime"] = formatTime(endMinute_);
  object["brightness"] = brightness_;
  object["active"] = active_;
}

BrightnessSchedule &ScheduledBrightness = BrightnessSchedule::getInstance();
