#include "plugins/BigClockPlugin.h"

void BigClockPlugin::setup()
{
  // loading screen
  Screen.setPixel(4, 7, 1);
  Screen.setPixel(5, 7, 1);
  Screen.setPixel(7, 7, 1);
  Screen.setPixel(8, 7, 1);
  Screen.setPixel(10, 7, 1);
  Screen.setPixel(11, 7, 1);

  previousMinutes = -1;
  previousHour = -1;
  previousHH.clear();
  previousMM.clear();
}

void BigClockPlugin::loop()
{
  if (getLocalTime(&timeinfo))
  {
    if (previousHour != timeinfo.tm_hour || previousMinutes != timeinfo.tm_min)
    {
      std::vector<int> hh = {(timeinfo.tm_hour - timeinfo.tm_hour % 10) / 10, timeinfo.tm_hour % 10};
      std::vector<int> mm = {(timeinfo.tm_min - timeinfo.tm_min % 10) / 10, timeinfo.tm_min % 10};

      if (hh != previousHH || mm != previousMM)
      {
        Screen.clear();
        Screen.drawBigNumbers(0, 0, hh);
        Screen.drawBigNumbers(0, ROWS / 2, mm);

        previousHH = hh;
        previousMM = mm;
      }

      previousMinutes = timeinfo.tm_min;
      previousHour = timeinfo.tm_hour;
    }
  }
}

const char *BigClockPlugin::getName() const
{
  return "Big Clock";
}
