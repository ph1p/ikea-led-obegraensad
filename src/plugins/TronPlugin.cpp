#include "plugins/TronPlugin.h"

// helpers

bool TronPlugin::cellFree(int8_t x, int8_t y) const
{
  if (x < 0 || x > (int8_t)(GRID_SIZE - 1) || y < 0 || y > (int8_t)(GRID_SIZE - 1))
  {
    return false;
  }
  return this->trail[y][x] == 0;
}

bool TronPlugin::cellFreeForSearch(int8_t x, int8_t y, int8_t excludeAX, int8_t excludeAY,
                                   int8_t excludeBX, int8_t excludeBY) const
{
  if (!this->cellFree(x, y))
  {
    return false;
  }
  if (x == excludeAX && y == excludeAY)
  {
    return false;
  }
  if (x == excludeBX && y == excludeBY)
  {
    return false;
  }
  return true;
}

uint16_t TronPlugin::floodFillCount(int8_t startX, int8_t startY, int8_t excludeAX, int8_t excludeAY,
                                    int8_t excludeBX, int8_t excludeBY) const
{
  static const int8_t NEIGHBOR_DX[4] = {-1, 1, 0, 0};
  static const int8_t NEIGHBOR_DY[4] = {0, 0, -1, 1};
  bool visited[TronPlugin::GRID_SIZE][TronPlugin::GRID_SIZE];
  int8_t queueX[TronPlugin::TRAIL_MAX];
  int8_t queueY[TronPlugin::TRAIL_MAX];
  uint16_t head = 0;
  uint16_t tail = 0;
  uint16_t count = 0;
  for (uint8_t y = 0; y < GRID_SIZE; y++)
  {
    for (uint8_t x = 0; x < GRID_SIZE; x++)
    {
      visited[y][x] = false;
    }
  }
  if (!this->cellFreeForSearch(startX, startY, excludeAX, excludeAY, excludeBX, excludeBY))
  {
    return 0;
  }
  queueX[tail] = startX;
  queueY[tail] = startY;
  tail++;
  visited[startY][startX] = true;
  while (head < tail && count < TronPlugin::TRAIL_MAX)
  {
    int8_t cx = queueX[head];
    int8_t cy = queueY[head];
    head++;
    count++;
    for (uint8_t i = 0; i < 4; i++)
    {
      int8_t nx = cx + NEIGHBOR_DX[i];
      int8_t ny = cy + NEIGHBOR_DY[i];
      if (!this->cellFreeForSearch(nx, ny, excludeAX, excludeAY, excludeBX, excludeBY))
      {
        continue;
      }
      if (visited[ny][nx])
      {
        continue;
      }
      visited[ny][nx] = true;
      queueX[tail] = nx;
      queueY[tail] = ny;
      tail++;
    }
  }
  return count;
}

uint8_t TronPlugin::riderBrightness(uint8_t idx)
{
  return (idx == RIDER_ONE) ? BRIGHTNESS_RIDER_1 : BRIGHTNESS_RIDER_2;
}

// riders

void TronPlugin::placeRider(uint8_t idx, int8_t x, int8_t y, int8_t dx, int8_t dy)
{
  this->riders[idx].x = x;
  this->riders[idx].y = y;
  this->riders[idx].dx = dx;
  this->riders[idx].dy = dy;
  this->trail[y][x] = riderBrightness(idx);
  this->histX[idx][0] = x;
  this->histY[idx][0] = y;
  this->histLen[idx] = 1;
}

void TronPlugin::resetBoard()
{
  for (uint8_t y = 0; y < GRID_SIZE; y++)
  {
    for (uint8_t x = 0; x < GRID_SIZE; x++)
    {
      this->trail[y][x] = 0;
    }
  }
  this->histLen[RIDER_ONE] = 0;
  this->histLen[RIDER_TWO] = 0;
  this->loser[RIDER_ONE] = false;
  this->loser[RIDER_TWO] = false;
  this->blinkVisible = true;
  this->blinkToggles = 0;
  placeRider(RIDER_ONE, 0, 5, 1, 0);
  placeRider(RIDER_TWO, GRID_SIZE - 1, 10, -1, 0);
  this->gameState = RUNNING;
  this->stepTimer.reset();
}

void TronPlugin::chooseMove(uint8_t idx, bool &hasTarget, int8_t &targetX, int8_t &targetY,
                            int8_t otherPendingX, int8_t otherPendingY)
{
  hasTarget = false;
  const RiderState &rider = this->riders[idx];
  const RiderState &other = this->riders[(idx == RIDER_ONE) ? RIDER_TWO : RIDER_ONE];
  const int8_t headings[MOVE_OPTION_COUNT][2] = {
      {rider.dx, rider.dy},
      {(int8_t)-rider.dy, rider.dx},
      {rider.dy, (int8_t)-rider.dx}};
  MoveOption options[MOVE_OPTION_COUNT];
  uint16_t scores[MOVE_OPTION_COUNT];
  uint8_t optionCount = 0;
  for (uint8_t i = 0; i < MOVE_OPTION_COUNT; i++)
  {
    int8_t tx = rider.x + headings[i][0];
    int8_t ty = rider.y + headings[i][1];
    if (!this->cellFree(tx, ty))
    {
      continue;
    }
    if (tx == other.x && ty == other.y)
    {
      continue;
    }
    uint16_t score = this->floodFillCount(tx, ty, other.x, other.y, otherPendingX, otherPendingY);
    options[optionCount].dx = headings[i][0];
    options[optionCount].dy = headings[i][1];
    options[optionCount].tx = tx;
    options[optionCount].ty = ty;
    scores[optionCount] = score;
    optionCount++;
  }
  if (optionCount == 0)
  {
    return;
  }
  uint16_t bestScore = scores[0];
  for (uint8_t i = 1; i < optionCount; i++)
  {
    if (scores[i] > bestScore)
    {
      bestScore = scores[i];
    }
  }
  uint8_t choice;
  int8_t straightIndex = -1;
  for (uint8_t i = 0; i < optionCount; i++)
  {
    if (options[i].dx == rider.dx && options[i].dy == rider.dy)
    {
      straightIndex = (int8_t)i;
      break;
    }
  }
  if (bestScore == 0)
  {
    choice = (uint8_t)random(0, optionCount);
  }
  else if (straightIndex >= 0 && scores[straightIndex] == bestScore && random(0, 100) < 50)
  {
    choice = (uint8_t)straightIndex;
  }
  else
  {
    uint8_t nearIndices[MOVE_OPTION_COUNT];
    uint8_t nearCount = 0;
    for (uint8_t i = 0; i < optionCount; i++)
    {
      if ((uint32_t)scores[i] * 10 >= (uint32_t)bestScore * 9)
      {
        nearIndices[nearCount] = i;
        nearCount++;
      }
    }
    choice = nearIndices[random(0, nearCount)];
  }
  hasTarget = true;
  targetX = options[choice].tx;
  targetY = options[choice].ty;
}

void TronPlugin::commitMove(uint8_t idx, int8_t targetX, int8_t targetY)
{
  RiderState &rider = this->riders[idx];
  rider.dx = targetX - rider.x;
  rider.dy = targetY - rider.y;
  rider.x = targetX;
  rider.y = targetY;
  this->trail[targetY][targetX] = riderBrightness(idx);
  uint16_t len = this->histLen[idx];
  this->histX[idx][len] = targetX;
  this->histY[idx][len] = targetY;
  this->histLen[idx] = len + 1;
}

void TronPlugin::tickRiders()
{
  bool hasTarget[RIDER_COUNT];
  int8_t targetX[RIDER_COUNT];
  int8_t targetY[RIDER_COUNT];
  for (uint8_t idx = 0; idx < RIDER_COUNT; idx++)
  {
    int8_t pendingX = -1;
    int8_t pendingY = -1;
    if (idx > RIDER_ONE && hasTarget[idx - 1])
    {
      pendingX = targetX[idx - 1];
      pendingY = targetY[idx - 1];
    }
    this->chooseMove(idx, hasTarget[idx], targetX[idx], targetY[idx], pendingX, pendingY);
  }
  if (hasTarget[RIDER_ONE] && hasTarget[RIDER_TWO] &&
      targetX[RIDER_ONE] == targetX[RIDER_TWO] &&
      targetY[RIDER_ONE] == targetY[RIDER_TWO])
  {
    hasTarget[RIDER_ONE] = false;
    hasTarget[RIDER_TWO] = false;
  }
  for (uint8_t idx = 0; idx < RIDER_COUNT; idx++)
  {
    if (hasTarget[idx])
    {
      this->commitMove(idx, targetX[idx], targetY[idx]);
    }
  }
  if (!hasTarget[RIDER_ONE] || !hasTarget[RIDER_TWO])
  {
    this->endRound(!hasTarget[RIDER_ONE], !hasTarget[RIDER_TWO]);
  }
}

void TronPlugin::endRound(bool loserOne, bool loserTwo)
{
  this->loser[RIDER_ONE] = loserOne;
  this->loser[RIDER_TWO] = loserTwo;
  this->blinkVisible = true;
  this->blinkToggles = 0;
  this->gameState = FLASH;
  this->seqTimer.reset();
}

void TronPlugin::rewindTick()
{
  for (uint8_t idx = 0; idx < RIDER_COUNT; idx++)
  {
    if (this->histLen[idx] > 0)
    {
      this->histLen[idx]--;
      uint8_t x = (uint8_t)this->histX[idx][this->histLen[idx]];
      uint8_t y = (uint8_t)this->histY[idx][this->histLen[idx]];
      this->trail[y][x] = 0;
    }
  }
  if (this->histLen[RIDER_ONE] == 0 && this->histLen[RIDER_TWO] == 0)
  {
    this->resetBoard();
  }
}

// rendering

void TronPlugin::render()
{
  Screen.clear();
  bool blinkOff = (this->gameState == FLASH) && !this->blinkVisible;
  for (uint8_t y = 0; y < GRID_SIZE; y++)
  {
    for (uint8_t x = 0; x < GRID_SIZE; x++)
    {
      uint8_t value = this->trail[y][x];
      if (value == 0)
      {
        continue;
      }
      if (blinkOff)
      {
        uint8_t owner = (value == BRIGHTNESS_RIDER_1) ? RIDER_ONE : RIDER_TWO;
        if (this->loser[owner])
        {
          continue;
        }
      }
      Screen.setPixel(x, y, 1, value);
    }
  }
}

// plugin lifecycle

void TronPlugin::setup()
{
  Screen.clear();
  this->resetBoard();
}

void TronPlugin::loop()
{
  switch (this->gameState)
  {
  case RUNNING:
    if (!this->stepTimer.isReady(STEP_INTERVAL_MS))
    {
      return;
    }
    this->tickRiders();
    break;
  case FLASH:
    if (!this->seqTimer.isReady(FLASH_INTERVAL_MS))
    {
      return;
    }
    this->blinkVisible = !this->blinkVisible;
    this->blinkToggles++;
    if (this->blinkToggles >= FLASH_TOGGLES)
    {
      this->gameState = REWIND;
      this->seqTimer.reset();
    }
    break;
  case REWIND:
    if (!this->seqTimer.isReady(REWIND_INTERVAL_MS))
    {
      return;
    }
    this->rewindTick();
    break;
  }

  this->render();
}

const char *TronPlugin::getName() const
{
  return "Tron";
}
