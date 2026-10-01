#include "plugins/BouncingBallPlugin.h"

void BouncingBallPlugin::generateLevel()
{
  obstacleCount = 0;
  bool reserved[GRID_HEIGHT][GRID_WIDTH] = {};
  uint8_t target = 6 + random(0, 5);

  for (uint8_t i = 0; i < target; i++)
  {
    bool isSegment = random(0, 2) == 0;
    for (uint8_t attempt = 0; attempt < 20; attempt++)
    {
      uint8_t len = isSegment ? random(3, 6) : 1;
      uint8_t x = isSegment ? random(0, GRID_WIDTH - len + 1) : random(0, GRID_WIDTH);
      uint8_t y = random(2, 14);

      bool free = true;
      for (uint8_t c = 0; c < len; c++)
      {
        if (reserved[y][x + c])
        {
          free = false;
          break;
        }
      }
      if (!free)
      {
        continue;
      }

      for (int8_t c = -1; c <= static_cast<int8_t>(len); c++)
      {
        for (int8_t dy = -1; dy <= 1; dy++)
        {
          int16_t rx = x + c;
          int16_t ry = y + dy;
          if (rx >= 0 && rx < GRID_WIDTH && ry >= 0 && ry < GRID_HEIGHT)
          {
            reserved[ry][rx] = true;
          }
        }
      }
      obstacles[obstacleCount] = {x, y, len};
      obstacleCount++;
      break;
    }
  }
}

void BouncingBallPlugin::spawnBall()
{
  posX = static_cast<float>(random(1, 15));
  posY = 0.0f;
  velY = 0.0f;
  float speed = random(18, 41) / 100.0f;
  velX = (random(0, 2) == 0) ? speed : -speed;
  levelStartTime = millis();
}

void BouncingBallPlugin::handleObstacleCollisions(uint8_t cellX, uint8_t cellY)
{
  float nearestX = posX < cellX ? cellX : (posX > cellX + 1.0f ? cellX + 1.0f : posX);
  float nearestY = posY < cellY ? cellY : (posY > cellY + 1.0f ? cellY + 1.0f : posY);
  float dx = posX - nearestX;
  float dy = posY - nearestY;
  if (dx * dx + dy * dy >= BALL_RADIUS * BALL_RADIUS)
  {
    return;
  }

  float centerX = cellX + 0.5f;
  float centerY = cellY + 0.5f;
  float penX = (posX < centerX) ? (posX + BALL_RADIUS - cellX) : (cellX + 1.0f - (posX - BALL_RADIUS));
  float penY = (posY < centerY) ? (posY + BALL_RADIUS - cellY) : (cellY + 1.0f - (posY - BALL_RADIUS));
  float jitter = (random(0, 2) == 0) ? JITTER : -JITTER;

  if (penX <= penY)
  {
    velX = -velX;
    velY += jitter;
    posX += (posX < centerX) ? -penX : penX;
  }
  else
  {
    velY = -velY;
    velX += jitter;
    posY += (posY < centerY) ? -penY : penY;
  }
}

void BouncingBallPlugin::runPhysicsTick()
{
  float gravity = (millis() - levelStartTime > GRAVITY_BOOST_AFTER_MS) ? BOOSTED_GRAVITY : GRAVITY;
  for (uint8_t substep = 0; substep < SUBSTEPS; substep++)
  {
    velY += gravity;
    posX += velX / SUBSTEPS;
    posY += velY / SUBSTEPS;

    if (posX < 0.0f)
    {
      posX = 0.0f;
      velX = -velX;
    }
    if (posX > GRID_WIDTH - 1.0f)
    {
      posX = GRID_WIDTH - 1.0f;
      velX = -velX;
    }
    if (posY < 0.0f)
    {
      posY = 0.0f;
      velY = -velY;
    }

    for (uint8_t i = 0; i < obstacleCount; i++)
    {
      for (uint8_t c = 0; c < obstacles[i].len; c++)
      {
        handleObstacleCollisions(obstacles[i].x + c, obstacles[i].y);
      }
    }

    if (posY >= 15.0f)
    {
      gameState = GameState::LevelEnd;
      return;
    }
  }
}

void BouncingBallPlugin::render()
{
  Screen.clear();
  for (uint8_t i = 0; i < obstacleCount; i++)
  {
    for (uint8_t c = 0; c < obstacles[i].len; c++)
    {
      Screen.setPixel(obstacles[i].x + c, obstacles[i].y, 1, obstacleBrightness);
    }
  }

  if (gameState == GameState::Run)
  {
    // the physics treats cell n as the area [n, n+1), rounding instead would
    // make a ball resting on a segment alternate between it and the row above
    int ballX = static_cast<int>(floorf(posX));
    int ballY = static_cast<int>(floorf(posY));
    if (ballX >= 0 && ballX < GRID_WIDTH && ballY >= 0 && ballY < GRID_HEIGHT)
    {
      Screen.setPixel(static_cast<uint8_t>(ballX), static_cast<uint8_t>(ballY), 1, FULL_BRIGHTNESS);
    }
  }
}

void BouncingBallPlugin::setup()
{
  Screen.clear();
  obstacleBrightness = MID_BRIGHTNESS;
  generateLevel();
  spawnBall();
  gameState = GameState::Run;
  render();
}

void BouncingBallPlugin::loop()
{
  switch (gameState)
  {
  case GameState::Run:
    if (physicsTimer.isReady(PHYSICS_INTERVAL_MS))
    {
      runPhysicsTick();
      render();
    }
    if (gameState == GameState::Run && millis() - levelStartTime > LEVEL_TIMEOUT_MS)
    {
      gameState = GameState::LevelEnd;
    }
    break;

  case GameState::LevelEnd:
    if (fadeTimer.isReady(FADE_INTERVAL_MS))
    {
      obstacleBrightness = (obstacleBrightness > FADE_STEP) ? obstacleBrightness - FADE_STEP : 0;
      if (obstacleBrightness == 0)
      {
        generateLevel();
        spawnBall();
        obstacleBrightness = MID_BRIGHTNESS;
        gameState = GameState::Run;
      }
      render();
    }
    break;
  }
}

const char *BouncingBallPlugin::getName() const
{
  return "Bouncing Ball";
}
