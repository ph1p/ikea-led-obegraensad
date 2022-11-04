#include <Arduino.h>
#include "mode/gameoflife.h"
#include "mode/mode.h"
#include "constants.h"

#include "led.h"

uint8_t previous[256];

uint8_t countNeighbours(int row, int col)
{
  int i, j;
  int count = 0;
  for (i = row - 1; i <= row + 1; i++)
  {
    for (j = col - 1; j <= col + 1; j++)
    {
      count += mode_buffer[i * 16 + j];
    }
  }
  count -= mode_buffer[row * 16 + col];
  return count;
};

uint8_t updateCell(int row, int col)
{
  uint8_t total = countNeighbours(row, col);
  if (total > 4 || total < 3)
  {
    return 0;
  }
  else if (mode_buffer[row * 16 + col] == 0 && total == 3)
  {
    return 1;
  }
  else
  {
    return mode_buffer[row * 16 + col];
  }
};

void gameOfLifeSetup()
{
  memset(previous, 0, ROWS * COLS);
  clearScreenAndBuffer(mode_buffer);
  for (int i = 0; i < ROWS * COLS; i++)
  {
    mode_buffer[i] = (random(20)) ? 1 : 0;
  }
  next();
};

void next()
{
  for (int i = 0; i < ROWS; i++)
  {
    for (int j = 0; j < COLS; j++)
    {
      mode_buffer[i * 16 + j] = updateCell(i, j);
      setPixelAtIndex(mode_buffer, i * 16 + j, mode_buffer[i * 16 + j]);
    }
  }
}

void gameOfLifeLoop()
{
  next();
  renderScreen(mode_buffer);

  if (memcmp(previous, mode_buffer, sizeof(previous)) == 0)
  {
    gameOfLifeSetup();
    delay(1200);
  }

  for (int i = 0; i < ROWS * COLS; i++)
  {
    previous[i] = mode_buffer[i];
  }
  delay(300);
};
