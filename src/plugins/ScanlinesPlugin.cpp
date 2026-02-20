#include "plugins/ScanlinesPlugin.h"

void ScanlinesPlugin::setup()
{
  Screen.clear();
  position = 0.0f;
  speed = 0.4f;
}

void ScanlinesPlugin::loop()
{
  if (!timer.isReady(60))
  {
    return;
  }

  Screen.clear();

  int y = static_cast<int>(position + 0.5f);
  if (y >= 0 && y < 16)
  {
    Screen.setPixel(0, y, 1, 200);
    Screen.drawLine(0, y, 15, y, 1, 220);
  }

  int y1 = y - 1;
  int y2 = y + 1;
  if (y1 >= 0)
  {
    Screen.drawLine(0, y1, 15, y1, 1, 100);
  }
  if (y2 < 16)
  {
    Screen.drawLine(0, y2, 15, y2, 1, 100);
  }

  position += speed;
  if (position > 15.0f || position < 0.0f)
  {
    speed = -speed;
    position = position < 0.0f ? 0.0f : 15.0f;
  }
}

const char *ScanlinesPlugin::getName() const
{
  return "Scanlines";
}
