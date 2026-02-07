#include "plugins/SparkleFieldPlugin.h"

void SparkleFieldPlugin::setup()
{
  Screen.clear();
  memset(sparkles, 0, sizeof(sparkles));
}

void SparkleFieldPlugin::loop()
{
  if (!timer.isReady(70))
  {
    return;
  }

  for (uint8_t y = 0; y < 16; y++)
  {
    for (uint8_t x = 0; x < 16; x++)
    {
      uint8_t value = sparkles[y][x];
      sparkles[y][x] = (value > 18) ? value - 18 : 0;
    }
  }

  for (uint8_t i = 0; i < 6; i++)
  {
    uint8_t x = static_cast<uint8_t>(random(0, 16));
    uint8_t y = static_cast<uint8_t>(random(0, 16));
    sparkles[y][x] = 255;
  }

  Screen.clear();
  for (uint8_t y = 0; y < 16; y++)
  {
    for (uint8_t x = 0; x < 16; x++)
    {
      uint8_t value = sparkles[y][x];
      if (value > 0)
      {
        Screen.setPixel(x, y, 1, value);
      }
    }
  }
}

const char *SparkleFieldPlugin::getName() const
{
  return "Sparkle Field";
}
