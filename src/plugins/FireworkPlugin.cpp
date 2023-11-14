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
    delay(explosionDelay);
  }

  for (int brightness = 248; brightness >= 0; brightness -= 8)
  {
    drawExplosion(x, y, maxRadius, brightness);
    delay(fadeDelay);
  }
}

void FireworkPlugin::setup()
{
  Screen.clear();
}

void FireworkPlugin::loop()
{
  static int rocketY = 16;
  static int rocketX = 0;
  static bool rocketLaunched = false;

  unsigned long currentMillis = millis();

  if (!rocketLaunched)
  {
    if (currentMillis - previousMillis >= rocketDelay)
    {
      previousMillis = currentMillis;
      Screen.clear();
      Screen.setPixel(rocketX, rocketY, 1, 255);
      rocketY--;

      if (rocketY < random(8))
      {
        rocketLaunched = true;
        explode(rocketX, rocketY);
      }
    }
  }
  else
  {
    if (currentMillis - previousMillis >= explosionDuration)
    {
      rocketY = 16;
      rocketX = random(16);
      rocketLaunched = false;
      previousMillis = currentMillis;
    }
  }
}

const char *FireworkPlugin::getName() const
{
  return "Firework";
}
