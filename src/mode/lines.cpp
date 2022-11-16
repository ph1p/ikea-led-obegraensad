#include "mode/lines.h"

int count = 0;
bool forwards = true;

void lines()
{
  listenOnButtonToChangeMode();
  std::vector<int> bits = Screen.readBytes(lineAnimation[count]);
  for (int row = 0; row < ROWS; row++)
  {
    for (int col = 0; col < bits.size(); col++)
    {
      Screen.setPixel(col, row, bits[col]);
    }
  }

  Screen.render();

  count++;
  if (count >= 4)
  {
    count = 0;
  }
  delay(200);
}
