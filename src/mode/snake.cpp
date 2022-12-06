#include "mode/snake.h"

void Snake::initGame()
{
  Screen.clear();

  //this->ballDelay = Snake::BALL_DELAY_MAX;
  //this->score = 0;
  //this->level = 0;
  initSnake();
}

void Snake::initSnake()
{
  // this->destroyedBricks = 0;
  // for (byte i = 0; i < Snake::Snake::BRICK_AMOUNT; i++)
  // {
  //   if (currentMode == SNAKE)
  //   {
  //     this->bricks[i].x = i % Snake::X_MAX;
  //     this->bricks[i].y = i / Snake::X_MAX;
  //     Screen.setPixelAtIndex(this->bricks[i].y * Snake::X_MAX + this->bricks[i].x, Snake::LED_TYPE_ON);

  //     delay(25);
  //     Screen.render();
  //   }
  // }

  for (byte i = 0; i < Snake::Snake::LENGTH; i++)
  {
    Screen.setPixelAtIndex(241 + i, Snake::LED_TYPE_ON);
    delay(25);
    Screen.render();
  }

  newDot();

}

void Snake::newDot() {

  this->dot = random(1, 256);
  //todo: check if point is on snake - just testing if.
  if (this->dot > 241 || this->dot <  161 ) {
     Serial.println("Dot on Snake!");
    newDot();
    return;
  }
  Screen.setPixelAtIndex(this->dot, Snake::LED_TYPE_ON);
  delay(25);
  Screen.render();

  this->gameState = Snake::GAME_STATE_RUNNING;
}

void Snake::newLevel()
{
  //this->initBricks();
  for (byte i = 0; i < Snake::PADDLE_WIDTH; i++)
  {
    this->paddle[i].x = (Snake::X_MAX / 2) - (Snake::PADDLE_WIDTH / 2) + i;
    this->paddle[i].y = Snake::Y_MAX - 1;
    Screen.setPixelAtIndex(this->paddle[i].y * Snake::X_MAX + paddle[i].x, Snake::LED_TYPE_ON);
  }
  this->ball.x = this->paddle[1].x;
  this->ball.y = this->paddle[1].y - 1;

  Screen.setPixelAtIndex(ball.y * Snake::X_MAX + ball.x, Snake::LED_TYPE_ON);
  this->ballMovement[0] = 1;
  this->ballMovement[1] = -1;
  this->lastBallUpdate = 0;

  Screen.render();
  this->level++;
  this->gameState = Snake::GAME_STATE_RUNNING;
}

void Snake::updateBall()
{
  if ((millis() - this->lastBallUpdate) < this->ballDelay)
  {
    return;
  }
  this->lastBallUpdate = millis();
  Screen.setPixelAtIndex(this->ball.y * Snake::X_MAX + this->ball.x, Snake::LED_TYPE_OFF);

  if (this->ballMovement[1] == 1)
  {
    // collision with bottom
    if (this->ball.y == (Snake::Y_MAX - 1))
    {
      this->end();
      return;
    }
    this->checkPaddleCollision();
  }

  // collision detection with bricks
  for (byte i = 0; i < Snake::BRICK_AMOUNT; i++)
  {
    if (this->bricks[i].x == this->ball.x && this->bricks[i].y == this->ball.y)
    {
      this->hitBrick(i);
      break;
    }
  }
  if (this->destroyedBricks >= Snake::BRICK_AMOUNT)
  {
    this->gameState = Snake::GAME_STATE_LEVEL;
    return;
  }

  // collision detection with wall
  if (this->ball.x <= 0 || this->ball.x >= (Snake::X_MAX - 1))
  {
    this->ballMovement[0] *= -1;
  }
  if (this->ball.y <= 0)
  {
    this->ballMovement[1] *= -1;
  }

  this->ball.x += this->ballMovement[0];
  this->ball.y += this->ballMovement[1];

  Screen.setPixelAtIndex(this->ball.y * Snake::X_MAX + this->ball.x, Snake::LED_TYPE_ON);
  Screen.render();
}

void Snake::hitBrick(byte i)
{
  this->bricks[i].x = -1;
  this->bricks[i].y = -1;
  // ballMovement[1] *= -1;
  this->score++;
  this->destroyedBricks++;
  if (this->ballDelay > Snake::BALL_DELAY_MIN)
  {
    this->ballDelay -= Snake::BALL_DELAY_STEP;
  }
  Screen.setPixelAtIndex(this->bricks[i].y * Snake::X_MAX + this->bricks[i].x, Snake::LED_TYPE_OFF);
}

void Snake::checkPaddleCollision()
{
  if ((this->paddle[0].y - 1) != this->ball.y)
  {
    return;
  }

  // reverse movement direction on the edges
  if (this->ballMovement[0] == 1 && (this->paddle[0].x - 1) == this->ball.x ||
      this->ballMovement[0] == -1 && (this->paddle[Snake::PADDLE_WIDTH - 1].x + 1) == this->ball.x)
  {
    this->ballMovement[0] *= -1;
    this->ballMovement[1] *= -1;

    return;
  }
  if (paddle[Snake::PADDLE_WIDTH / 2].x == this->ball.x)
  {
    this->ballMovement[0] = 0;
    this->ballMovement[1] *= -1;

    return;
  }

  for (byte i = 0; i < Snake::PADDLE_WIDTH; i++)
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

void Snake::updatePaddle()
{
  byte direction = random(0, 2) > 0 ? 1 : 2;

  unsigned int moveDirection = 0;
  if (direction == Snake::DIRECTION_LEFT && this->paddle[0].x > 0)
  {
    moveDirection = -1;
  }
  if (direction == Snake::DIRECTION_RIGHT && this->paddle[Snake::PADDLE_WIDTH - 1].x < (Snake::X_MAX - 1))
  {
    moveDirection = 1;
  }

  if (moveDirection != 0)
  {
    // turn off paddle LEDs
    for (byte i = 0; i < Snake::PADDLE_WIDTH; i++)
    {
      Screen.setPixelAtIndex(this->paddle[i].y * Snake::X_MAX + this->paddle[i].x, Snake::LED_TYPE_OFF);
    }
    for (byte i = 0; i < Snake::PADDLE_WIDTH; i++)
    {
      this->paddle[i].x += moveDirection;
    }
    for (byte i = 0; i < Snake::PADDLE_WIDTH; i++)
    {
      Screen.setPixelAtIndex(this->paddle[i].y * Snake::X_MAX + this->paddle[i].x, Snake::LED_TYPE_ON);
    }
  }
  Screen.render();
}

void Snake::end()
{
  // Serial.println("GAME OVER!");
  this->gameState = Snake::GAME_STATE_END;
  Screen.setPixelAtIndex(this->ball.y * Snake::X_MAX + this->ball.x, Snake::LED_TYPE_ON);

  Screen.render();
  // Serial.println("Final score: " + String(score) + " in level " + String(level));
}

void Snake::setup()
{
  this->gameState = Snake::GAME_STATE_END;
}

void Snake::loop()
{
  listenOnButtonToChangeMode();
  switch (this->gameState)
  {
  case Snake::GAME_STATE_LEVEL:
    this->newLevel();
    break;
  case Snake::GAME_STATE_RUNNING:
    //this->updateBall();
    //this->updatePaddle();
    //delay(300);
    break;
  case Snake::GAME_STATE_END:
    this->initGame();
    break;
  }
}