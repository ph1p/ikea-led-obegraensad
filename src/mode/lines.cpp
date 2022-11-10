#include "mode/lines.h"

int count = 0;
bool forwards = true;

void lines()
{
  Screen.drawLine(modeBuffer, count, true);
  Screen.drawLine(modeBuffer, count, false);
  Screen.renderScreen(modeBuffer);
  memset(modeBuffer, 0, sizeof(modeBuffer));

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
  delay(120);
}
