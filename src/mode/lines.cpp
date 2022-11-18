#include "mode/lines.h"

void Lines::setup()
{
  this->count = 0;
}

void Lines::loop()
{
  listenOnButtonToChangeMode();
  std::vector<int> bits = Screen.readBytes(this->frames[this->count]);
  for (int row = 0; row < ROWS; row++)
  {
    for (int col = 0; col < bits.size(); col++)
    {
      Screen.setPixel(col, row, bits[col]);
    }
  }

  Screen.render();

  this->count++;
  if (this->count >= 4)
  {
    this->count = 0;
  }
  delay(200);
}
