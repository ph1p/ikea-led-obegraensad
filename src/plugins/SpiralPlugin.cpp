#include "plugins/SpiralPlugin.h"
#include <math.h>

void SpiralPlugin::setup()
{
  Screen.clear();
  angle = 0;
  radius = 0;
  expanding = true;
}

void SpiralPlugin::loop()
{
  if (!timer.isReady(30))
    return;

  // Fade previous frame
  for (uint8_t x = 0; x < WIDTH; x++)
  {
    for (uint8_t y = 0; y < HEIGHT; y++)
    {
      uint8_t brightness = Screen.getBufferIndex(y * WIDTH + x);
      if (brightness > 20)
        brightness -= 20;
      else
        brightness = 0;
      
      Screen.setPixel(x, y, 1, brightness);
    }
  }

  // Draw spiral
  for (uint8_t i = 0; i < 5; i++)
  {
    float currentAngle = angle + (i * 0.5);
    float currentRadius = radius - (i * 0.3);
    
    if (currentRadius >= 0)
    {
      int16_t x = CENTER_X + (int16_t)(cos(currentAngle) * currentRadius);
      int16_t y = CENTER_Y + (int16_t)(sin(currentAngle) * currentRadius);
      
      if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT)
      {
        uint8_t brightness = 255 - (i * 40);
        Screen.setPixel(x, y, 1, brightness);
      }
    }
  }

  // Update animation
  angle += 0.15;
  
  if (expanding)
  {
    radius += 0.1;
    if (radius >= 10)
      expanding = false;
  }
  else
  {
    radius -= 0.1;
    if (radius <= 1)
      expanding = true;
  }

  // Keep angle in reasonable bounds
  if (angle > 2 * PI * 10)
    angle -= 2 * PI * 10;
}

const char *SpiralPlugin::getName() const
{
  return "Spiral";
}
