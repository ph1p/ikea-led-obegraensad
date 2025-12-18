#include "plugins/SnakePlugin.h"
#include "constants.h"

void SnakePlugin::initGame()
{
  Screen.clear();

  this->position = {TOTAL_PIXELS - COLS, TOTAL_PIXELS - COLS + 1, TOTAL_PIXELS - COLS + 2};
  for (const int &n : this->position)
  {
    Screen.setPixelAtIndex(n, SnakePlugin::LED_TYPE_ON);
  }

  newDot();
}

void SnakePlugin::newDot()
{
  this->dot = random(0, TOTAL_PIXELS);
  for (const uint &n : this->position)
  {
    if (n == this->dot)
    {
      newDot();
      return;
    }
  }

  Screen.setPixelAtIndex(this->dot, SnakePlugin::LED_TYPE_ON, 40);

  this->gameState = SnakePlugin::GAME_STATE_RUNNING;
}

void SnakePlugin::findDirection()
{
  // possible directions
  uint up = 1;
  uint down = 1;
  uint left = 1;
  uint right = 1;

  int snakesize = this->position.size();
  uint snakehead = this->position[snakesize - 1];

  uint up_pos = snakehead - COLS;
  uint down_pos = snakehead + COLS;
  uint left_pos = snakehead - 1;
  uint right_pos = snakehead + 1;

  // remove possible directions by borders
  if (snakehead <= COLS - 1)
  {
    up = 0;
  }
  if (snakehead >= TOTAL_PIXELS - COLS)
  {
    down = 0;
  }
  if (snakehead % COLS == 0)
  {
    left = 0;
  }
  if (snakehead % COLS == COLS - 1)
  {
    right = 0;
  }

  // remove possible directions by snake position to avoid snake hitting
  for (const uint &n : this->position)
  {
    if (up && n == up_pos)
    {
      up = 0;
    }
    if (down && n == down_pos)
    {
      down = 0;
    }
    if (left && n == left_pos)
    {
      left = 0;
    }
    if (right && n == right_pos)
    {
      right = 0;
    }
  }

  // so now we can move the snake random... but what about intelligent movement...?
  uint bestway_up = up;
  uint bestway_down = down;
  uint bestway_left = left;
  uint bestway_right = right;

  // left, right or stay in col?
  if (snakehead % COLS == this->dot % COLS)
  {
    // stay in col
    bestway_left = 0;
    bestway_right = 0;
  }
  else if (snakehead % COLS > this->dot % COLS)
  {
    // go left
    if (bestway_left)
    {
      bestway_left = snakehead % COLS - this->dot % COLS;
    }
    bestway_right = 0;
  }
  else
  {
    // go right
    if (bestway_right)
    {
      bestway_right = this->dot % COLS - snakehead % COLS;
    }
    bestway_left = 0;
  }

  // up, down or stay in row?
  if (floor(snakehead / COLS) == floor(this->dot / COLS))
  {
    // stay in row
    bestway_up = 0;
    bestway_down = 0;
  }
  else if (floor(snakehead / COLS) > floor(this->dot / COLS))
  {
    // go up
    if (bestway_up)
    {
      bestway_up = floor(snakehead / COLS) - floor(this->dot / COLS);
    }
    bestway_down = 0;
  }
  else
  {
    // go down
    if (bestway_down)
    {
      bestway_down = floor(this->dot / COLS) - floor(snakehead / COLS);
    }
    bestway_up = 0;
  }

  // make the next step like the last if possible
  if (this->lastDirection == 1 && bestway_up)
  {
    moveSnake(snakehead - COLS);
    return;
  }
  else if (this->lastDirection == 2 && bestway_right)
  {
    moveSnake(snakehead + 1);
    return;
  }
  else if (this->lastDirection == 3 && bestway_down)
  {
    moveSnake(snakehead + COLS);
    return;
  }
  else if (this->lastDirection == 4 && bestway_left)
  {
    moveSnake(snakehead - 1);
    return;
  }

  // ok, redoing last step was not possible so ...
  // go to the best possible direction by distance
  if (bestway_up == 0 && bestway_right == 0 && bestway_down == 0 && bestway_left == 0)
  {
    // there are no good (bestway) directions, what about other possible directions?!
    if (up)
    {
      moveSnake(snakehead - COLS);
      this->lastDirection = 1;
    }
    else if (down)
    {
      moveSnake(snakehead + COLS);
      this->lastDirection = 3;
    }
    else if (left)
    {
      moveSnake(snakehead - 1);
      this->lastDirection = 4;
    }
    else if (right)
    {
      moveSnake(snakehead + 1);
      this->lastDirection = 2;
    }
    else
    {
      // killed yourself - no possible directions
      end();
    }
  }
  else if (bestway_up > bestway_right && bestway_up > bestway_down && bestway_up > bestway_left)
  {
    // go up!
    moveSnake(snakehead - COLS);
    this->lastDirection = 1;
  }
  else if (bestway_right > bestway_down && bestway_right > bestway_left && bestway_right > bestway_up)
  {
    // go right!
    moveSnake(snakehead + 1);
    this->lastDirection = 2;
  }
  else if (bestway_down > bestway_left && bestway_down > bestway_up && bestway_down > bestway_right)
  {
    // go down!
    moveSnake(snakehead + COLS);
    this->lastDirection = 3;
  }
  else if (bestway_left > bestway_up && bestway_left > bestway_right && bestway_left > bestway_down)
  {
    // go left!
    moveSnake(snakehead - 1);
    this->lastDirection = 4;
  }
  else
  {

    // hmm, same distance - prio on up -> down -> left -> right
    if (bestway_up)
    {
      moveSnake(snakehead - COLS);
      this->lastDirection = 1;
    }
    else if (bestway_down)
    {
      moveSnake(snakehead + COLS);
      this->lastDirection = 3;
    }
    else if (bestway_left)
    {
      moveSnake(snakehead - 1);
      this->lastDirection = 4;
    }
    else
    {
      moveSnake(snakehead + 1);
      this->lastDirection = 2;
    }
  }
}

void SnakePlugin::moveSnake(uint newpos)
{
  if (newpos == this->dot)
  {
    Screen.setPixelAtIndex(this->dot, SnakePlugin::LED_TYPE_ON);
    this->position.push_back(newpos);
    newDot();
  }
  else
  {

    Screen.setPixelAtIndex(newpos, SnakePlugin::LED_TYPE_ON);
    this->position.push_back(newpos); // adding element (head) to snake

    Screen.setPixelAtIndex(this->position[0], SnakePlugin::LED_TYPE_OFF);
    this->position.erase(this->position.begin()); // removing first element (end) of snake
  }
}

void SnakePlugin::end()
{
  this->gameState = SnakePlugin::GAME_STATE_DEATH_ANIMATION;
  this->animationStep = 0;
  this->animationTimer.reset();
}

void SnakePlugin::updateDeathAnimation()
{
  // Death animation sequence with non-blocking delays
  switch (this->animationStep)
  {
  case 0: // Blink off
  case 2:
  case 4:
    if (animationTimer.isReady(SnakePlugin::BLINK_SHORT_MS))
    {
      for (const int &n : this->position)
      {
        Screen.setPixelAtIndex(n, SnakePlugin::LED_TYPE_OFF);
      }
      this->animationStep++;
    }
    break;

  case 1: // Blink on
  case 3:
    if (animationTimer.isReady(SnakePlugin::BLINK_SHORT_MS))
    {
      for (const int &n : this->position)
      {
        Screen.setPixelAtIndex(n, SnakePlugin::LED_TYPE_ON);
      }
      this->animationStep++;
    }
    break;

  case 5: // Last blink on (longer)
    if (animationTimer.isReady(SnakePlugin::BLINK_SHORT_MS))
    {
      for (const int &n : this->position)
      {
        Screen.setPixelAtIndex(n, SnakePlugin::LED_TYPE_ON);
      }
      this->animationStep++;
    }
    break;

  case 6: // Wait before fade out
    if (animationTimer.isReady(SnakePlugin::BLINK_LONG_MS))
    {
      this->animationStep++;
    }
    break;

  case 7: // Fade out snake pixel by pixel
    if (animationTimer.isReady(SnakePlugin::BLINK_SHORT_MS))
    {
      if (this->position.size() > 0)
      {
        Screen.setPixelAtIndex(this->position[0], SnakePlugin::LED_TYPE_OFF);
        this->position.erase(this->position.begin());
      }
      else
      {
        this->animationStep++;
      }
    }
    break;

  case 8: // Turn off dot
    if (animationTimer.isReady(SnakePlugin::BLINK_SHORT_MS))
    {
      Screen.setPixelAtIndex(this->dot, SnakePlugin::LED_TYPE_OFF);
      this->animationStep++;
    }
    break;

  case 9: // Final delay
    if (animationTimer.isReady(SnakePlugin::BLINK_LONG_MS))
    {
      this->gameState = SnakePlugin::GAME_STATE_END;
      this->animationStep = 0;
    }
    break;
  }
}

void SnakePlugin::setup()
{
  this->gameState = SnakePlugin::GAME_STATE_END;
}

void SnakePlugin::loop()
{
  switch (this->gameState)
  {
  case SnakePlugin::GAME_STATE_RUNNING:
    if (moveTimer.isReady(SnakePlugin::SNAKE_DELAY_MS))
    {
      this->findDirection();
    }
    break;
  case SnakePlugin::GAME_STATE_DEATH_ANIMATION:
    updateDeathAnimation();
    break;
  case SnakePlugin::GAME_STATE_END:
    this->initGame();
    break;
  }
}

const char *SnakePlugin::getName() const
{
  return "Snake";
}
