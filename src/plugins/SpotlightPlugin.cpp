#include "plugins/SpotlightPlugin.h"

// helpers

void SpotlightPlugin::rollVelocity()
{
  float magX = random(28, 37) / 100.0f;
  float magY = random(17, 26) / 100.0f;
  this->vx = (random(0, 2) == 0) ? magX : -magX;
  this->vy = (random(0, 2) == 0) ? magY : -magY;
}

// motion

void SpotlightPlugin::updatePosition()
{
  this->hx += this->vx;
  this->hy += this->vy;

  if (this->hx < 0.0f)
  {
    this->hx = 0.0f;
    this->vx = -this->vx;
  }
  else if (this->hx > (float)(X_MAX - 1))
  {
    this->hx = (float)(X_MAX - 1);
    this->vx = -this->vx;
  }

  if (this->hy < 0.0f)
  {
    this->hy = 0.0f;
    this->vy = -this->vy;
  }
  else if (this->hy > (float)(Y_MAX - 1))
  {
    this->hy = (float)(Y_MAX - 1);
    this->vy = -this->vy;
  }
}

// rendering

void SpotlightPlugin::render()
{
  Screen.clear();

  for (uint8_t y = 0; y < Y_MAX; y++)
  {
    for (uint8_t x = 0; x < X_MAX; x++)
    {
      float dx = (float)x - this->hx;
      float dy = (float)y - this->hy;
      float d = sqrtf(dx * dx + dy * dy);
      if (d >= RADIUS)
      {
        continue;
      }
      float falloff = 1.0f - d / RADIUS;
      int b = (int)(255.0f * falloff * falloff + 0.5f);
      if (b <= 0)
      {
        continue;
      }
      Screen.setPixel(x, y, 1, (uint8_t)b);
    }
  }

  Screen.setPixel((uint8_t)(this->hx + 0.5f), (uint8_t)(this->hy + 0.5f), 1,
                  BRIGHTNESS_FULL);
}

// plugin lifecycle

void SpotlightPlugin::setup()
{
  this->hx = (float)random(POS_MIN, POS_MAX + 1);
  this->hy = (float)random(POS_MIN, POS_MAX + 1);
  rollVelocity();
  Screen.clear();
}

void SpotlightPlugin::loop()
{
  if (!this->frameTimer.isReady(FRAME_INTERVAL_MS))
  {
    return;
  }
  this->updatePosition();
  this->render();
}

const char *SpotlightPlugin::getName() const
{
  return "Spotlight";
}
