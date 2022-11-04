#ifndef BREAKOUT_HEADER_H
#define BREAKOUT_HEADER_H

#define DEBOUNCE_TIME 100 // in ms
#define X_MAX 16
#define Y_MAX 16
#define BRICK_AMOUNT X_MAX * 4
#define BALL_DELAY_MAX 250 // in ms
#define BALL_DELAY_MIN 100 // in ms
#define BALL_DELAY_STEP 5  // in ms
#define PADDLE_WIDTH 4
#define DIRECTION_NONE 0
#define DIRECTION_LEFT 1
#define DIRECTION_RIGHT 2
#define LED_TYPE_OFF 0
#define LED_TYPE_ON 1
#define GAME_STATE_RUNNING 1
#define GAME_STATE_END 2
#define GAME_STATE_LEVEL 3

void resetLEDs();
void initGame();
void initBricks();
void newLevel();
void updateBall();
void hitBrick(byte i);
void checkPaddleCollision();
void updatePaddle();
void endGame();
void breakoutSetup();
void breakoutLoop();

#endif