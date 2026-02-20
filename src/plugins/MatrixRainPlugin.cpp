#include "plugins/MatrixRainPlugin.h"

void MatrixRainPlugin::setup()
{
  Screen.clear();
  
  for (uint8_t i = 0; i < NUM_COLUMNS; i++)
  {
    columns[i].y = -random(HEIGHT);
    columns[i].speed = random(1, 4);
    columns[i].length = random(3, MAX_TRAIL_LENGTH);
    columns[i].active = random(100) > 30;
  }
}

void MatrixRainPlugin::loop()
{
  if (!timer.isReady(50))
    return;

  // Fade all pixels
  for (uint8_t x = 0; x < NUM_COLUMNS; x++)
  {
    for (uint8_t y = 0; y < HEIGHT; y++)
    {
      uint8_t brightness = Screen.getBufferIndex(y * NUM_COLUMNS + x);
      if (brightness > 15)
        brightness -= 15;
      else
        brightness = 0;
      
      Screen.setPixel(x, y, 1, brightness);
    }
  }

  // Update columns
  for (uint8_t i = 0; i < NUM_COLUMNS; i++)
  {
    if (!columns[i].active)
    {
      if (random(100) > 95)
      {
        columns[i].active = true;
        columns[i].y = -random(5);
        columns[i].speed = random(1, 4);
        columns[i].length = random(3, MAX_TRAIL_LENGTH);
      }
      continue;
    }

    // Draw the bright head
    if (columns[i].y >= 0 && columns[i].y < HEIGHT)
    {
      Screen.setPixel(i, columns[i].y, 1, 255);
    }

    // Draw trail with decreasing brightness
    for (uint8_t j = 1; j < columns[i].length; j++)
    {
      int16_t trailY = columns[i].y - j;
      if (trailY >= 0 && trailY < HEIGHT)
      {
        uint8_t brightness = 255 - (j * 255 / columns[i].length);
        Screen.setPixel(i, trailY, 1, brightness);
      }
    }

    // Move column
    columns[i].y += columns[i].speed;

    // Reset column if it's past the bottom
    if (columns[i].y - columns[i].length >= HEIGHT)
    {
      columns[i].active = false;
    }
  }
}

const char *MatrixRainPlugin::getName() const
{
  return "Matrix Rain";
}
