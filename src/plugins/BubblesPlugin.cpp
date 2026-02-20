#include "plugins/BubblesPlugin.h"
#include <math.h>

void BubblesPlugin::resetBubble(Bubble &bubble)
{
  bubble.x = static_cast<float>(random(0, 16));
  bubble.y = static_cast<float>(random(0, 16));
  bubble.radius = static_cast<float>(random(1, 3));
  bubble.speed = 0.2f + static_cast<float>(random(0, 5)) / 10.0f;
  bubble.brightness = static_cast<uint8_t>(random(120, 255));
}

void BubblesPlugin::setup()
{
  Screen.clear();
  for (uint8_t i = 0; i < kBubbleCount; i++)
  {
    resetBubble(bubbles[i]);
  }
}

void BubblesPlugin::loop()
{
  if (!timer.isReady(80))
  {
    return;
  }

  Screen.clear();

  for (uint8_t i = 0; i < kBubbleCount; i++)
  {
    Bubble &bubble = bubbles[i];
    bubble.radius += bubble.speed;
    if (bubble.radius > 10.0f)
    {
      resetBubble(bubble);
    }

    for (uint8_t y = 0; y < 16; y++)
    {
      for (uint8_t x = 0; x < 16; x++)
      {
        float dx = static_cast<float>(x) - bubble.x;
        float dy = static_cast<float>(y) - bubble.y;
        float distance = sqrtf(dx * dx + dy * dy);
        float delta = fabsf(distance - bubble.radius);
        if (delta < 0.6f)
        {
          uint8_t brightness = static_cast<uint8_t>(bubble.brightness * (1.0f - delta / 0.6f));
          Screen.setPixel(x, y, 1, brightness);
        }
      }
    }
  }
}

const char *BubblesPlugin::getName() const
{
  return "Bubbles";
}
