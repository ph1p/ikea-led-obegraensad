#include "plugins/FireworkPlugin.h"

void FireworkPlugin::drawExplosion(int x, int y, int maxRadius, int brightness)
{
  for (int i = 0; i < 16; i++)
  {
    for (int j = 0; j < 16; j++)
    {
      int distance = round(sqrt(pow(i - x, 2) + pow(j - y, 2)));
      if (distance <= maxRadius)
      {
        Screen.setPixel(i, j, 1, brightness);
      }
    }
  }
}

void FireworkPlugin::explode(int x, int y)
{
  int maxRadius = random(3, 6);

  for (int radius = 1; radius <= maxRadius; radius++)
  {
    drawExplosion(x, y, radius, 255);
    delay(60);
  }

  for (int brightness = 248; brightness >= 0; brightness -= 8)
  {
    drawExplosion(x, y, maxRadius, brightness);
    delay(24);
  }
}

void FireworkPlugin::setup()
{
  Screen.clear();
}

void FireworkPlugin::loop()
{
  Screen.clear();

  int rocketX = random(16);

  int r = random(6);
  for (int i = 16 - 1; i >= r; i--)
  {
    Screen.clear();
    Screen.setPixel(rocketX, i, 1, 255);

    delay(60);
  }

  explode(rocketX, r);

  delay(500);
}

const char *FireworkPlugin::getName() const
{
  return "Firework";
}
