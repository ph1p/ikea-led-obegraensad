#include "plugins/RainPlugin.h"

void RainPlugin::setup()
{
  Screen.clear();
  for (byte i = 0; i < RainPlugin::RainPlugin::NUM_DROPS; i++)
    for (unsigned char i = 0; i < RainPlugin::RainPlugin::NUM_DROPS; i++)
    {
      this->drops[i].x = 0;
      this->drops[i].y = 0;
      this->drops[i].visible = false;
    }
}

void RainPlugin::loop()
{
  // dim the trail
  for (uint8_t x = 0; x < RainPlugin::X_MAX; x++)
  {
    for (uint8_t y = 0; y < RainPlugin::Y_MAX; y++)
    {
      uint8_t brightness = Screen.getBufferIndex(y * RainPlugin::X_MAX + x);

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
  for (unsigned char i = 0; i < RainPlugin::NUM_DROPS; i++)
  {
    if (!drops[i].visible)
    {
      if (random(10) > 1)
        continue;

      this->drops[i].x = random(RainPlugin::X_MAX);
      this->drops[i].y = 0;
      this->drops[i].visible = true;

      Screen.setPixel(this->drops[i].x, this->drops[i].y, RainPlugin::LED_TYPE_ON, 255);
    }
    else
    {
      this->drops[i].y++;
      if (this->drops[i].y >= (RainPlugin::Y_MAX))
      {
        this->drops[i].visible = false;
        continue;
      }
      Screen.setPixel(this->drops[i].x, this->drops[i].y, RainPlugin::LED_TYPE_ON, 255);
    }
  }

  delay(100);
}

const char *RainPlugin::getName() const
{
  return "Rain";
}
