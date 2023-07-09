#include "plugins/SnakePlugin.h"

void SnakePlugin::initGame()
{
  Screen.clear();

  this->position = {240, 241, 242};
  for (const int &n : this->position)
  {
    Screen.setPixelAtIndex(n, SnakePlugin::LED_TYPE_ON);
  }

  newDot();
}

void SnakePlugin::newDot()
{
  this->dot = random(0, 255);
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

  uint up_pos = snakehead - 16;
  uint down_pos = snakehead + 16;
  uint left_pos = snakehead - 1;
  uint right_pos = snakehead + 1;

  // remove possible directions by borders
  if (snakehead <= 15)
  {
    up = 0;
  }
  if (snakehead >= 240)
  {
    down = 0;
  }
  if (snakehead % 16 == 0)
  {
    left = 0;
  }
  if (snakehead % 16 == 15)
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
  if (snakehead % 16 == this->dot % 16)
  {
    // stay in col
    bestway_left = 0;
    bestway_right = 0;
  }
  else if (snakehead % 16 > this->dot % 16)
  {
    // go left
    if (bestway_left)
    {
      bestway_left = snakehead % 16 - this->dot % 16;
    }
    bestway_right = 0;
  }
  else
  {
    // go right
    if (bestway_right)
    {
      bestway_right = this->dot % 16 - snakehead % 16;
    }
    bestway_left = 0;
  }

  // up, down or stay in row?
  if (floor(snakehead / 16) == floor(this->dot / 16))
  {
    // stay in row
    bestway_up = 0;
    bestway_down = 0;
  }
  else if (floor(snakehead / 16) > floor(this->dot / 16))
  {
    // go up
    if (bestway_up)
    {
      bestway_up = floor(snakehead / 16) - floor(this->dot / 16);
    }
    bestway_down = 0;
  }
  else
  {
    // go down
    if (bestway_down)
    {
      bestway_down = floor(this->dot / 16) - floor(snakehead / 16);
    }
    bestway_up = 0;
  }

  // make the next step like the last if possible
  if (this->lastDirection == 1 && bestway_up)
  {
    moveSnake(snakehead - 16);
    return;
  }
  else if (this->lastDirection == 2 && bestway_right)
  {
    moveSnake(snakehead + 1);
    return;
  }
  else if (this->lastDirection == 3 && bestway_down)
  {
    moveSnake(snakehead + 16);
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
      moveSnake(snakehead - 16);
      this->lastDirection = 1;
    }
    else if (down)
    {
      moveSnake(snakehead + 16);
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
      Serial.println("congrats, you killed yourself! (no possible movements)");
      end();
    }
  }
  else if (bestway_up > bestway_right && bestway_up > bestway_down && bestway_up > bestway_left)
  {
    // go up!
    moveSnake(snakehead - 16);
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
    moveSnake(snakehead + 16);
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
      moveSnake(snakehead - 16);
      this->lastDirection = 1;
    }
    else if (bestway_down)
    {
      moveSnake(snakehead + 16);
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
  Serial.println("GAME OVER!");

  for (const int &n : this->position)
  {
    Screen.setPixelAtIndex(n, SnakePlugin::LED_TYPE_OFF);
  }
  delay(200);

  for (const int &n : this->position)
  {
    Screen.setPixelAtIndex(n, SnakePlugin::LED_TYPE_ON);
  }
  delay(200);

  for (const int &n : this->position)
  {
    Screen.setPixelAtIndex(n, SnakePlugin::LED_TYPE_OFF);
  }
  delay(200);

  for (const int &n : this->position)
  {
    Screen.setPixelAtIndex(n, SnakePlugin::LED_TYPE_ON);
  }
  delay(200);

  for (const int &n : this->position)
  {
    Screen.setPixelAtIndex(n, SnakePlugin::LED_TYPE_OFF);
  }
  delay(200);

  for (const int &n : this->position)
  {
    Screen.setPixelAtIndex(n, SnakePlugin::LED_TYPE_ON);
  }
  delay(500);

  for (const int &n : this->position)
  {
    Screen.setPixelAtIndex(n, SnakePlugin::LED_TYPE_OFF);
    delay(200);
  }

  delay(200);
  Screen.setPixelAtIndex(this->dot, SnakePlugin::LED_TYPE_OFF);
  delay(500);

  this->gameState = SnakePlugin::GAME_STATE_END;
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
    this->findDirection();
    delay(100);
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
