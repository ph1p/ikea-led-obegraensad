#include "mode/lines.h"

int count = 0;
bool forwards = true;

void lines()
{
  listenOnButtonToChangeMode();
  Screen.clear();
  Screen.drawLine(count, 0, count, 15, 1);
  Screen.drawLine(0, count, 15, count, 1);
  Screen.render();

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
