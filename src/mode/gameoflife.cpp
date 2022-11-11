#include "mode/gameoflife.h"

uint8_t GameOfLife::countNeighbours(int row, int col)
{
  int i, j;
  int count = 0;
  for (i = row - 1; i <= row + 1; i++)
  {
    for (j = col - 1; j <= col + 1; j++)
    {
      count += modeBuffer[i * 16 + j];
    }
  }
  count -= modeBuffer[row * 16 + col];
  return count;
};

uint8_t GameOfLife::updateCell(int row, int col)
{
  uint8_t total = this->countNeighbours(row, col);
  if (total > 4 || total < 3)
  {
    return 0;
  }
  else if (modeBuffer[row * 16 + col] == 0 && total == 3)
  {
    return 1;
  }
  else
  {
    return modeBuffer[row * 16 + col];
  }
};

void GameOfLife::setup()
{
  memset(previous, 0, ROWS * COLS);
  Screen.clear(modeBuffer);
  for (int i = 0; i < ROWS * COLS; i++)
  {
    modeBuffer[i] = (random(20)) ? 1 : 0;
  }
  this->next();
};

void GameOfLife::next()
{
  for (int i = 0; i < ROWS; i++)
  {
    for (int j = 0; j < COLS; j++)
    {
      modeBuffer[i * 16 + j] = this->updateCell(i, j);
      Screen.setPixelAtIndex(modeBuffer, i * 16 + j, modeBuffer[i * 16 + j]);
    }
  }
}

void GameOfLife::loop()
{
  this->next();
  Screen.render(modeBuffer);

  if (memcmp(this->previous, modeBuffer, sizeof(this->previous)) == 0)
  {
    this->setup();
    delay(1000);
  }

  for (int i = 0; i < ROWS * COLS; i++)
  {
    this->previous[i] = modeBuffer[i];
  }
  delay(300);
};
