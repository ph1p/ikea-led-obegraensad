#include "mode/clock.h"

#ifdef ENABLE_SERVER

struct tm timeinfo;

int previousMinutes;
int previousHour;

void clockSetup()
{
  previousMinutes = -1;
  previousHour = -1;
}

int modeButtonState2 = 0;
int lastModeButtonState2 = 0;

void clockLoop()
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
      Screen.clear();
      Screen.drawNumbers(3, 2, {(timeinfo.tm_hour - timeinfo.tm_hour % 10) / 10, timeinfo.tm_hour % 10});
      Screen.drawNumbers(3, 8, {(timeinfo.tm_min - timeinfo.tm_min % 10) / 10, timeinfo.tm_min % 10});
      Screen.render();
    }

    previousMinutes = timeinfo.tm_min;
    previousHour = timeinfo.tm_hour;
  }
  delay(60);
}

void bigClockSetup()
{
  previousMinutes = -1;
  previousHour = -1;
}

void bigClockLoop()
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

#endif