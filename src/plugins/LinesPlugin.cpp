#include "plugins/LinesPlugin.h"

void LinesPlugin::setup()
{
  this->count = 0;
}

void LinesPlugin::loop()
{
  std::vector<int> bits = Screen.readBytes(this->frames[this->count]);
  for (int row = 0; row < ROWS; row++)
  {
    for (int col = 0; col < bits.size(); col++)
    {
      Screen.setPixel(col, row, bits[col]);
    }
  }

  this->count++;
  if (this->count >= 4)
  {
    this->count = 0;
  }
  delay(200);
}

const char *LinesPlugin::getName() const
{
  return "Lines";
}