// copyright https://elektro.turanis.de/html/prj104/index.html
#include "mode/breakout.h"

struct Coords
{
  byte x;
  byte y;
};

byte gameState;
byte level;
byte destroyedBricks;
Coords paddle[PADDLE_WIDTH];
Coords bricks[BRICK_AMOUNT];
Coords ball;

int ballMovement[2];
unsigned int ballDelay;
unsigned int score;
unsigned long lastBallUpdate = 0;

void initGame()
{
  clearScreenAndBuffer(mode_buffer);

  ballDelay = BALL_DELAY_MAX;
  score = 0;
  level = 0;
  newLevel();
}

void initBricks()
{
  destroyedBricks = 0;
  for (byte i = 0; i < BRICK_AMOUNT; i++)
  {
    if (currentMode == BREAKOUT)
    {
      bricks[i].x = i % X_MAX;
      bricks[i].y = i / X_MAX;
      setPixelAtIndex(mode_buffer, bricks[i].y * X_MAX + bricks[i].x, LED_TYPE_ON);

      delay(25);
      renderScreen(mode_buffer);
    }
  }
}

void newLevel()
{
  initBricks();
  for (byte i = 0; i < PADDLE_WIDTH; i++)
  {
    paddle[i].x = (X_MAX / 2) - (PADDLE_WIDTH / 2) + i;
    paddle[i].y = Y_MAX - 1;
    setPixelAtIndex(mode_buffer, paddle[i].y * X_MAX + paddle[i].x, LED_TYPE_ON);
  }
  ball.x = paddle[1].x;
  ball.y = paddle[1].y - 1;

  setPixelAtIndex(mode_buffer, ball.y * X_MAX + ball.x, LED_TYPE_ON);
  ballMovement[0] = 1;
  ballMovement[1] = -1;
  lastBallUpdate = 0;

  renderScreen(mode_buffer);
  level++;
  gameState = GAME_STATE_RUNNING;
}

void updateBall()
{
  if ((millis() - lastBallUpdate) < ballDelay)
  {
    return;
  }
  lastBallUpdate = millis();
  setPixelAtIndex(mode_buffer, ball.y * X_MAX + ball.x, LED_TYPE_OFF);

  if (ballMovement[1] == 1)
  {
    // collision with bottom
    if (ball.y == (Y_MAX - 1))
    {
      endGame();
      return;
    }
    checkPaddleCollision();
  }

  // collision detection with bricks
  for (byte i = 0; i < BRICK_AMOUNT; i++)
  {
    if (bricks[i].x == ball.x && bricks[i].y == ball.y)
    {
      hitBrick(i);
      break;
    }
  }
  if (destroyedBricks >= BRICK_AMOUNT)
  {
    gameState = GAME_STATE_LEVEL;
    return;
  }

  // collision detection with wall
  if (ball.x <= 0 || ball.x >= (X_MAX - 1))
  {
    ballMovement[0] *= -1;
  }
  if (ball.y <= 0)
  {
    ballMovement[1] *= -1;
  }

  ball.x += ballMovement[0];
  ball.y += ballMovement[1];

  setPixelAtIndex(mode_buffer, ball.y * X_MAX + ball.x, LED_TYPE_ON);
  renderScreen(mode_buffer);
}

void hitBrick(byte i)
{
  bricks[i].x = -1;
  bricks[i].y = -1;
  // ballMovement[1] *= -1;
  score++;
  destroyedBricks++;
  if (ballDelay > BALL_DELAY_MIN)
  {
    ballDelay -= BALL_DELAY_STEP;
  }
  setPixelAtIndex(mode_buffer, bricks[i].y * X_MAX + bricks[i].x, LED_TYPE_OFF);
}

void checkPaddleCollision()
{
  if ((paddle[0].y - 1) != ball.y)
  {
    return;
  }

  // reverse movement direction on the edges
  if (ballMovement[0] == 1 && (paddle[0].x - 1) == ball.x ||
      ballMovement[0] == -1 && (paddle[PADDLE_WIDTH - 1].x + 1) == ball.x)
  {
    ballMovement[0] *= -1;
    ballMovement[1] *= -1;

    return;
  }
  if (paddle[PADDLE_WIDTH / 2].x == ball.x)
  {
    ballMovement[0] = 0;
    ballMovement[1] *= -1;

    return;
  }

  for (byte i = 0; i < PADDLE_WIDTH; i++)
  {
    if (paddle[i].x == ball.x)
    {
      ballMovement[1] *= -1;
      if (random(2) == 0)
      {
        ballMovement[0] = 1;
      }
      else
      {
        ballMovement[0] = -1;
      }

      break;
    }
  }
}

void updatePaddle()
{
  byte direction = random(0, 2) > 0 ? 1 : 2;

  unsigned int moveDirection = 0;
  if (direction == DIRECTION_LEFT && paddle[0].x > 0)
  {
    moveDirection = -1;
  }
  if (direction == DIRECTION_RIGHT && paddle[PADDLE_WIDTH - 1].x < (X_MAX - 1))
  {
    moveDirection = 1;
  }

  if (moveDirection != 0)
  {
    // turn off paddle LEDs
    for (byte i = 0; i < PADDLE_WIDTH; i++)
    {
      setPixelAtIndex(mode_buffer, paddle[i].y * X_MAX + paddle[i].x, LED_TYPE_OFF);
    }
    for (byte i = 0; i < PADDLE_WIDTH; i++)
    {
      paddle[i].x += moveDirection;
    }
    for (byte i = 0; i < PADDLE_WIDTH; i++)
    {
      setPixelAtIndex(mode_buffer, paddle[i].y * X_MAX + paddle[i].x, LED_TYPE_ON);
    }
  }
  renderScreen(mode_buffer);
}

void endGame()
{
  // Serial.println("GAME OVER!");
  gameState = GAME_STATE_END;
  setPixelAtIndex(mode_buffer, ball.y * X_MAX + ball.x, LED_TYPE_ON);

  renderScreen(mode_buffer);
  // Serial.println("Final score: " + String(score) + " in level " + String(level));
}

void breakoutSetup()
{
  clearScreenAndBuffer(mode_buffer);
  gameState = GAME_STATE_END;
}

void breakoutLoop()
{
  switch (gameState)
  {
  case GAME_STATE_LEVEL:
    newLevel();
    break;
  case GAME_STATE_RUNNING:
    updateBall();
    updatePaddle();
    delay(300);
    break;
  case GAME_STATE_END:
    initGame();
    break;
  }
}
