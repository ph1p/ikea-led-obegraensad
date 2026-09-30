#include "plugins/FallingSandPlugin.h"

// helpers

void FallingSandPlugin::resetGrid()
{
  for (uint8_t y = 0; y < GRID_SIZE; y++)
  {
    for (uint8_t x = 0; x < GRID_SIZE; x++)
    {
      this->amount[y][x] = 0;
    }
  }
}

bool FallingSandPlugin::topRowOccupied() const
{
  for (uint8_t x = 0; x < GRID_SIZE; x++)
  {
    if (this->amount[0][x] > 0)
    {
      return true;
    }
  }
  return false;
}

uint16_t FallingSandPlugin::totalAmount() const
{
  uint16_t total = 0;
  for (uint8_t y = 0; y < GRID_SIZE; y++)
  {
    for (uint8_t x = 0; x < GRID_SIZE; x++)
    {
      total += this->amount[y][x];
    }
  }
  return total;
}

// spawning & physics

void FallingSandPlugin::wanderSpawnColumn()
{
  int8_t nextCol = (int8_t)this->spawnCol + (int8_t)random(-2, 3);
  if (nextCol < SPAWN_COL_MIN)
  {
    nextCol = SPAWN_COL_MIN;
  }
  if (nextCol > SPAWN_COL_MAX)
  {
    nextCol = SPAWN_COL_MAX;
  }
  this->spawnCol = (uint8_t)nextCol;
}

void FallingSandPlugin::spawnSand()
{
  if (this->amount[0][this->spawnCol] < MAX_AMOUNT)
  {
    this->amount[0][this->spawnCol]++;
  }
}

void FallingSandPlugin::applyPhysics()
{
  bool preferLeft = (random(0, 2) == 0);
  int8_t candidates[2] = {preferLeft ? (int8_t)-1 : (int8_t)1,
                          preferLeft ? (int8_t)1 : (int8_t)-1};
  for (int8_t y = GRID_SIZE - 2; y >= 0; y--)
  {
    for (uint8_t x = 0; x < GRID_SIZE; x++)
    {
      if (this->amount[y][x] == 0)
      {
        continue;
      }
      if (this->amount[y + 1][x] == 0)
      {
        this->amount[y][x]--;
        this->amount[y + 1][x]++;
        continue;
      }

      for (uint8_t i = 0; i < 2; i++)
      {
        int8_t dx = candidates[i];
        if ((dx < 0 && x == 0) || (dx > 0 && x >= GRID_SIZE - 1))
        {
          continue;
        }
        uint8_t targetX = x + dx;
        if (this->amount[y + 1][targetX] == 0)
        {
          this->amount[y][x]--;
          this->amount[y + 1][targetX]++;
          break;
        }
      }
    }
  }
}

// draining

void FallingSandPlugin::drainStep()
{
  for (uint8_t y = GRID_SIZE - 1; y > 0; y--)
  {
    for (uint8_t x = 0; x < GRID_SIZE; x++)
    {
      this->amount[y][x] = this->amount[y - 1][x];
    }
  }
  for (uint8_t x = 0; x < GRID_SIZE; x++)
  {
    this->amount[0][x] = 0;
  }
}

// rendering

void FallingSandPlugin::render()
{
  Screen.clear();
  for (uint8_t y = 0; y < GRID_SIZE; y++)
  {
    for (uint8_t x = 0; x < GRID_SIZE; x++)
    {
      if (this->amount[y][x] == 0)
      {
        continue;
      }
      Screen.setPixel(x, y, 1, BRIGHTNESS_FULL);
    }
  }
}

// plugin lifecycle

void FallingSandPlugin::setup()
{
  Screen.clear();
  this->resetGrid();
  this->spawnCol = SPAWN_COL_START;
  this->wanderIntervalMs = (uint16_t)random(WANDER_MIN_MS, WANDER_MAX_MS + 1);
  this->state = SPAWNING;
}

void FallingSandPlugin::loop()
{
  switch (this->state)
  {
  case SPAWNING:
    if (!this->tickTimer.isReady(TICK_INTERVAL_MS))
    {
      return;
    }
    if (this->wanderTimer.isReady(this->wanderIntervalMs))
    {
      this->wanderIntervalMs = (uint16_t)random(WANDER_MIN_MS, WANDER_MAX_MS + 1);
      this->wanderSpawnColumn();
    }
    if (this->spawnTimer.isReady(SPAWN_INTERVAL_MS))
    {
      this->spawnSand();
    }
    this->applyPhysics();
    this->render();
    if (this->topRowOccupied())
    {
      this->state = DRAINING;
    }
    break;
  case DRAINING:
    if (!this->drainTimer.isReady(DRAIN_INTERVAL_MS))
    {
      return;
    }
    this->drainStep();
    this->render();
    if (this->totalAmount() == 0)
    {
      this->spawnCol = SPAWN_COL_START;
      this->state = SPAWNING;
    }
    break;
  }
}

const char *FallingSandPlugin::getName() const
{
  return "Falling Sand";
}
