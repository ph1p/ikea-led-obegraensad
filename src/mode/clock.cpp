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

    }

    previousMinutes = timeinfo.tm_min;
    previousHour = timeinfo.tm_hour;
  }
}

#endif
