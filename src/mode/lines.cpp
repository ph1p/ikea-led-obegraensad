#include "mode/lines.h"

int count = 0;
bool forwards = true;

void lines()
{
  Screen.setRenderBuffer(modeBuffer);
  Screen.drawLine(count, true);
  Screen.drawLine(count, false);
  Screen.render();
  memset(modeBuffer, 0, ROWS * COLS);

  if (count == 15 && forwards)
  {
    forwards = false;
  }
  if (count == 0 && !forwards)
  {
    forwards = true;
    count = 0;
  }

  if (forwards)
  {
    count++;
  }
  else
  {
    count--;
  }
  delay(60);
}
