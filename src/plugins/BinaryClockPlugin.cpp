#include "plugins/BinaryClockPlugin.h"

void BinaryClockPlugin::setup()
{
  previousHour = -1;
  previousMinutes = -1;
  previousSecond = -1;
}

void BinaryClockPlugin::loop()
{
  if (getLocalTime(&timeinfo))
  {
    if (previousMinutes != timeinfo.tm_sec || previousMinutes != timeinfo.tm_min || previousHour != timeinfo.tm_hour)
    {
      Screen.clear();

      drawBinaryNumber(0, timeinfo.tm_hour);
      drawBinaryNumber(1, timeinfo.tm_min);
      drawBinaryNumber(2, timeinfo.tm_sec);

      previousHour = timeinfo.tm_hour;
      previousMinutes = timeinfo.tm_min;
      previousSecond = timeinfo.tm_sec;
    }

  }
}

void BinaryClockPlugin::drawBinaryNumber(uint8_t col, uint8_t number)
{
    if (col > 2)
    {
        col = 2;
    }

    if (number > 60)
    {
        number = 60;
    }

    auto isBitSet =[](uint8_t number, uint8_t bitIndex)
    {
        return number & (1 << bitIndex);
    };

    int x = 0;
    int y = 0;
    for (unsigned char bitIndex = 0; bitIndex < 6; bitIndex++)
    {
        if (isBitSet(number, bitIndex))
        {
            x = 1;
            x += (bitIndex % 2 == 0) ? 2 : 0;
            x += col * 5;

            y = 1;
            y += 5 * (2 - bitIndex / 2);

            Screen.drawRectangle(x, y, 2, 4, true, BinaryClockPlugin::LED_STATUS_ON);
        }
    }
}

const char *BinaryClockPlugin::getName() const
{
  return "Binary Clock";
}
