#include "plugins/GameOfLifePlugin.h"

uint8_t GameOfLifePlugin::countNeighbours(int row, int col)
{
  int i, j;
  int count = 0;
  for (i = row - 1; i <= row + 1; i++)
  {
    for (j = col - 1; j <= col + 1; j++)
    {
      count += this->buffer[i * 16 + j];
    }
  }
  count -= this->buffer[row * 16 + col];
  return count;
};

uint8_t GameOfLifePlugin::updateCell(int row, int col)
{
  uint8_t total = this->countNeighbours(row, col);
  if (total > 4 || total < 3)
  {
    return 0;
  }
  else if (this->buffer[row * 16 + col] == 0 && total == 3)
  {
    return 1;
  }
  else
  {
    return this->buffer[row * 16 + col];
  }
};

void GameOfLifePlugin::setup()
{
  Screen.clear();

  memset(previous, 0, ROWS * COLS);
  for (int i = 0; i < ROWS * COLS; i++)
  {
    this->buffer[i] = (random(10)) ? 1 : 0;
  }
  this->next();
};

void GameOfLifePlugin::next()
{
  Screen.clear();
  for (int i = 0; i < ROWS; i++)
  {
    for (int j = 0; j < COLS; j++)
    {
      this->buffer[i * 16 + j] = this->updateCell(i, j);
    }
  }
}

int generations = 30;
void GameOfLifePlugin::loop()
{
  generations--;
  this->next();

  for (int i = 0; i < ROWS; i++)
  {
    for (int j = 0; j < COLS; j++)
    {
      Screen.setPixelAtIndex(i * 16 + j, this->buffer[i * 16 + j]);
    }
  }
  delay(150);

  if (generations == 0)
  {
    generations = 30;
    this->setup();
  }
};

const char *GameOfLifePlugin::getName() const
{
  return "GameOfLife";
}
