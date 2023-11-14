#include "plugins/StarsPlugin.h"

void StarsPlugin::setup()
{
  for (int i = 0; i < numStars; i++)
  {
    stars[i].x = random(0, 16);
    stars[i].y = random(0, 16);
    stars[i].brightness = random(255);
    stars[i].lastUpdateTime = millis();
  }
}

void StarsPlugin::loop()
{
  for (int fadeStep = 255; fadeStep >= 0; fadeStep -= 8)
  {
    for (int i = 0; i < numStars; i++)
    {
      if (stars[i].brightness > 0)
      {
        unsigned long currentTime = millis();
        if (currentTime - stars[i].lastUpdateTime >= 100)
        {
          stars[i].brightness = max(0, stars[i].brightness - 8);
          Screen.setPixel(stars[i].x, stars[i].y, 1, stars[i].brightness);
          stars[i].lastUpdateTime = currentTime;
        }
      }
      else
      {
        stars[i].x = random(0, 16);
        stars[i].y = random(0, 16);
        stars[i].brightness = random(8, 255);

        for (int brightness = 0; brightness <= stars[i].brightness; brightness += 5)
        {
          unsigned long currentTime = millis();
          if (currentTime - stars[i].lastUpdateTime >= 50)
          {
            Screen.setPixel(stars[i].x, stars[i].y, 1, brightness);
            stars[i].lastUpdateTime = currentTime;
          }
        }
      }
    }
  }
}

const char *StarsPlugin::getName() const
{
  return "Stars";
}
