#include "mode/stars.h"

void stars()
{
  for (uint8_t row = 0; row < ROWS; row++)
  {
    for (uint8_t col = 0; col < COLS; col++)
    {
      uint8_t ra = random(20);

      if (ra > 1)
      {
        ra = 0;
      }
      modeBuffer[row * 16 + col] = ra;
    }
  }
  Screen.render(modeBuffer);
  delay(400);
}