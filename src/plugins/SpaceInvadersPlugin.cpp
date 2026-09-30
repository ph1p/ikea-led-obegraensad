#include "plugins/SpaceInvadersPlugin.h"

// helpers

int8_t SpaceInvadersPlugin::enemyX(uint8_t col) const
{
  return (int8_t)(COL_START_X + col * COL_SPACING) + this->colOffset;
}

int8_t SpaceInvadersPlugin::enemyY(uint8_t row) const
{
  return (int8_t)(BASE_ROW_Y + row * ROW_SPACING) + this->rowOffset;
}

// formation

void SpaceInvadersPlugin::spawnWave()
{
  for (uint8_t i = 0; i < ENEMY_ROWS; i++)
  {
    for (uint8_t j = 0; j < ENEMY_COLS; j++)
    {
      this->alive[i][j] = true;
    }
  }
  this->colOffset = 0;
  this->rowOffset = 0;
  this->marchDir = random(0, 2) ? 1 : -1;
  this->waveIntervalMs = (uint16_t)(MARCH_INTERVAL_MS * random(85, 116) / 100);
  this->enemiesAlive = ENEMY_ROWS * ENEMY_COLS;
  this->bulletActive = false;
}

void SpaceInvadersPlugin::updateFormation()
{
  bool atEdge = false;
  for (uint8_t i = 0; i < ENEMY_ROWS && !atEdge; i++)
  {
    for (uint8_t j = 0; j < ENEMY_COLS; j++)
    {
      if (!this->alive[i][j])
      {
        continue;
      }
      int8_t ex = this->enemyX(j);
      if (ex <= 0 || ex >= (int8_t)(X_MAX - 1))
      {
        atEdge = true;
        break;
      }
    }
  }
  if (atEdge)
  {
    this->rowOffset++;
    this->marchDir = -this->marchDir;
  }
  else
  {
    this->colOffset += this->marchDir;
  }

  for (uint8_t i = 0; i < ENEMY_ROWS; i++)
  {
    for (uint8_t j = 0; j < ENEMY_COLS; j++)
    {
      if (this->alive[i][j] && this->enemyY(i) >= (int8_t)GAME_OVER_Y)
      {
        this->startGameOver();
        return;
      }
    }
  }
}

// ship

void SpaceInvadersPlugin::updateShip()
{
  int8_t bestDist = 127;
  int8_t bestX = this->shipX;
  for (uint8_t j = 0; j < ENEMY_COLS; j++)
  {
    for (int8_t i = ENEMY_ROWS - 1; i >= 0; i--)
    {
      if (!this->alive[i][j])
      {
        continue;
      }
      int8_t ex = this->enemyX(j);
      int8_t dist = ex - this->shipX;
      if (dist < 0)
      {
        dist = -dist;
      }
      if (dist < bestDist)
      {
        bestDist = dist;
        bestX = ex;
      }
      break;
    }
  }
  this->targetX = bestX;

  if (this->shipX < this->targetX)
  {
    this->shipX++;
  }
  else if (this->shipX > this->targetX)
  {
    this->shipX--;
  }

  bool aligned = false;
  for (uint8_t j = 0; j < ENEMY_COLS && !aligned; j++)
  {
    for (int8_t i = ENEMY_ROWS - 1; i >= 0; i--)
    {
      if (!this->alive[i][j])
      {
        continue;
      }
      aligned = (this->enemyX(j) == this->shipX);
      break;
    }
  }

  if (!this->bulletActive && aligned)
  {
    this->bulletX = this->shipX;
    this->bulletY = BULLET_SPAWN_Y;
    this->bulletActive = true;
    this->bulletTimer.reset();
  }
}

// bullet

void SpaceInvadersPlugin::updateBullet()
{
  if (!this->bulletActive)
  {
    return;
  }
  this->bulletY--;
  if (this->bulletY < 0)
  {
    this->bulletActive = false;
    return;
  }
  for (uint8_t i = 0; i < ENEMY_ROWS; i++)
  {
    for (uint8_t j = 0; j < ENEMY_COLS; j++)
    {
      if (!this->alive[i][j])
      {
        continue;
      }
      if (this->enemyX(j) == this->bulletX && this->enemyY(i) == this->bulletY)
      {
        this->alive[i][j] = false;
        this->enemiesAlive--;
        this->bulletActive = false;
        if (this->enemiesAlive == 0)
        {
          this->startWaveClear();
        }
        return;
      }
    }
  }
}

// state transitions

void SpaceInvadersPlugin::startWaveClear()
{
  this->blinkVisible = true;
  this->blinkToggles = 0;
  this->gameState = WAVE_DELAY;
  this->seqTimer.reset();
}

void SpaceInvadersPlugin::startGameOver()
{
  this->blinkVisible = true;
  this->blinkToggles = 0;
  this->gameState = GAME_OVER_BLINK;
  this->seqTimer.reset();
}

// rendering

void SpaceInvadersPlugin::render()
{
  Screen.clear();

  if ((this->gameState == GAME_OVER_BLINK) && !this->blinkVisible)
  {
    return;
  }

  for (uint8_t i = 0; i < ENEMY_ROWS; i++)
  {
    for (uint8_t j = 0; j < ENEMY_COLS; j++)
    {
      if (!this->alive[i][j])
      {
        continue;
      }
      Screen.setPixel((uint8_t)this->enemyX(j), (uint8_t)this->enemyY(i), 1, BRIGHTNESS_MID);
    }
  }

  for (int8_t dx = -1; dx <= 1; dx++)
  {
    Screen.setPixel((uint8_t)(this->shipX + dx), SHIP_Y, 1, BRIGHTNESS_FULL);
  }
  Screen.setPixel((uint8_t)this->shipX, SHIP_Y - 1, 1, BRIGHTNESS_FULL);

  if (this->bulletActive)
  {
    Screen.setPixel((uint8_t)this->bulletX, (uint8_t)this->bulletY, 1, BRIGHTNESS_HIGH);
  }
}

// plugin lifecycle

void SpaceInvadersPlugin::setup()
{
  Screen.clear();
  this->wave = 1;
  this->shipX = SHIP_START_X;
  this->targetX = SHIP_START_X;
  this->blinkVisible = true;
  this->blinkToggles = 0;
  this->spawnWave();
  this->gameState = RUNNING;
}

void SpaceInvadersPlugin::loop()
{
  switch (this->gameState)
  {
  case RUNNING:
  {
    uint16_t effectiveInterval = (uint16_t)(this->waveIntervalMs * random(90, 111) / 100);
    if (this->marchTimer.isReady(effectiveInterval))
    {
      this->updateFormation();
      if (this->gameState != RUNNING)
      {
        break;
      }
    }
    if (this->moveTimer.isReady(MOVE_INTERVAL_MS))
    {
      this->updateShip();
    }
    if (this->bulletTimer.isReady(BULLET_INTERVAL_MS))
    {
      this->updateBullet();
    }
    break;
  }
  case GAME_OVER_BLINK:
    if (!this->seqTimer.isReady(BLINK_INTERVAL_MS))
    {
      return;
    }
    this->blinkVisible = !this->blinkVisible;
    this->blinkToggles++;
    if (this->blinkToggles >= BLINK_TOGGLES)
    {
      Screen.clear();
      this->wave = 1;
      this->shipX = SHIP_START_X;
      this->targetX = SHIP_START_X;
      this->spawnWave();
      this->gameState = RUNNING;
    }
    break;
  case WAVE_DELAY:
    if (!this->seqTimer.isReady(WAVE_PAUSE_MS))
    {
      return;
    }
    this->wave++;
    this->spawnWave();
    this->gameState = RUNNING;
    break;
  }

  this->render();
}

const char *SpaceInvadersPlugin::getName() const
{
  return "Space Invaders";
}
