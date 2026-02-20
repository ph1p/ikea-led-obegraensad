#include "plugins/WavePlugin.h"
#include <math.h>

void WavePlugin::setup()
{
  Screen.clear();
  phase = 0;
  waveSpeed = 0.2;
}

void WavePlugin::loop()
{
  if (!timer.isReady(50))
    return;

  // Generate wave pattern
  for (uint8_t y = 0; y < HEIGHT; y++)
  {
    for (uint8_t x = 0; x < WIDTH; x++)
    {
      // Create multiple overlapping waves
      float wave1 = sin(x * 0.5 + phase);
      float wave2 = sin(y * 0.4 + phase * 1.3);
      float wave3 = cos((x + y) * 0.3 - phase * 0.8);
      
      // Combine waves
      float combined = (wave1 + wave2 + wave3) / 3.0;
      
      // Convert to brightness (0-255)
      uint8_t brightness = (uint8_t)(((combined + 1.0) / 2.0) * 255);
      
      Screen.setPixel(x, y, 1, brightness);
    }
  }

  phase += waveSpeed;
  if (phase > 2 * PI * 100)
    phase -= 2 * PI * 100;
}

const char *WavePlugin::getName() const
{
  return "Wave";
}
