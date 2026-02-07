#include "plugins/CheckerboardPlugin.h"

void CheckerboardPlugin::setup()
{
  Screen.clear();
  offset = 0;
  inverted = false;
  phase = 0;
}

void CheckerboardPlugin::loop()
{
  if (!timer.isReady(100))
    return;

  for (uint8_t y = 0; y < HEIGHT; y++)
  {
    for (uint8_t x = 0; x < WIDTH; x++)
    {
      // Calculate which square this pixel belongs to
      uint8_t squareX = (x + offset) / SQUARE_SIZE;
      uint8_t squareY = (y + offset) / SQUARE_SIZE;
      
      // Determine if this square should be lit
      bool isLit = ((squareX + squareY) % 2) == 0;
      
      if (inverted)
        isLit = !isLit;
      
      // Create pulsing effect
      uint8_t brightness = 0;
      if (isLit)
      {
        brightness = phase;
      }
      else
      {
        brightness = 255 - phase;
      }
      
      Screen.setPixel(x, y, 1, brightness);
    }
  }

  // Animate the pattern
  if (phase < 255)
  {
    phase += 15;
  }
  else
  {
    phase = 0;
    inverted = !inverted;
    
    // Occasionally shift the pattern
    if (random(100) > 70)
    {
      offset = (offset + 1) % (SQUARE_SIZE * 2);
    }
  }
}

const char *CheckerboardPlugin::getName() const
{
  return "Checkerboard";
}
