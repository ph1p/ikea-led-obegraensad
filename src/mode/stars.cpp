#include "mode/stars.h"

void stars()
{
  for (int row = 0; row < ROWS; row++)
  {
    for (int col = 0; col < COLS; col++)
    {
      int ra = random(20);

      if (ra > 1)
      {
        ra = 0;
      }
      mode_buffer[row * 16 + col] = ra;
    }
  }
  renderScreen(mode_buffer);
  delay(400);
}