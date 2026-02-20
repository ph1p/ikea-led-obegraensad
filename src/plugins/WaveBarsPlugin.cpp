#include "plugins/WaveBarsPlugin.h"
#include <math.h>

void WaveBarsPlugin::setup()
{
  Screen.clear();
  phase = 0.0f;
}

void WaveBarsPlugin::loop()
{
  if (!timer.isReady(60))
  {
    return;
  }

  Screen.clear();

  for (uint8_t x = 0; x < 16; x++)
  {
    float wave = (sinf(phase + x * 0.5f) + 1.0f) * 0.5f;
    int height = static_cast<int>(wave * 15.0f);
    for (int y = 15; y >= 15 - height; y--)
    {
      if (y >= 0 && y < 16)
      {
        uint8_t brightness = static_cast<uint8_t>(120 + wave * 120.0f);
        Screen.setPixel(x, static_cast<uint8_t>(y), 1, brightness);
      }
    }
  }

  phase += 0.2f;
}

const char *WaveBarsPlugin::getName() const
{
  return "Wave Bars";
}
