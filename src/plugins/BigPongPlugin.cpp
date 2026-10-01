#include "plugins/BigPongPlugin.h"

// helpers

int8_t BigPongPlugin::clampPaddle(int8_t value)
{
  if (value < 0)
  {
    return 0;
  }
  if (value > (int8_t)(X_MAX - PADDLE_WIDTH))
  {
    return (int8_t)(X_MAX - PADDLE_WIDTH);
  }
  return value;
}

int8_t BigPongPlugin::rollAimError()
{
  int8_t magnitude = (int8_t)random(0, AIM_ERROR_MAX + 1);
  if (random(0, 100) < 20)
  {
    magnitude += (int8_t)random(1, 3);
  }
  return (random(0, 2) == 0) ? magnitude : -magnitude;
}

// ball

void BigPongPlugin::scaleBallSpeed()
{
  float current = sqrtf(vx * vx + vy * vy);
  if (current <= 0.0f)
  {
    return;
  }
  float speed = current * SPEED_UP;
  float maxSpeed = BASE_SPEED * MAX_SPEED_FACTOR;
  if (speed > maxSpeed)
  {
    speed = maxSpeed;
  }
  float scale = speed / current;
  vx *= scale;
  vy *= scale;
}

void BigPongPlugin::bounceOffPaddle(bool top)
{
  float centerX = (top ? topPaddleX : bottomPaddleX) + (PADDLE_WIDTH - 1) / 2.0f;
  vx += (bx - centerX) * NUDGE;
  by = top ? -by : 2.0f * PADDLE_BOTTOM_Y - by;
  vy = -vy;
  scaleBallSpeed();
  aimErrorTop = rollAimError();
  aimErrorBottom = rollAimError();
}

void BigPongPlugin::serve(float dirY)
{
  bx = 7.5f;
  by = 7.5f;
  float angle = radians((float)random(0, SERVE_ANGLE_MAX_DEG + 1));
  float side = (random(0, 2) == 0) ? 1.0f : -1.0f;
  vx = side * BASE_SPEED * sinf(angle);
  vy = dirY * BASE_SPEED * cosf(angle);
  aimErrorTop = rollAimError();
  aimErrorBottom = rollAimError();
}

void BigPongPlugin::startGoal(Player scorer)
{
  this->scoringPlayer = scorer;
  this->serveDirY = (scorer == PLAYER_TOP) ? 1 : -1;
  this->blinkVisible = true;
  this->blinkToggles = 0;
  this->gameState = GOAL_BLINK;
  this->seqTimer.reset();
}

void BigPongPlugin::tickGame()
{
  this->updateAi();

  bx += vx;
  by += vy;

  if (bx < 0.0f)
  {
    bx = 0.0f;
    vx = -vx;
  }
  else if (bx > (float)(X_MAX - 1))
  {
    bx = (float)(X_MAX - 1);
    vx = -vx;
  }

  int ballX = (int)(bx + 0.5f);

  if (by <= (float)PADDLE_TOP_Y && vy < 0.0f && ballX >= topPaddleX &&
      ballX <= topPaddleX + (int)PADDLE_WIDTH - 1)
  {
    bounceOffPaddle(true);
  }
  if (by >= (float)PADDLE_BOTTOM_Y && vy > 0.0f && ballX >= bottomPaddleX &&
      ballX <= bottomPaddleX + (int)PADDLE_WIDTH - 1)
  {
    bounceOffPaddle(false);
  }

  if (by < 0.0f)
  {
    this->startGoal(PLAYER_BOTTOM);
    return;
  }
  if (by > (float)PADDLE_BOTTOM_Y)
  {
    this->startGoal(PLAYER_TOP);
    return;
  }
}

// ai

int8_t BigPongPlugin::predictLanding(bool top) const
{
  float targetY = top ? (float)PADDLE_TOP_Y : (float)PADDLE_BOTTOM_Y;
  // step exactly like tickGame() does, so the prediction stays the same for
  // the whole flight and the paddle does not jitter between two targets
  float px = bx;
  float py = by;
  float pvx = vx;
  for (uint16_t i = 0; i < PREDICT_MAX_STEPS; i++)
  {
    if (top ? (py <= targetY) : (py >= targetY))
    {
      break;
    }
    px += pvx;
    py += vy;
    if (px < 0.0f)
    {
      px = 0.0f;
      pvx = -pvx;
    }
    else if (px > (float)(X_MAX - 1))
    {
      px = (float)(X_MAX - 1);
      pvx = -pvx;
    }
  }
  return clampPaddle((int8_t)(px + 0.5f));
}

void BigPongPlugin::updateAi()
{
  if (vy < 0.0f)
  {
    int8_t target = predictLanding(true) - (PADDLE_WIDTH - 1) / 2;
    topTargetX = clampPaddle(target + aimErrorTop);
  }
  else if (vy > 0.0f)
  {
    int8_t target = predictLanding(false) - (PADDLE_WIDTH - 1) / 2;
    bottomTargetX = clampPaddle(target + aimErrorBottom);
  }

  if (topPaddleX < topTargetX)
  {
    topPaddleX++;
  }
  else if (topPaddleX > topTargetX)
  {
    topPaddleX--;
  }
  if (bottomPaddleX < bottomTargetX)
  {
    bottomPaddleX++;
  }
  else if (bottomPaddleX > bottomTargetX)
  {
    bottomPaddleX--;
  }
}

// rendering

void BigPongPlugin::drawPaddleRow(int8_t x, uint8_t y)
{
  for (uint8_t i = 0; i < PADDLE_WIDTH; i++)
  {
    Screen.setPixel((uint8_t)x + i, y, 1, BRIGHTNESS_FULL);
  }
}

void BigPongPlugin::render()
{
  Screen.clear();

  for (uint8_t x = 0; x < X_MAX; x += 2)
  {
    Screen.setPixel(x, MID_LINE_Y, 1, BRIGHTNESS_FAINT);
  }

  bool blinkOff = (this->gameState == GOAL_BLINK) && !this->blinkVisible;
  if (!blinkOff || this->scoringPlayer != PLAYER_TOP)
  {
    this->drawPaddleRow(this->topPaddleX, PADDLE_TOP_Y);
  }
  if (!blinkOff || this->scoringPlayer != PLAYER_BOTTOM)
  {
    this->drawPaddleRow(this->bottomPaddleX, PADDLE_BOTTOM_Y);
  }

  bool drawBall = false;
  uint8_t ballY = (uint8_t)(by + 0.5f);
  if (this->gameState == RUNNING)
  {
    drawBall = true;
  }
  else if (this->gameState == GOAL_BLINK && this->blinkVisible)
  {
    drawBall = true;
    ballY = (this->scoringPlayer == PLAYER_BOTTOM) ? PADDLE_TOP_Y : PADDLE_BOTTOM_Y;
  }
  if (drawBall)
  {
    Screen.setPixel((uint8_t)(bx + 0.5f), ballY, 1, BRIGHTNESS_FULL);
  }
}

// plugin lifecycle

void BigPongPlugin::setup()
{
  Screen.clear();
  this->topPaddleX = clampPaddle((X_MAX - PADDLE_WIDTH) / 2);
  this->bottomPaddleX = this->topPaddleX;
  this->topTargetX = this->topPaddleX;
  this->bottomTargetX = this->bottomPaddleX;
  serve((random(0, 2) == 0) ? 1.0f : -1.0f);
  this->gameState = RUNNING;
}

void BigPongPlugin::loop()
{
  switch (this->gameState)
  {
  case RUNNING:
    if (!this->moveTimer.isReady(MOVE_INTERVAL_MS))
    {
      return;
    }
    this->tickGame();
    break;
  case GOAL_BLINK:
    if (!this->seqTimer.isReady(GOAL_BLINK_INTERVAL_MS))
    {
      return;
    }
    this->blinkVisible = !this->blinkVisible;
    this->blinkToggles++;
    if (this->blinkToggles >= GOAL_BLINK_TOGGLES)
    {
      this->gameState = SERVE_DELAY;
      this->seqTimer.reset();
    }
    break;
  case SERVE_DELAY:
    if (!this->seqTimer.isReady(SERVE_DELAY_MS))
    {
      return;
    }
    serve((float)this->serveDirY);
    this->gameState = RUNNING;
    break;
  }

  this->render();
}

const char *BigPongPlugin::getName() const
{
  return "BigPong";
}
