#include "plugins/TetrisPlugin.h"

// pieces

void TetrisPlugin::cellOffset(uint8_t index, int8_t &dx, int8_t &dy)
{
  dx = index % 4;
  dy = index / 4;
}

bool TetrisPlugin::collides(uint8_t type, uint8_t rot, int8_t px, int8_t py,
                            const uint8_t f[16][16])
{
  uint16_t mask = PIECES[type][rot];
  for (uint8_t i = 0; i < 16; i++)
  {
    if ((mask & (1 << i)) == 0)
    {
      continue;
    }
    int8_t dx;
    int8_t dy;
    cellOffset(i, dx, dy);
    int8_t cx = px + dx;
    int8_t cy = py + dy;
    if (cx < 0 || cx >= FIELD_W || cy >= FIELD_H)
    {
      return true;
    }
    if (cy >= 0 && f[cy][cx] != 0)
    {
      return true;
    }
  }
  return false;
}

// ai

void TetrisPlugin::copyField(const uint8_t src[16][16], uint8_t dst[16][16])
{
  for (uint8_t y = 0; y < FIELD_H; y++)
  {
    for (uint8_t x = 0; x < FIELD_W; x++)
    {
      dst[y][x] = src[y][x];
    }
  }
}

uint8_t TetrisPlugin::columnHeight(const uint8_t f[16][16], uint8_t col)
{
  for (uint8_t y = 0; y < FIELD_H; y++)
  {
    if (f[y][col] != 0)
    {
      return FIELD_H - y;
    }
  }
  return 0;
}

uint16_t TetrisPlugin::countHoles(const uint8_t f[16][16])
{
  uint16_t holes = 0;
  for (uint8_t col = 0; col < FIELD_W; col++)
  {
    bool filled = false;
    for (uint8_t y = 0; y < FIELD_H; y++)
    {
      if (f[y][col] != 0)
      {
        filled = true;
      }
      else if (filled)
      {
        holes++;
      }
    }
  }
  return holes;
}

uint16_t TetrisPlugin::countFullRows(const uint8_t f[16][16])
{
  uint16_t rows = 0;
  for (uint8_t y = 0; y < FIELD_H; y++)
  {
    bool full = true;
    for (uint8_t x = 0; x < FIELD_W; x++)
    {
      if (f[y][x] == 0)
      {
        full = false;
        break;
      }
    }
    if (full)
    {
      rows++;
    }
  }
  return rows;
}

void TetrisPlugin::simulateDrop(uint8_t type, uint8_t rot, uint8_t left,
                                uint8_t out[16][16]) const
{
  int8_t py = SPAWN_Y;
  while (!collides(type, rot, left, py + 1, this->field))
  {
    py++;
  }
  copyField(this->field, out);
  uint16_t mask = PIECES[type][rot];
  for (uint8_t i = 0; i < 16; i++)
  {
    if ((mask & (1 << i)) == 0)
    {
      continue;
    }
    int8_t dx;
    int8_t dy;
    cellOffset(i, dx, dy);
    out[py + dy][left + dx] = BRIGHTNESS[type];
  }
}

int32_t TetrisPlugin::scorePlacement(const uint8_t f[16][16], uint16_t holesBefore)
{
  int32_t lines = countFullRows(f);
  int32_t holes = (int32_t)countHoles(f) - (int32_t)holesBefore;
  int32_t aggregateHeight = 0;
  int32_t bumpiness = 0;
  uint8_t prevHeight = 0;
  for (uint8_t c = 0; c < FIELD_W; c++)
  {
    uint8_t height = columnHeight(f, c);
    aggregateHeight += height;
    if (c > 0)
    {
      bumpiness += (height > prevHeight) ? (height - prevHeight) : (prevHeight - height);
    }
    prevHeight = height;
  }
  return 76 * lines - 256 * holes - 16 * aggregateHeight - 10 * bumpiness;
}

void TetrisPlugin::planAi()
{
  uint8_t sim[16][16];
  uint16_t holesBefore = countHoles(this->field);
  bool found = false;
  int32_t bestScore = 0;
  this->targetRot = this->pieceRot;
  this->targetX = this->pieceX;
  for (uint8_t rot = 0; rot < 4; rot++)
  {
    for (uint8_t col = 0; col < FIELD_W; col++)
    {
      uint8_t left = col;
      if (collides(this->pieceType, rot, left, SPAWN_Y, this->field))
      {
        continue;
      }
      simulateDrop(this->pieceType, rot, left, sim);
      int32_t score = scorePlacement(sim, holesBefore);
      if (!found || score > bestScore)
      {
        found = true;
        bestScore = score;
        this->targetRot = rot;
        this->targetX = left;
      }
    }
  }
}

// game logic

void TetrisPlugin::tickRunning()
{
  if (this->pieceRot < this->targetRot &&
      !collides(this->pieceType, this->pieceRot + 1, this->pieceX, this->pieceY,
                this->field))
  {
    this->pieceRot++;
    return;
  }
  if (this->pieceX != this->targetX)
  {
    int8_t dir = (this->targetX > this->pieceX) ? 1 : -1;
    if (!collides(this->pieceType, this->pieceRot, this->pieceX + dir, this->pieceY,
                  this->field))
    {
      this->pieceX += dir;
      return;
    }
  }
  this->dropStep();
}

void TetrisPlugin::dropStep()
{
  if (!collides(this->pieceType, this->pieceRot, this->pieceX, this->pieceY + 1,
                this->field))
  {
    this->pieceY++;
    return;
  }
  this->lockPiece();
}

void TetrisPlugin::lockPiece()
{
  uint16_t mask = PIECES[this->pieceType][this->pieceRot];
  for (uint8_t i = 0; i < 16; i++)
  {
    if ((mask & (1 << i)) == 0)
    {
      continue;
    }
    int8_t dx;
    int8_t dy;
    cellOffset(i, dx, dy);
    this->field[this->pieceY + dy][this->pieceX + dx] = BRIGHTNESS[this->pieceType];
  }
  this->startClearing();
}

void TetrisPlugin::startClearing()
{
  this->clearRowCount = 0;
  for (uint8_t y = 0; y < FIELD_H; y++)
  {
    bool full = true;
    for (uint8_t x = 0; x < FIELD_W; x++)
    {
      if (this->field[y][x] == 0)
      {
        full = false;
        break;
      }
    }
    if (full && this->clearRowCount < 4)
    {
      this->clearRows[this->clearRowCount++] = y;
    }
  }
  if (this->clearRowCount == 0)
  {
    this->spawnPiece();
    return;
  }
  this->seqToggles = 0;
  this->blinkVisible = true;
  this->gameState = CLEARING;
  this->seqTimer.reset();
}

bool TetrisPlugin::isClearingRow(uint8_t y) const
{
  for (uint8_t i = 0; i < this->clearRowCount; i++)
  {
    if (this->clearRows[i] == y)
    {
      return true;
    }
  }
  return false;
}

void TetrisPlugin::collapse()
{
  int8_t writeY = FIELD_H - 1;
  for (int8_t y = FIELD_H - 1; y >= 0; y--)
  {
    if (this->isClearingRow((uint8_t)y))
    {
      continue;
    }
    if (writeY != y)
    {
      for (uint8_t x = 0; x < 16; x++)
      {
        this->field[writeY][x] = this->field[y][x];
      }
    }
    writeY--;
  }
  for (; writeY >= 0; writeY--)
  {
    for (uint8_t x = 0; x < 16; x++)
    {
      this->field[writeY][x] = 0;
    }
  }
  this->linesTotal += this->clearRowCount;
  this->clearRowCount = 0;
  this->updateSpeed();
}

void TetrisPlugin::updateSpeed()
{
  int32_t interval = GRAVITY_BASE_MS - GRAVITY_STEP_MS * (int32_t)this->linesTotal;
  this->dropInterval = (interval < GRAVITY_MIN_MS) ? GRAVITY_MIN_MS : (uint16_t)interval;
}

void TetrisPlugin::spawnPiece()
{
  this->pieceType = random(0, PIECE_COUNT);
  this->pieceRot = 0;
  this->pieceX = SPAWN_X;
  this->pieceY = SPAWN_Y;
  this->planAi();
  if (collides(this->pieceType, this->pieceRot, this->pieceX, this->pieceY, this->field))
  {
    this->startGameOver();
    return;
  }
  this->gameState = RUNNING;
  this->gravityTimer.reset();
}

void TetrisPlugin::startGameOver()
{
  this->seqToggles = 0;
  this->fieldVisible = true;
  this->gameState = GAMEOVER;
  this->seqTimer.reset();
}

void TetrisPlugin::resetGame()
{
  for (uint8_t y = 0; y < FIELD_H; y++)
  {
    for (uint8_t x = 0; x < 16; x++)
    {
      this->field[y][x] = 0;
    }
  }
  this->linesTotal = 0;
  this->clearRowCount = 0;
  this->updateSpeed();
}

// rendering

void TetrisPlugin::render()
{
  Screen.clear();
  bool showField = !(this->gameState == GAMEOVER && !this->fieldVisible);
  if (showField)
  {
    for (uint8_t y = 0; y < FIELD_H; y++)
    {
      bool hideRow = (this->gameState == CLEARING) && !this->blinkVisible &&
                     this->isClearingRow(y);
      if (hideRow)
      {
        continue;
      }
      for (uint8_t x = 0; x < FIELD_W; x++)
      {
        if (this->field[y][x] != 0)
        {
          Screen.setPixel(x, y, 1, this->field[y][x]);
        }
      }
    }
  }
  if (this->gameState == RUNNING)
  {
    uint16_t mask = PIECES[this->pieceType][this->pieceRot];
    for (uint8_t i = 0; i < 16; i++)
    {
      if ((mask & (1 << i)) == 0)
      {
        continue;
      }
      int8_t dx;
      int8_t dy;
      cellOffset(i, dx, dy);
      Screen.setPixel(this->pieceX + dx, this->pieceY + dy, 1,
                      BRIGHTNESS[this->pieceType]);
    }
  }
}

// plugin lifecycle

void TetrisPlugin::setup()
{
  Screen.clear();
  this->resetGame();
  this->spawnPiece();
}

void TetrisPlugin::loop()
{
  switch (this->gameState)
  {
  case RUNNING:
    if (!this->gravityTimer.isReady(this->dropInterval))
    {
      return;
    }
    this->tickRunning();
    break;
  case CLEARING:
    if (!this->seqTimer.isReady(BLINK_INTERVAL_MS))
    {
      return;
    }
    this->blinkVisible = !this->blinkVisible;
    this->seqToggles++;
    if (this->seqToggles >= SEQ_TOGGLES)
    {
      this->collapse();
      this->spawnPiece();
    }
    break;
  case GAMEOVER:
    if (!this->seqTimer.isReady(FLASH_INTERVAL_MS))
    {
      return;
    }
    this->fieldVisible = !this->fieldVisible;
    this->seqToggles++;
    if (this->seqToggles >= SEQ_TOGGLES)
    {
      this->resetGame();
      this->spawnPiece();
    }
    break;
  }

  this->render();
}

const char *TetrisPlugin::getName() const
{
  return "Tetris";
}
