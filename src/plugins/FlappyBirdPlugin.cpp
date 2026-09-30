#include "plugins/FlappyBirdPlugin.h"

// helpers

float FlappyBirdPlugin::clampVy(float value)
{
  if (value < VY_MIN)
  {
    return VY_MIN;
  }
  if (value > VY_MAX)
  {
    return VY_MAX;
  }
  return value;
}

bool FlappyBirdPlugin::isSolidRow(const Pipe &pipe, int y)
{
  return y < (int)pipe.gy || y >= (int)(pipe.gy + GAP_HEIGHT);
}

// pipes

void FlappyBirdPlugin::spawnPipe()
{
  if (this->pipeCount >= PIPE_MAX)
  {
    return;
  }
  Pipe &pipe = this->pipes[this->pipeCount];
  this->pipeCount++;
  pipe.x = SPAWN_X;
  pipe.gy = (uint8_t)random(GAP_TOP_MIN, GAP_TOP_MAX + 1);
}

void FlappyBirdPlugin::tickPipes()
{
  for (uint8_t i = 0; i < this->pipeCount; i++)
  {
    this->pipes[i].x--;
  }

  uint8_t writeIndex = 0;
  for (uint8_t i = 0; i < this->pipeCount; i++)
  {
    if (this->pipes[i].x >= 0)
    {
      this->pipes[writeIndex++] = this->pipes[i];
    }
  }
  this->pipeCount = writeIndex;

  if (this->pipeCount == 0 ||
      this->pipes[this->pipeCount - 1].x <= SPAWN_THRESHOLD_X)
  {
    this->spawnPipe();
  }
}

// physics

void FlappyBirdPlugin::tickPhysics()
{
  this->updateAi();

  this->vy = clampVy(this->vy + GRAVITY);
  this->by += this->vy;

  if (this->by < CEILING_Y)
  {
    this->by = CEILING_Y;
    if (this->vy < 0.0f)
    {
      this->vy = 0.0f;
    }
  }

  if (this->by > FLOOR_Y)
  {
    this->by = FLOOR_Y;
    this->startCrash();
    return;
  }
  if (this->birdHitsPipe())
  {
    this->startCrash();
  }
}

// ai

const FlappyBirdPlugin::Pipe *FlappyBirdPlugin::findControllingPipe() const
{
  const Pipe *controlling = nullptr;
  for (uint8_t i = 0; i < this->pipeCount; i++)
  {
    const Pipe &pipe = this->pipes[i];
    if ((int)pipe.x < (int)BIRD_X)
    {
      continue;
    }
    if (controlling == nullptr || pipe.x < controlling->x)
    {
      controlling = &pipe;
    }
  }
  return controlling;
}

void FlappyBirdPlugin::updateAi()
{
  float simY = this->by;
  float simV = this->vy;
  float projectedMinY = this->by;
  for (uint8_t t = 0; t < PREDICT_TICKS; t++)
  {
    simV = clampVy(simV + GRAVITY);
    simY += simV;
    if (simY < projectedMinY)
    {
      projectedMinY = simY;
    }
  }

  const Pipe *pipe = this->findControllingPipe();
  int dx = pipe == nullptr ? 0 : (int)pipe->x - (int)BIRD_X;
  bool flap = false;

  if (pipe != nullptr && dx >= 0 && dx <= (int)NEAR_DX_MAX)
  {
    float upper = (float)pipe->gy + BAND_UPPER_OFFSET;
    float lower = (float)pipe->gy + BAND_LOWER_OFFSET;
    bool inBand = this->by >= upper && this->by <= lower;
    bool gapEntrySafe = this->by > lower && projectedMinY > upper;
    bool floorRisk = simY > PIPE_FLOOR_RISK_Y && this->by > upper + 1.0f;
    bool risingIntoBand = this->vy < RISE_SUPPRESS_VY ||
                          (this->vy < 0.0f && this->by < upper + 1.0f);
    bool wantsFlap = inBand ? floorRisk : (gapEntrySafe || floorRisk);
    flap = wantsFlap && !risingIntoBand;
  }
  else
  {
    bool cruiseFall =
        this->by > CRUISE_FLAP_Y ||
        (this->by > CRUISE_HOLD_Y && this->vy > CRUISE_FALL_VY);
    bool floorRisk = simY > CRUISE_FLOOR_RISK_Y;
    flap = cruiseFall || floorRisk;
  }

  if (flap)
  {
    this->vy = FLAP_VY;
  }
}

// collision

bool FlappyBirdPlugin::birdHitsPipe() const
{
  int cell = (int)(this->by + 0.5f);
  for (uint8_t i = 0; i < this->pipeCount; i++)
  {
    const Pipe &pipe = this->pipes[i];
    if ((int)pipe.x == (int)BIRD_X && isSolidRow(pipe, cell))
    {
      return true;
    }
  }
  return false;
}

// crash sequence

void FlappyBirdPlugin::startCrash()
{
  this->blinkVisible = true;
  this->blinkToggles = 0;
  this->gameState = CRASH_BLINK;
  this->seqTimer.reset();
}

void FlappyBirdPlugin::resetGame()
{
  this->pipeCount = 0;
  this->by = BIRD_START_Y;
  this->vy = 0.0f;
  this->blinkVisible = true;
  this->blinkToggles = 0;
  this->physicsTimer.reset();
  this->pipeTimer.reset();
  this->gameState = RUNNING;
}

// rendering

void FlappyBirdPlugin::drawPipe(const Pipe &pipe)
{
  if (pipe.x < 0 || pipe.x > (int8_t)(X_MAX - 1))
  {
    return;
  }
  uint8_t gapTop = pipe.gy;
  uint8_t gapBottom = pipe.gy + GAP_HEIGHT;
  for (uint8_t y = 0; y < gapTop; y++)
  {
    Screen.setPixel((uint8_t)pipe.x, y, 1, BRIGHTNESS_HIGH);
  }
  for (uint8_t y = gapBottom; y < Y_MAX; y++)
  {
    Screen.setPixel((uint8_t)pipe.x, y, 1, BRIGHTNESS_HIGH);
  }
}

void FlappyBirdPlugin::render()
{
  Screen.clear();

  bool sceneVisible = (this->gameState != CRASH_BLINK) || this->blinkVisible;
  if (sceneVisible)
  {
    for (uint8_t i = 0; i < this->pipeCount; i++)
    {
      this->drawPipe(this->pipes[i]);
    }
    Screen.setPixel(BIRD_X, (uint8_t)(this->by + 0.5f), 1, BRIGHTNESS_FULL);
  }
}

// plugin lifecycle

void FlappyBirdPlugin::setup()
{
  Screen.clear();
  this->resetGame();
}

void FlappyBirdPlugin::loop()
{
  switch (this->gameState)
  {
  case RUNNING:
    if (this->pipeTimer.isReady(PIPE_TICK_MS))
    {
      this->tickPipes();
    }
    if (this->physicsTimer.isReady(TICK_MS))
    {
      this->tickPhysics();
    }
    break;
  case CRASH_BLINK:
    if (!this->seqTimer.isReady(BLINK_INTERVAL_MS))
    {
      return;
    }
    this->blinkVisible = !this->blinkVisible;
    this->blinkToggles++;
    if (this->blinkToggles >= BLINK_TOGGLES)
    {
      this->blinkVisible = true;
      this->blinkToggles = 0;
      this->gameState = CRASH_FALL;
    }
    break;
  case CRASH_FALL:
    if (!this->physicsTimer.isReady(TICK_MS))
    {
      return;
    }
    this->vy = clampVy(this->vy + GRAVITY);
    this->by += this->vy;
    if (this->by >= FLOOR_Y)
    {
      this->by = FLOOR_Y;
      this->gameState = CRASH_PAUSE;
      this->seqTimer.reset();
    }
    break;
  case CRASH_PAUSE:
    if (!this->seqTimer.isReady(CRASH_PAUSE_MS))
    {
      return;
    }
    this->resetGame();
    break;
  }

  this->render();
}

const char *FlappyBirdPlugin::getName() const
{
  return "Flappy Bird";
}
