#include "mode/lines.h"

int count = 0;
bool forwards = true;

void lines()
{
  drawLine(mode_buffer, count, true);
  drawLine(mode_buffer, count, false);
  renderScreen(mode_buffer);
  memset(mode_buffer, 0, sizeof(mode_buffer));

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
