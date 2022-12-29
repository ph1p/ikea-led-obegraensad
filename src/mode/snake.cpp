#include "mode/snake.h"

void Snake::initGame()
{
  Screen.clear();

  //this->ballDelay = ;
  //this->score = 0;
  
  this->position = {240,241,242};
  for (const int &n: this->position)
  {
    Screen.setPixelAtIndex(n, Snake::LED_TYPE_ON);
  }

  Screen.render();
  newDot();
}

void Snake::newDot() {

  this->dot = random(0, 255);
  for (const int &n: this->position) {
    if (n == this->dot) { // todo: avoid next dot to be in direct front of snake
      newDot();
      return;
    }
  }

  Screen.setPixelAtIndex(this->dot, Snake::LED_TYPE_ON);
  Screen.render();
  
  this->gameState = Snake::GAME_STATE_RUNNING;
}

void Snake::findDirection() {

  // possible directions
  int up = 1;
  int down = 1;
  int left = 1;
  int right = 1;

  int snakesize = this->position.size();
  int snakehead = this->position[snakesize - 1];

  int up_pos = snakehead - 16;
  int down_pos = snakehead + 16;
  int left_pos = snakehead - 1;
  int right_pos = snakehead + 1;

  // remove possible directions by grid borders
  if (snakehead <= 15) { up = 0; }
  if (snakehead >= 240) { down = 0; }
  if (snakehead % 16 == 0) { left = 0; }
  if (snakehead % 16 == 1) { right = 0; }

  // remove possible directions by snake position
  for (const int &n: this->position) {
    if (up && n == up_pos) { up = 0; }
    if (down && n == down_pos) { down = 0; }
    if (left && n == left_pos) { left = 0; }
    if (right && n == right_pos) { right = 0; }
  }

  if (up) { Serial.println("snake can go up"); }
  if (down) { Serial.println("snake can go down"); }
  if (left) { Serial.println("snake can go left"); }
  if (right) { Serial.println("snake can go right"); }

  // so now we can move the snake random... but what about intelligent movement...?

  // left, right or stay in col?
  if (snakehead % 16 == this->dot % 16) {
    // stay in col
    left = 0; right = 0;
    Serial.println("Stay in col");
  } else if (snakehead % 16 > this->dot % 16) {
    // go left
    if (left) { left = snakehead % 16 - this->dot % 16; }
    right = 0;
    Serial.println("go left");
  } else {
    // go right
    if (right) { right = this->dot % 16 - snakehead % 16; }
    left = 0;
    Serial.println("go right");
  }

  // up, down or stay in row?
  if (floor(snakehead / 16) == floor(this->dot / 16)) {
    // stay in row
    up = 0;
    down = 0;
  } else if (floor(snakehead / 16) > floor(this->dot / 16)) {
    // go up
    if (up) { up = floor(snakehead / 16) - floor(this->dot / 16); }
    down = 0;
  } else {
    // go down
    if (down) { down = floor(this->dot / 16)- floor(snakehead / 16); }
    up = 0;
  }


  Serial.println(up);
  Serial.println(right);
  Serial.println(down);
  Serial.println(left);

  if (up == 0 && right == 0 && down == 0 && left == 0) {
    // killed yourself!
    // happens when snake is in the way to point
    Serial.println("congrats, you killed yourself!");
    Serial.println("Snake is: ");
    for (const int &n: this->position) {
      Serial.println(n);
    }
    end();
  } else if (up > right && up > down && up > left) {
    // go up!
    moveSnake(snakehead - 16);
  } else if (right > down && right > left && right > up) {
    // go right!
    moveSnake(snakehead + 1);
  } else if (down > left && down > up && down > right) {
    // go down!
    moveSnake(snakehead + 16);
  } else if (left > up && left > right && left > down) {
    // go left!
    moveSnake(snakehead - 1);
  } else {
    // hmm, should be the same distance - prio on up / down
    if (up) { moveSnake(snakehead - 16); } else if (down) { moveSnake(snakehead + 16); } else if (left) { moveSnake(snakehead - 1); } else { moveSnake(snakehead + 1); }
  }

}

void Snake::moveSnake(int newpos) 
{
  if (newpos == this->dot) {
    Serial.println("arrived dot!");
    this->position.push_back(newpos);
    newDot();
  } else {

    
    Screen.setPixelAtIndex(newpos, Snake::LED_TYPE_ON);
    this->position.push_back(newpos); // adding element (head) to snake

    Screen.setPixelAtIndex(this->position[0], Snake::LED_TYPE_OFF);
    this->position.erase(this->position.begin()); // removing first element (end) of snake
    
    Screen.render();
  }

}


void Snake::end()
{
  Serial.println("GAME OVER!");

  for (const int &n: this->position) { Screen.setPixelAtIndex(n, Snake::LED_TYPE_OFF); }
  Screen.render();
  delay(200);

  for (const int &n: this->position) { Screen.setPixelAtIndex(n, Snake::LED_TYPE_ON); }
  Screen.render();
  delay(200);

  for (const int &n: this->position) { Screen.setPixelAtIndex(n, Snake::LED_TYPE_OFF); }
  Screen.render();
  delay(200);

  for (const int &n: this->position) { Screen.setPixelAtIndex(n, Snake::LED_TYPE_ON); }
  Screen.render();
  delay(200);

  for (const int &n: this->position) { Screen.setPixelAtIndex(n, Snake::LED_TYPE_OFF); }
  Screen.render();
  delay(200);

  for (const int &n: this->position) { Screen.setPixelAtIndex(n, Snake::LED_TYPE_ON); }
  Screen.render();
  delay(500);

  for (const int &n: this->position) { Screen.setPixelAtIndex(n, Snake::LED_TYPE_OFF); Screen.render(); delay(200); }
  
  delay(200);
  Screen.setPixelAtIndex(this->dot, Snake::LED_TYPE_OFF);
  Screen.render();
  delay(500);

  this->gameState = Snake::GAME_STATE_END;

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
  case Snake::GAME_STATE_RUNNING:
    this->findDirection();
    delay(100);
    break;
  case Snake::GAME_STATE_END:
    this->initGame();
    break;
  }
}