#include "mode/bigclock.h"


void BigClock::setup()
{
  previousMinutes = -1;
  previousHour = -1;
}

void BigClock::loop()
{
  listenOnButtonToChangeMode();

  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
  }
  else
  {
    if (previousHour != timeinfo.tm_hour || previousMinutes != timeinfo.tm_min) 
    {
      std::vector<int> hh = {(timeinfo.tm_hour - timeinfo.tm_hour % 10) / 10, timeinfo.tm_hour % 10};
      std::vector<int> mm = {(timeinfo.tm_min - timeinfo.tm_min % 10) / 10, timeinfo.tm_min % 10};
      bool leadingZero = (hh.at(0) == 0);
      Screen.clear();
      if (leadingZero) {
        hh.erase(hh.begin());
        Screen.drawBigNumbers(COLS/2, 0, hh);
        Screen.drawBigNumbers(0, ROWS/2, mm);
      } else {
        Screen.drawBigNumbers(0, 0, hh);
        Screen.drawBigNumbers(0, ROWS/2, mm);
      }
      Screen.render();
    }

  previousMinutes = timeinfo.tm_min;
  previousHour = timeinfo.tm_hour;
  }
  delay(60);
}