#include "mode/stars.h"

void stars()
{
  uint8_t buf[256];
  for (uint8_t row = 0; row < ROWS; row++)
  {
    for (uint8_t col = 0; col < COLS; col++)
    {
      uint8_t ra = random(20);

      if (ra > 1)
      {
        ra = 0;
      }
      buf[col * 16 + row] = ra * random(255);
    }
  }
  Screen.setRenderBuffer(buf, true);
  delay(400);
}