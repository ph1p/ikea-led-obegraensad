#include "plugins/StarsPlugin.h"

void StarsPlugin::setup()
{
  numStars = 25;
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
  Screen.clear();
  for (int i = 0; i < numStars; i++)
  {
    unsigned long currentTime = millis();
    if (stars[i].brightness >= 8)
    {
      if (currentTime - stars[i].lastUpdateTime >= 128)
      {
        stars[i].brightness = max(0, stars[i].brightness - 8);
        stars[i].lastUpdateTime = currentTime;
      }
      Screen.setPixel(stars[i].x, stars[i].y, 1, stars[i].brightness);
    }
    else
    {
      stars[i].x = random(0, 16);
      stars[i].y = random(0, 16);
      stars[i].brightness = random(8, 255);
      stars[i].lastUpdateTime = currentTime;
      Screen.setPixel(stars[i].x, stars[i].y, 1, stars[i].brightness);
    }
  }
  Screen.present(); // Show this frame
  delay(20);        // Control the speed of the star twinkling effect
}

void StarsPlugin::teardown()
{
  numStars = 0;
  Screen.clear();
}

const char *StarsPlugin::getName() const
{
  return "Stars";
}
