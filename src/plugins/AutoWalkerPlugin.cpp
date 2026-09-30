#include "plugins/AutoWalkerPlugin.h"

// helpers

void AutoWalkerPlugin::pickRandomPosition()
{
  this->x = (int8_t)random(0, X_MAX);
  this->y = (int8_t)random(0, Y_MAX);
}

uint8_t AutoWalkerPlugin::brightnessForCount(uint8_t count) const
{
  if (count <= 1)
  {
    return BRIGHTNESS_VISIT_1;
  }
  if (count == 2)
  {
    return BRIGHTNESS_VISIT_2;
  }
  return BRIGHTNESS_VISIT_MAX;
}

// walking

void AutoWalkerPlugin::resetRun()
{
  for (uint8_t y = 0; y < Y_MAX; y++)
  {
    for (uint8_t x = 0; x < X_MAX; x++)
    {
      this->counts[y][x] = 0;
    }
  }
  this->visitedCells = 0;
  this->pickRandomPosition();
  this->state = RUN;
}

void AutoWalkerPlugin::step()
{
  int8_t neighborX[NEIGHBOR_COUNT];
  int8_t neighborY[NEIGHBOR_COUNT];
  uint8_t neighborTotal = 0;

  if (this->x > 0)
  {
    neighborX[neighborTotal] = this->x - 1;
    neighborY[neighborTotal] = this->y;
    neighborTotal++;
  }
  if (this->x < (int8_t)(X_MAX - 1))
  {
    neighborX[neighborTotal] = this->x + 1;
    neighborY[neighborTotal] = this->y;
    neighborTotal++;
  }
  if (this->y > 0)
  {
    neighborX[neighborTotal] = this->x;
    neighborY[neighborTotal] = this->y - 1;
    neighborTotal++;
  }
  if (this->y < (int8_t)(Y_MAX - 1))
  {
    neighborX[neighborTotal] = this->x;
    neighborY[neighborTotal] = this->y + 1;
    neighborTotal++;
  }

  uint8_t choice = 0;
  if (random(0, WANDER_ONE_IN) == 0)
  {
    choice = (uint8_t)random(0, neighborTotal);
  }
  else
  {
    uint8_t minCount = 255;
    for (uint8_t i = 0; i < neighborTotal; i++)
    {
      uint8_t count = this->counts[(uint8_t)neighborY[i]][(uint8_t)neighborX[i]];
      if (count < minCount)
      {
        minCount = count;
      }
    }
    uint8_t candidates[NEIGHBOR_COUNT];
    uint8_t candidateTotal = 0;
    for (uint8_t i = 0; i < neighborTotal; i++)
    {
      if (this->counts[(uint8_t)neighborY[i]][(uint8_t)neighborX[i]] == minCount)
      {
        candidates[candidateTotal] = i;
        candidateTotal++;
      }
    }
    choice = candidates[random(0, candidateTotal)];
  }

  this->x = neighborX[choice];
  this->y = neighborY[choice];

  uint8_t *count = &this->counts[(uint8_t)this->y][(uint8_t)this->x];
  if (*count == 0)
  {
    this->visitedCells++;
  }
  if (*count < 255)
  {
    (*count)++;
  }

  if (this->visitedCells >= CELL_COUNT)
  {
    this->startHold();
  }
}

// hold and fade

void AutoWalkerPlugin::startHold()
{
  for (uint8_t y = 0; y < Y_MAX; y++)
  {
    for (uint8_t x = 0; x < X_MAX; x++)
    {
      this->display[y][x] = this->brightnessForCount(this->counts[y][x]);
    }
  }
  this->display[(uint8_t)this->y][(uint8_t)this->x] = BRIGHTNESS_WALKER;
  this->state = HOLD;
  this->holdTimer.reset();
}

void AutoWalkerPlugin::fadeStep()
{
  bool anyRemaining = false;
  for (uint8_t y = 0; y < Y_MAX; y++)
  {
    for (uint8_t x = 0; x < X_MAX; x++)
    {
      uint8_t value = this->display[y][x];
      if (value == 0)
      {
        continue;
      }
      value = (value > FADE_STEP) ? (uint8_t)(value - FADE_STEP) : 0;
      this->display[y][x] = value;
      if (value > 0)
      {
        anyRemaining = true;
      }
    }
  }
  if (!anyRemaining)
  {
    this->resetRun();
  }
}

// rendering

void AutoWalkerPlugin::render()
{
  Screen.clear();
  if (this->state == FADE)
  {
    for (uint8_t y = 0; y < Y_MAX; y++)
    {
      for (uint8_t x = 0; x < X_MAX; x++)
      {
        if (this->display[y][x] > 0)
        {
          Screen.setPixel(x, y, 1, this->display[y][x]);
        }
      }
    }
    return;
  }

  for (uint8_t y = 0; y < Y_MAX; y++)
  {
    for (uint8_t x = 0; x < X_MAX; x++)
    {
      if (this->counts[y][x] > 0)
      {
        Screen.setPixel(x, y, 1, this->brightnessForCount(this->counts[y][x]));
      }
    }
  }
  Screen.setPixel((uint8_t)this->x, (uint8_t)this->y, 1, BRIGHTNESS_WALKER);
}

// plugin lifecycle

void AutoWalkerPlugin::setup()
{
  Screen.clear();
  this->resetRun();
}

void AutoWalkerPlugin::loop()
{
  switch (this->state)
  {
  case RUN:
    if (!this->stepTimer.isReady(STEP_INTERVAL_MS))
    {
      return;
    }
    this->step();
    break;
  case HOLD:
    if (!this->holdTimer.isReady(HOLD_DURATION_MS))
    {
      return;
    }
    this->state = FADE;
    this->fadeTimer.reset();
    break;
  case FADE:
    if (!this->fadeTimer.isReady(FADE_INTERVAL_MS))
    {
      return;
    }
    this->fadeStep();
    break;
  }

  this->render();
}

const char *AutoWalkerPlugin::getName() const
{
  return "Auto Walker";
}
