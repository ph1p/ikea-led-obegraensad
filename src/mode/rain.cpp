#include "mode/rain.h"

void Rain::setup()
{
  Screen.clear();
  for (byte i = 0; i < Rain::Rain::NUM_DROPS; i++)
  {
    this->drops[i].x = 0;
    this->drops[i].y = 0;
    this->drops[i].visible = false;
  }
}

void Rain::loop()
{
  // dim the trail
  for (uint8_t x = 0; x < Rain::X_MAX; x++)
  {
    for (uint8_t y = 0; y < Rain::Y_MAX; y++)
    {
      uint8_t brightness = Screen.getBufferIndex(y * Rain::X_MAX + x);

      if (brightness <= 25)
        brightness = 0;
      else if (brightness <= 50)
        brightness = 25;
      else if (brightness <= 75)
        brightness = 50;
      else
        brightness = 75;

      Screen.setPixel(x, y, 1, brightness);
    }
  }

  // draw the drops
  for (byte i = 0; i < Rain::NUM_DROPS; i++)
  {
    if (!drops[i].visible)
    {
      if (random(10) > 1) continue;

      this->drops[i].x = random(Rain::X_MAX);
      this->drops[i].y = 0;
      this->drops[i].visible = true;
      
      Screen.setPixel(this->drops[i].x, this->drops[i].y, Rain::LED_TYPE_ON, 255);
    }
    else
    {
      this->drops[i].y++;
      if (this->drops[i].y >= (Rain::Y_MAX))
      {
        this->drops[i].visible = false;
        continue;
      }
      Screen.setPixel(this->drops[i].x, this->drops[i].y, Rain::LED_TYPE_ON, 255);
    }
  }

  delay(80);
}