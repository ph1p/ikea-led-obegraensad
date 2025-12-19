#include "plugins/TickingClockPlugin.h"

void TickingClockPlugin::setup()
{
  previousMinutes = -1;
  previousHour = -1;
  previousHH.clear();
  previousMM.clear();
}

void TickingClockPlugin::loop()
{
  if (getLocalTime(&timeinfo))
  {

    if ((timeinfo.tm_hour * 60 + timeinfo.tm_min) < 6 * 60 + 30 ||
        (timeinfo.tm_hour * 60 + timeinfo.tm_min) > 22 * 60 + 30) // only between 6:30 and 22:30
    {
      Screen.clear();
      previousHH.clear();
      previousMM.clear();
      return;
    }

    if (previousHour != timeinfo.tm_hour || previousMinutes != timeinfo.tm_min)
    {

      std::vector<int> hh = {(timeinfo.tm_hour - timeinfo.tm_hour % 10) / 10, timeinfo.tm_hour % 10};
      std::vector<int> mm = {(timeinfo.tm_min - timeinfo.tm_min % 10) / 10, timeinfo.tm_min % 10};

      if (previousHH.empty())
      {
        Screen.clear();
        Screen.drawCharacter(2,
                             0,
                             Screen.readBytes(fonts[1].data[hh[0]]),
                             8,
                             Screen.getCurrentBrightness());
        Screen.drawCharacter(9,
                             0,
                             Screen.readBytes(fonts[1].data[hh[1]]),
                             8,
                             Screen.getCurrentBrightness());
        Screen.drawCharacter(2,
                             9,
                             Screen.readBytes(fonts[1].data[mm[0]]),
                             8,
                             Screen.getCurrentBrightness());
        Screen.drawCharacter(9,
                             9,
                             Screen.readBytes(fonts[1].data[mm[1]]),
                             8,
                             Screen.getCurrentBrightness());
      }
      else
      {
        if (hh[0] != previousHH[0])
        {
          Screen.drawCharacter(2,
                               0,
                               Screen.readBytes(fonts[1].data[hh[0]]),
                               8,
                               Screen.getCurrentBrightness());
        }
        if (hh[1] != previousHH[1])
        {
          Screen.drawCharacter(9,
                               0,
                               Screen.readBytes(fonts[1].data[hh[1]]),
                               8,
                               Screen.getCurrentBrightness());
        }
        if (mm[0] != previousMM[0])
        {
          Screen.drawCharacter(2,
                               9,
                               Screen.readBytes(fonts[1].data[mm[0]]),
                               8,
                               Screen.getCurrentBrightness());
        }
        if (mm[1] != previousMM[1])
        {
          Screen.drawCharacter(9,
                               9,
                               Screen.readBytes(fonts[1].data[mm[1]]),
                               8,
                               Screen.getCurrentBrightness());
        }
      }

      previousHH = hh;
      previousMM = mm;
      previousMinutes = timeinfo.tm_min;
      previousHour = timeinfo.tm_hour;
    }
    if (previousSecond != timeinfo.tm_sec)
    {
      // clear second lane
      Screen.clearRect(0, 7, 16, 2);
      // alternating second pixel
      if ((timeinfo.tm_sec * 32 / 60) % 2 == 0)
        Screen.setPixel(timeinfo.tm_sec * 16 / 60, 7, 1, Screen.getCurrentBrightness());
      else
        Screen.setPixel(timeinfo.tm_sec * 16 / 60, 8, 1, Screen.getCurrentBrightness());

      previousSecond = timeinfo.tm_sec;
    }
  }
}

const char *TickingClockPlugin::getName() const
{
  return "Ticking Clock";
}
