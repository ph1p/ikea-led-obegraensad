// copyright https://elektro.turanis.de/html/prj104/index.html
#include "mode/breakout.h"

void Breakout::initGame()
{
  Screen.clear();

  this->ballDelay = Breakout::BALL_DELAY_MAX;
  this->score = 0;
  this->level = 0;
  newLevel();
}

void Breakout::initBricks()
{
  this->destroyedBricks = 0;
  for (byte i = 0; i < Breakout::Breakout::BRICK_AMOUNT; i++)
  {
    if (currentMode == BREAKOUT)
    {
      this->bricks[i].x = i % Breakout::X_MAX;
      this->bricks[i].y = i / Breakout::X_MAX;
      Screen.setPixelAtIndex(this->bricks[i].y * Breakout::X_MAX + this->bricks[i].x, Breakout::LED_TYPE_ON);

      delay(25);
      Screen.render();
    }
  }
}

void Breakout::newLevel()
{
  this->initBricks();
  for (byte i = 0; i < Breakout::PADDLE_WIDTH; i++)
  {
    this->paddle[i].x = (Breakout::X_MAX / 2) - (Breakout::PADDLE_WIDTH / 2) + i;
    this->paddle[i].y = Breakout::Y_MAX - 1;
    Screen.setPixelAtIndex(this->paddle[i].y * Breakout::X_MAX + paddle[i].x, Breakout::LED_TYPE_ON);
  }
  this->ball.x = this->paddle[1].x;
  this->ball.y = this->paddle[1].y - 1;

  Screen.setPixelAtIndex(ball.y * Breakout::X_MAX + ball.x, Breakout::LED_TYPE_ON);
  this->ballMovement[0] = 1;
  this->ballMovement[1] = -1;
  this->lastBallUpdate = 0;

  Screen.render();
  this->level++;
  this->gameState = Breakout::GAME_STATE_RUNNING;
}

void Breakout::updateBall()
{
  if ((millis() - this->lastBallUpdate) < this->ballDelay)
  {
    return;
  }
  this->lastBallUpdate = millis();
  Screen.setPixelAtIndex(this->ball.y * Breakout::X_MAX + this->ball.x, Breakout::LED_TYPE_OFF);

  if (this->ballMovement[1] == 1)
  {
    // collision with bottom
    if (this->ball.y == (Breakout::Y_MAX - 1))
    {
      this->end();
      return;
    }
    this->checkPaddleCollision();
  }

  // collision detection with bricks
  for (byte i = 0; i < Breakout::BRICK_AMOUNT; i++)
  {
    if (this->bricks[i].x == this->ball.x && this->bricks[i].y == this->ball.y)
    {
      this->hitBrick(i);
      break;
    }
  }
  if (this->destroyedBricks >= Breakout::BRICK_AMOUNT)
  {
    this->gameState = Breakout::GAME_STATE_LEVEL;
    return;
  }

  // collision detection with wall
  if (this->ball.x <= 0 || this->ball.x >= (Breakout::X_MAX - 1))
  {
    this->ballMovement[0] *= -1;
  }
  if (this->ball.y <= 0)
  {
    this->ballMovement[1] *= -1;
  }

  this->ball.x += this->ballMovement[0];
  this->ball.y += this->ballMovement[1];

  Screen.setPixelAtIndex(this->ball.y * Breakout::X_MAX + this->ball.x, Breakout::LED_TYPE_ON);
  Screen.render();
}

void Breakout::hitBrick(byte i)
{
  this->bricks[i].x = -1;
  this->bricks[i].y = -1;
  // ballMovement[1] *= -1;
  this->score++;
  this->destroyedBricks++;
  if (this->ballDelay > Breakout::BALL_DELAY_MIN)
  {
    this->ballDelay -= Breakout::BALL_DELAY_STEP;
  }
  Screen.setPixelAtIndex(this->bricks[i].y * Breakout::X_MAX + this->bricks[i].x, Breakout::LED_TYPE_OFF);
}

void Breakout::checkPaddleCollision()
{
  if ((this->paddle[0].y - 1) != this->ball.y)
  {
    return;
  }

  // reverse movement direction on the edges
  if (this->ballMovement[0] == 1 && (this->paddle[0].x - 1) == this->ball.x ||
      this->ballMovement[0] == -1 && (this->paddle[Breakout::PADDLE_WIDTH - 1].x + 1) == this->ball.x)
  {
    this->ballMovement[0] *= -1;
    this->ballMovement[1] *= -1;

    return;
  }
  if (paddle[Breakout::PADDLE_WIDTH / 2].x == this->ball.x)
  {
    this->ballMovement[0] = 0;
    this->ballMovement[1] *= -1;

    return;
  }

  for (byte i = 0; i < Breakout::PADDLE_WIDTH; i++)
  {
    if (this->paddle[i].x == this->ball.x)
    {
      this->ballMovement[1] *= -1;
      if (random(2) == 0)
      {
        this->ballMovement[0] = 1;
      }
      else
      {
        this->ballMovement[0] = -1;
      }

      break;
    }
  }
}

void Breakout::updatePaddle()
{
  byte direction = random(0, 2) > 0 ? 1 : 2;

  unsigned int moveDirection = 0;
  if (direction == Breakout::DIRECTION_LEFT && this->paddle[0].x > 0)
  {
    moveDirection = -1;
  }
  if (direction == Breakout::DIRECTION_RIGHT && this->paddle[Breakout::PADDLE_WIDTH - 1].x < (Breakout::X_MAX - 1))
  {
    moveDirection = 1;
  }

  if (moveDirection != 0)
  {
    // turn off paddle LEDs
    for (byte i = 0; i < Breakout::PADDLE_WIDTH; i++)
    {
      Screen.setPixelAtIndex( this->paddle[i].y * Breakout::X_MAX + this->paddle[i].x, Breakout::LED_TYPE_OFF);
    }
    for (byte i = 0; i < Breakout::PADDLE_WIDTH; i++)
    {
      this->paddle[i].x += moveDirection;
    }
    for (byte i = 0; i < Breakout::PADDLE_WIDTH; i++)
    {
      Screen.setPixelAtIndex( this->paddle[i].y * Breakout::X_MAX + this->paddle[i].x, Breakout::LED_TYPE_ON);
    }
  }
  Screen.render();
}

void Breakout::end()
{
  // Serial.println("GAME OVER!");
  this->gameState = Breakout::GAME_STATE_END;
  Screen.setPixelAtIndex(this->ball.y * Breakout::X_MAX + this->ball.x, Breakout::LED_TYPE_ON);

  Screen.render();
  // Serial.println("Final score: " + String(score) + " in level " + String(level));
}

void Breakout::setup()
{
  Screen.clear();
  this->gameState = Breakout::GAME_STATE_END;
}

void Breakout::loop()
{
  listenOnButtonToChangeMode();
  switch (this->gameState)
  {
  case Breakout::GAME_STATE_LEVEL:
    this->newLevel();
    break;
  case Breakout::GAME_STATE_RUNNING:
    this->updateBall();
    this->updatePaddle();
    delay(300);
    break;
  case Breakout::GAME_STATE_END:
    this->initGame();
    break;
  }
}
