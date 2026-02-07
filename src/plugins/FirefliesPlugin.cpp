#include "plugins/FirefliesPlugin.h"
#include <math.h>

void FirefliesPlugin::setup()
{
  Screen.clear();
  for (uint8_t i = 0; i < kFireflyCount; i++)
  {
    fireflies[i].x = static_cast<float>(random(0, 16));
    fireflies[i].y = static_cast<float>(random(0, 16));
    fireflies[i].vx = (static_cast<float>(random(-5, 6))) / 20.0f;
    fireflies[i].vy = (static_cast<float>(random(-5, 6))) / 20.0f;
  }
}

void FirefliesPlugin::loop()
{
  if (!timer.isReady(60))
  {
    return;
  }

  Screen.clear();

  float t = static_cast<float>(millis()) / 350.0f;
  for (uint8_t i = 0; i < kFireflyCount; i++)
  {
    Firefly &firefly = fireflies[i];
    firefly.x += firefly.vx + (static_cast<float>(random(-2, 3)) / 40.0f);
    firefly.y += firefly.vy + (static_cast<float>(random(-2, 3)) / 40.0f);

    if (firefly.x < 0.0f || firefly.x > 15.0f)
    {
      firefly.vx = -firefly.vx;
      firefly.x = firefly.x < 0.0f ? 0.0f : 15.0f;
    }
    if (firefly.y < 0.0f || firefly.y > 15.0f)
    {
      firefly.vy = -firefly.vy;
      firefly.y = firefly.y < 0.0f ? 0.0f : 15.0f;
    }

    uint8_t brightness = static_cast<uint8_t>((sinf(t + i) * 0.5f + 0.5f) * 155.0f + 100.0f);
    Screen.setPixel(static_cast<uint8_t>(firefly.x), static_cast<uint8_t>(firefly.y), 1, brightness);
  }
}

const char *FirefliesPlugin::getName() const
{
  return "Fireflies";
}
