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
}

void BigClockPlugin::loop()
{
  static unsigned long lastUpdate = 0;
  unsigned long now = millis();
  if (now - lastUpdate >= 1000) // Only update once per second
  {
    lastUpdate = now;
    if (getLocalTime(&timeinfo))
    {
      previousMinutes = timeinfo.tm_min;
      previousHour = timeinfo.tm_hour;
    }

    std::vector<int> hh = {(previousHour - previousHour % 10) / 10, previousHour % 10};
    std::vector<int> mm = {(previousMinutes - previousMinutes % 10) / 10, previousMinutes % 10};

    bool leadingZero = (hh.at(0) == 0);

    noInterrupts();
    Screen.clear();
    if (leadingZero)
    {
      hh.erase(hh.begin());
      Screen.drawBigNumbers(COLS / 2, 0, hh);
      Screen.drawBigNumbers(0, ROWS / 2, mm);
    }
    else
    {
      Screen.drawBigNumbers(0, 0, hh);
      Screen.drawBigNumbers(0, ROWS / 2, mm);
    }
    interrupts();
  }
}

const char *BigClockPlugin::getName() const
{
  return "Big Clock";
}
