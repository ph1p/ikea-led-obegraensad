#include "plugins/CometPlugin.h"

void CometPlugin::resetComet()
{
  x = static_cast<float>(random(0, 16));
  y = static_cast<float>(random(0, 16));
  vx = (random(0, 2) == 0) ? 0.8f : -0.8f;
  vy = (random(0, 2) == 0) ? 0.5f : -0.5f;
}

void CometPlugin::setup()
{
  Screen.clear();
  memset(trail, 0, sizeof(trail));
  resetComet();
}

void CometPlugin::loop()
{
  if (!timer.isReady(40))
  {
    return;
  }

  for (uint8_t yIndex = 0; yIndex < 16; yIndex++)
  {
    for (uint8_t xIndex = 0; xIndex < 16; xIndex++)
    {
      uint8_t value = trail[yIndex][xIndex];
      trail[yIndex][xIndex] = (value > 14) ? value - 14 : 0;
    }
  }

  x += vx;
  y += vy;

  if (x < 0.0f || x > 15.0f)
  {
    vx = -vx;
    x = x < 0.0f ? 0.0f : 15.0f;
  }
  if (y < 0.0f || y > 15.0f)
  {
    vy = -vy;
    y = y < 0.0f ? 0.0f : 15.0f;
  }

  int headX = static_cast<int>(x + 0.5f);
  int headY = static_cast<int>(y + 0.5f);
  if (headX >= 0 && headX < 16 && headY >= 0 && headY < 16)
  {
    trail[headY][headX] = 255;
  }

  for (int i = 1; i <= 3; i++)
  {
    int tx = static_cast<int>(x - vx * i + 0.5f);
    int ty = static_cast<int>(y - vy * i + 0.5f);
    if (tx >= 0 && tx < 16 && ty >= 0 && ty < 16)
    {
      uint8_t brightness = static_cast<uint8_t>(220 - i * 50);
      if (trail[ty][tx] < brightness)
      {
        trail[ty][tx] = brightness;
      }
    }
  }

  Screen.clear();
  for (uint8_t yIndex = 0; yIndex < 16; yIndex++)
  {
    for (uint8_t xIndex = 0; xIndex < 16; xIndex++)
    {
      uint8_t value = trail[yIndex][xIndex];
      if (value > 0)
      {
        Screen.setPixel(xIndex, yIndex, 1, value);
      }
    }
  }
}

const char *CometPlugin::getName() const
{
  return "Comet";
}
