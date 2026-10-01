#include "plugins/LinesPlugin.h"

// One 16 px row per frame, repeated over the whole screen
static const uint8_t frames[4][2] PROGMEM = {{0xcc, 0xcc}, {0x66, 0x66}, {0x33, 0x33}, {0x99, 0x99}};

void LinesPlugin::setup()
{
  this->count = 0;
}

void LinesPlugin::loop()
{
  if (!timer.isReady(200))
    return;

  for (int row = 0; row < ROWS; row++)
  {
    Screen.drawBitmap(0, row, frames[this->count], sizeof(frames[0]), COLS);
  }

  this->count++;
  if (this->count >= 4)
  {
    this->count = 0;
  }
}

const char *LinesPlugin::getName() const
{
  return "Lines";
}