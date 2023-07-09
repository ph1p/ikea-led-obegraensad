#include "mode/lines.h"

void Lines::setup()
{
  this->count = 0;
}

void Lines::loop()
{
  std::vector<int> bits = Screen.readBytes(this->frames[this->count]);
  for (int row = 0; row < ROWS; row++)
  {
    for (uint col = 0; col < bits.size(); col++)
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
