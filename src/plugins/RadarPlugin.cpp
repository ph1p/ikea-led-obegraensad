#include "plugins/RadarPlugin.h"
#include <math.h>

void RadarPlugin::setup()
{
  Screen.clear();
  sweepAngle = 0;
  sweepSpeed = 0.1;
  
  for (uint8_t i = 0; i < NUM_BLIPS; i++)
  {
    blips[i].x = random(WIDTH);
    blips[i].y = random(HEIGHT);
    blips[i].age = 0;
    blips[i].active = false;
  }
}

void RadarPlugin::loop()
{
  if (!timer.isReady(50))
    return;

  // Fade all pixels
  for (uint8_t y = 0; y < HEIGHT; y++)
  {
    for (uint8_t x = 0; x < WIDTH; x++)
    {
      uint8_t brightness = Screen.getBufferIndex(y * WIDTH + x);
      if (brightness > 10)
        brightness -= 10;
      else
        brightness = 0;
      
      Screen.setPixel(x, y, 1, brightness);
    }
  }

  // Draw grid circles
  for (uint8_t r = 2; r < 12; r += 3)
  {
    for (float a = 0; a < 2 * PI; a += 0.3)
    {
      int16_t x = CENTER_X + (int16_t)(cos(a) * r);
      int16_t y = CENTER_Y + (int16_t)(sin(a) * r);
      
      if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT)
      {
        Screen.setPixel(x, y, 1, 30);
      }
    }
  }

  // Draw sweep line
  for (uint8_t r = 0; r < 12; r++)
  {
    int16_t x = CENTER_X + (int16_t)(cos(sweepAngle) * r);
    int16_t y = CENTER_Y + (int16_t)(sin(sweepAngle) * r);
    
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT)
    {
      uint8_t brightness = 255 - (r * 20);
      Screen.setPixel(x, y, 1, brightness);
    }
  }

  // Check for blips in sweep path
  for (uint8_t i = 0; i < NUM_BLIPS; i++)
  {
    if (!blips[i].active)
      continue;
    
    // Calculate angle to blip
    float dx = blips[i].x - CENTER_X;
    float dy = blips[i].y - CENTER_Y;
    float blipAngle = atan2(dy, dx);
    
    // Normalize angles
    float angleDiff = fabs(sweepAngle - blipAngle);
    if (angleDiff > PI) angleDiff = 2 * PI - angleDiff;
    
    // If sweep hits the blip, highlight it
    if (angleDiff < 0.3)
    {
      Screen.setPixel(blips[i].x, blips[i].y, 1, 255);
      blips[i].age = 0;
    }
    else if (blips[i].age < 100)
    {
      uint8_t brightness = 200 - (blips[i].age * 2);
      Screen.setPixel(blips[i].x, blips[i].y, 1, brightness);
      blips[i].age++;
    }
    else
    {
      blips[i].active = false;
    }
  }

  // Randomly spawn new blips
  if (random(100) > 90)
  {
    for (uint8_t i = 0; i < NUM_BLIPS; i++)
    {
      if (!blips[i].active)
      {
        blips[i].x = random(WIDTH);
        blips[i].y = random(HEIGHT);
        blips[i].age = 0;
        blips[i].active = true;
        break;
      }
    }
  }

  sweepAngle += sweepSpeed;
  if (sweepAngle > 2 * PI)
    sweepAngle -= 2 * PI;
}

const char *RadarPlugin::getName() const
{
  return "Radar";
}
