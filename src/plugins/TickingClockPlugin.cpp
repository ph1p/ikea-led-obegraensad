#include "plugins/TickingClockPlugin.h"
#include "timing.h"

void TickingClockPlugin::setup()
{
  previousMinutes = -1;
  previousHour = -1;
  previousHH.clear();
  previousMM.clear();
}

void TickingClockPlugin::loop()
{
  if (getSyncedLocalTime(timeinfo))
  {
    if (previousHour != timeinfo.tm_hour || previousMinutes != timeinfo.tm_min)
    {

      std::vector<int> hh = {(timeinfo.tm_hour - timeinfo.tm_hour % 10) / 10, timeinfo.tm_hour % 10};
      std::vector<int> mm = {(timeinfo.tm_min - timeinfo.tm_min % 10) / 10, timeinfo.tm_min % 10};

      if (previousHH.empty())
      {
        Screen.clear();
        Screen.drawBitmap(2, 0, fontGlyph(fonts[1], hh[0]), fonts[1].sizeY, 8, MAX_BRIGHTNESS);
        Screen.drawBitmap(9, 0, fontGlyph(fonts[1], hh[1]), fonts[1].sizeY, 8, MAX_BRIGHTNESS);
        Screen.drawBitmap(2, 9, fontGlyph(fonts[1], mm[0]), fonts[1].sizeY, 8, MAX_BRIGHTNESS);
        Screen.drawBitmap(9, 9, fontGlyph(fonts[1], mm[1]), fonts[1].sizeY, 8, MAX_BRIGHTNESS);
      }
      else
      {
        if (hh[0] != previousHH[0])
        {
          Screen.drawBitmap(2, 0, fontGlyph(fonts[1], hh[0]), fonts[1].sizeY, 8, MAX_BRIGHTNESS);
        }
        if (hh[1] != previousHH[1])
        {
          Screen.drawBitmap(9, 0, fontGlyph(fonts[1], hh[1]), fonts[1].sizeY, 8, MAX_BRIGHTNESS);
        }
        if (mm[0] != previousMM[0])
        {
          Screen.drawBitmap(2, 9, fontGlyph(fonts[1], mm[0]), fonts[1].sizeY, 8, MAX_BRIGHTNESS);
        }
        if (mm[1] != previousMM[1])
        {
          Screen.drawBitmap(9, 9, fontGlyph(fonts[1], mm[1]), fonts[1].sizeY, 8, MAX_BRIGHTNESS);
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
        Screen.setPixel(timeinfo.tm_sec * 16 / 60, 7, 1, MAX_BRIGHTNESS);
      else
        Screen.setPixel(timeinfo.tm_sec * 16 / 60, 8, 1, MAX_BRIGHTNESS);

      previousSecond = timeinfo.tm_sec;
    }
  }
}

const char *TickingClockPlugin::getName() const
{
  return "Ticking Clock";
}
