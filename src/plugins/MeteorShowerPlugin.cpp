#include "plugins/MeteorShowerPlugin.h"

void MeteorShowerPlugin::resetMeteor(Meteor &meteor)
{
  meteor.x = static_cast<float>(random(0, 16));
  meteor.y = static_cast<float>(random(-12, 0));
  meteor.vx = 0.4f + static_cast<float>(random(0, 4)) / 10.0f;
  meteor.vy = 0.9f + static_cast<float>(random(0, 6)) / 10.0f;
}

void MeteorShowerPlugin::setup()
{
  Screen.clear();
  for (uint8_t i = 0; i < kMeteorCount; i++)
  {
    resetMeteor(meteors[i]);
  }
}

void MeteorShowerPlugin::loop()
{
  if (!timer.isReady(50))
  {
    return;
  }

  Screen.clear();

  for (uint8_t i = 0; i < kMeteorCount; i++)
  {
    Meteor &meteor = meteors[i];
    meteor.x += meteor.vx;
    meteor.y += meteor.vy;

    if (meteor.x > 18.0f || meteor.y > 18.0f)
    {
      resetMeteor(meteor);
    }

    for (uint8_t t = 0; t < 4; t++)
    {
      int tx = static_cast<int>(meteor.x - meteor.vx * t + 0.5f);
      int ty = static_cast<int>(meteor.y - meteor.vy * t + 0.5f);
      if (tx >= 0 && tx < 16 && ty >= 0 && ty < 16)
      {
        uint8_t brightness = static_cast<uint8_t>(230 - t * 50);
        Screen.setPixel(static_cast<uint8_t>(tx), static_cast<uint8_t>(ty), 1, brightness);
      }
    }
  }
}

const char *MeteorShowerPlugin::getName() const
{
  return "Meteor Shower";
}
