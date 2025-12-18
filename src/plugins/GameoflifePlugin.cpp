#include "plugins/GameOfLifePlugin.h"

uint8_t GameOfLifePlugin::countNeighbours(int row, int col)
{
  int i, j;
  int count = 0;
  for (i = row - 1; i <= row + 1; i++)
  {
    for (j = col - 1; j <= col + 1; j++)
    {
      int r = i;
      int c = j;
      // OOB handling
      if (r > (ROWS - 1))
        r = 0;
      if (r < 0)
        r = ROWS - 1;
      if (c > (COLS - 1))
        c = 0;
      if (c < 0)
        c = COLS - 1;

      count += this->previous[r * COLS + c];
    }
  }
  count -= this->previous[row * COLS + col];
  return count;
};

uint8_t GameOfLifePlugin::updateCell(int row, int col)
{
  uint8_t total = this->countNeighbours(row, col);
  if (total > 3 || total < 2)
  {
    return 0;
  }
  else if (this->previous[row * COLS + col] == 0 && total == 3)
  {
    return 1;
  }
  else
  {
    return this->previous[row * COLS + col];
  }
};

void GameOfLifePlugin::setup()
{
  this->state = this->STATE_END;
  this->initStep = 0;
};

void GameOfLifePlugin::init()
{
  if (initStep == 0)
  {
    memset(this->previous2, 0, ROWS * COLS);
    memset(this->previous, 0, ROWS * COLS);
    memset(this->buffer, 0, ROWS * COLS);
    for (int i = 0; i < ROWS * COLS; i++)
    {
      this->buffer[i] = (random(2)) ? 1 : 0;
    }
    initStep = 1;
    initTimer.reset();
    return;
  }

  // Animated reveal of the board (non-blocking)
  if (initTimer.isReady(50))
  {
    int j = (initStep - 1) / COLS;
    int i = (initStep - 1) % COLS;

    if (j < 8)
    {
      if (j < 4)
      { // grayish cover
        Screen.setPixel((i), (j * 4 + 0), 1, 25);
        Screen.setPixel((i), (j * 4 + 1), 1, 25);
        Screen.setPixel((i), (j * 4 + 2), 1, 25);
        Screen.setPixel((i), (j * 4 + 3), 1, 25);
      }
      else
      { // fill in actual cells
        int actualRow = j - 4;
        Screen.setPixel(i, (actualRow * 4 + 0), this->buffer[(actualRow * 4 + 0) * COLS + i]);
        Screen.setPixel(i, (actualRow * 4 + 1), this->buffer[(actualRow * 4 + 1) * COLS + i]);
        Screen.setPixel(i, (actualRow * 4 + 2), this->buffer[(actualRow * 4 + 2) * COLS + i]);
        Screen.setPixel(i, (actualRow * 4 + 3), this->buffer[(actualRow * 4 + 3) * COLS + i]);
      }

      initStep++;
    }
    else
    {
      // Animation complete
      this->state = this->STATE_RUNNING;
      this->initStep = 0;
    }
  }
}

void GameOfLifePlugin::next()
{
  memcpy(this->previous2, this->previous, ROWS * COLS);
  memcpy(this->previous, this->buffer, ROWS * COLS);
  for (int i = 0; i < ROWS; i++)
  {
    for (int j = 0; j < COLS; j++)
    {
      this->buffer[i * COLS + j] = this->updateCell(i, j);
    }
  }
}

void GameOfLifePlugin::show()
{
  Screen.clear();

  for (int i = 0; i < ROWS; i++)
  {
    for (int j = 0; j < COLS; j++)
    {
      Screen.setPixel(j, i, this->buffer[i * COLS + j]);
    }
  }
}

int generations = 120;
bool updated;
bool updated2;
void GameOfLifePlugin::loop()
{
  switch (this->state)
  {
  case this->STATE_RUNNING:
    if (updateTimer.isReady(gol_delay))
    {
      this->show();

      generations--;
      this->next();
      updated = memcmp(this->buffer, this->previous, ROWS * COLS);
      updated2 = memcmp(this->buffer, this->previous2, ROWS * COLS);

      // is running in period of 2
      if (!updated2)
        generations -= 5;

      if (generations < 0 || !updated)
      {
        generations = 120;
        updateTimer.reset();
        this->state = this->STATE_END_DELAY;
      }
    }
    break;
  case this->STATE_END_DELAY:
    if (updateTimer.isReady(gol_delay * 4))
    {
      this->state = this->STATE_END;
    }
    break;
  case this->STATE_INIT:
    this->init();
    break;
  case this->STATE_END:
    this->state = this->STATE_INIT;
    this->initStep = 0;
    break;
  }
};

const char *GameOfLifePlugin::getName() const
{
  return "GameOfLife";
}

void GameOfLifePlugin::websocketHook(DynamicJsonDocument &request)
{
  const char *event = request["event"];

  if (currentStatus == NONE)
  {
    if (!strcmp(event, "goldelay"))
    {
      uint16_t new_delay = request["delay"].as<uint16_t>();
      Serial.print("Changing Game of life delay to ");
      Serial.println(new_delay);
      gol_delay = new_delay;
    }
  }
}
