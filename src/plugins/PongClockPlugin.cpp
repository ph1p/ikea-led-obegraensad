#include "plugins/PongClockPlugin.h"

/************************************************
  PongClock

  based on the awesome work of Jeremy Williams
  https://github.com/Jerware/GameFrameV2
  LEDSEQ.COM
*************************************************/
void PongClockPlugin::drawCharacter(int x, int y, std::vector<int> bits, int bitCount, uint8_t brightness)
{
  for (int i = 0; i < bits.size(); i += bitCount)
  {
    for (int j = 0; j < bitCount; j++)
    {
      int xPos = (x+j-1);
      int yPos = y + (i / bitCount);
      if (xPos >= 0 && xPos < X_MAX && yPos >= 0 && yPos < Y_MAX) {
        Screen.setPixel(xPos, yPos, bits[i + j], brightness);
      }
    }
  }
}

long PongClockPlugin::realRandom(int max)
{
  if (0 == max)
  {
    return 0;
  }
  long rand = random(0, max);
  return rand;
}

int PongClockPlugin::realRandom(int min, int max)
{
  if (min >= max)
  {
    return min;
  }
  return realRandom(max - min) + min;
}

void PongClockPlugin::drawDigits()
{
  drawCharacter(0, 0, Screen.readBytes(smallNumbers[(current_hour - current_hour % 10) / 10]), 4, 100);
  drawCharacter(4, 0, Screen.readBytes(smallNumbers[current_hour % 10]), 4, 100);
  drawCharacter(9, 0, Screen.readBytes(smallNumbers[(current_minute - current_minute % 10) / 10]), 4, 100);
  drawCharacter(13, 0, Screen.readBytes(smallNumbers[current_minute % 10]), 4, 100);
}

float PongClockPlugin::degToRad(float deg)
{
  float result;
  result = deg * 3.14159265359 / 180;
  return result;
}

byte PongClockPlugin::getScreenIndex(byte x, byte y)
{
  byte screenX = x / X_MAX;
  byte screenY = y / Y_MAX;
  byte index;
  index = screenY * Y_MAX;
  if (screenY == 0)
  {
    index = X_MAX - 1 - screenX;
  }
  else if (screenY % 2 != 0)
  {
    index = (screenY * Y_MAX) + screenX;
  }
  else
  {
    index = (screenY * Y_MAX + (Y_MAX - 1)) - screenX;
  }
  return index;
}

int PongClockPlugin::pong_predict_y(int x, int y, int angle)
{
  while (x >= X_MAX && x <= 256 - X_MAX)
  {
    if ((y + cos(degToRad(angle)) * X_MAX) + .5 < 0)
    {
      if (angle > 90 && angle < 270)
      {
        if (angle > 180)
          angle = 360 - (angle - 180);
        else
          angle = 90 - (angle - 90);
      }
      else
      {
        if (angle < 90)
          angle = 90 + (90 - angle);
        else
          angle = 180 + (360 - angle);
      }
    }
    else if ((y + cos(degToRad(angle)) * Y_MAX) + .5 > 256)
    {
      if (angle > 90 && angle < 270)
      {
        if (angle > 180)
          angle = 360 - (angle - 180);
        else
          angle = 90 - (angle - 90);
      }
      else
      {
        if (angle < 90)
          angle = 90 + (90 - angle);
        else
          angle = 180 + (360 - angle);
      }
    }
    x = x + sin(degToRad(angle)) * X_MAX + .5;
    y = y + cos(degToRad(angle)) * Y_MAX + .5;
  }
  return y;
}

void PongClockPlugin::swapXdirection()
{
  if (ballAngle < 180)
  {
    if (ballAngle < 90)
    {
      ballAngle = 270 + (90 - ballAngle);
    }
    else
      ballAngle = 270 - (ballAngle - 90);
  }
  else
  {
    if (ballAngle > 270)
    {
      ballAngle = 360 - ballAngle;
    }
    else
      ballAngle = 180 - (ballAngle - 180);
  }
}

void PongClockPlugin::swapYdirection()
{
  if (ballAngle > 90 && ballAngle < 270)
  {
    if (ballAngle > 180)
    {
      ballAngle = 360 - (ballAngle - 180);
    }
    else
    {
      ballAngle = 90 - (ballAngle - 90);
    }
  }
  else
  {
    if (ballAngle < 90)
    {
      ballAngle = 90 + (90 - ballAngle);
    }
    else
    {
      ballAngle = 180 + (360 - ballAngle);
    }
  }
}

void PongClockPlugin::reset()
{
  pongReset = false;
  pongCelebrate = false;
  pongScoredHour = false;
  pongScoredMinute = false;
  pongPaddleLeftY = 128;
  pongPaddleLeftStart = 128;
  pongPaddleLeftTarget = 128;
  pongPaddleRightY = 128;
  pongPaddleRightStart = 128;
  pongPaddleRightTarget = 128;
  ballX = 255 - X_MAX;
  ballY = 128;
  ballAngle = realRandom(225, 315);
  pongBallDirection = 1;
  pongPaddleRightTarget = constrain(pong_predict_y(ballX, ballY, ballAngle), X_MAX, 255 - X_MAX);
  ballBrightness = 255;
  ballBrightnessStep = -1;
  drawDigits();
}

void PongClockPlugin::setup()
{
  Screen.clear();
  if (getLocalTime(&timeinfo))
  {
    current_hour = timeinfo.tm_hour;
    current_minute = timeinfo.tm_min;
    previousMinutes = current_minute;
    previousHour = current_hour;
  }
  reset();
}

void PongClockPlugin::loop()
{
  unsigned long currentMillis = millis();

  if (!pongCelebrate)
  {
    if (currentMillis > pongShowtime)
    {

      if (getLocalTime(&timeinfo))
      {
        if (previousHour != timeinfo.tm_hour)
        {
          pongScoredHour = true;
        }
        else if (previousMinutes != timeinfo.tm_min)
        {
          pongScoredMinute = true;
        }
        previousMinutes = timeinfo.tm_min;
        previousHour = timeinfo.tm_hour;
      }

      pongShowtime = currentMillis + PongClockPlugin::SPEED;

      // left side
      if ((ballX + sin(degToRad(ballAngle)) * X_MAX) + .5 > 256 - X_MAX)
      {
        if (!pongScoredMinute)
        {
          if (getScreenIndex(ballX, ballY) < getScreenIndex(255, pongPaddleLeftY))
            ballAngle = realRandom(225 - 25, 225 + 25);
          else if (getScreenIndex(ballX, ballY) == getScreenIndex(255, pongPaddleLeftY) + 1)
            swapXdirection();
          else if (getScreenIndex(ballX, ballY) == getScreenIndex(255, pongPaddleLeftY) - 1)
            swapXdirection();
          else
            ballAngle = realRandom(315 - 25, 315 + 25);
          pongBallDirection = 1;
          pongPaddleRightStart = pongPaddleRightY;
          pongPaddleRightTarget = constrain(pong_predict_y(ballX, ballY, ballAngle), Y_MAX, 255 - Y_MAX);
        }
        else
        {
          pongCelebrate = true;
          pongCelebrationEnd = currentMillis + 2000;
        }
      }
      // right side
      else if ((ballX + sin(degToRad(ballAngle)) * Y_MAX) + .5 < Y_MAX)
      {
        if (!pongScoredHour)
        {
          if (getScreenIndex(ballX, ballY) < getScreenIndex(0, pongPaddleRightY))
            ballAngle = realRandom(135 - 25, 135 + 25);
          else if (getScreenIndex(ballX, ballY) == getScreenIndex(0, pongPaddleRightY) + 1)
            swapXdirection();
          else if (getScreenIndex(ballX, ballY) == getScreenIndex(0, pongPaddleRightY) - 1)
            swapXdirection();
          else
            ballAngle = realRandom(45 - 25, 45 + 25);
          pongBallDirection = 0;
          pongPaddleLeftStart = pongPaddleLeftY;
          pongPaddleLeftTarget = constrain(pong_predict_y(ballX, ballY, ballAngle), Y_MAX, 255 - Y_MAX);
        }
        else
        {
          pongCelebrate = true;
          pongCelebrationEnd = currentMillis + 2000;
        }
      }

      if ((ballY + cos(degToRad(ballAngle)) * Y_MAX) + .5 < 0)
        swapYdirection();
      else if ((ballY + cos(degToRad(ballAngle)) * Y_MAX) + .5 > 256)
        swapYdirection();

      ballX = ballX + sin(degToRad(ballAngle)) * X_MAX + .5;
      ballY = ballY + cos(degToRad(ballAngle)) * Y_MAX + .5;
    }
  }
  else if (currentMillis > pongCelebrationEnd)
  {
    pongReset = true;
    // Switch to new minute after celebration ends so score is updated afterwards
    current_hour = timeinfo.tm_hour;
    current_minute = timeinfo.tm_min;
  }

  if (pongReset)
  {
    reset();
  }

  // Blink the ball
  if (pongCelebrate)
  { 
    if (currentMillis > nextUpdateMillis) {
      if (ballBrightness == 255)
        ballBrightness = 0;
      else
        ballBrightness = 255;
      nextUpdateMillis = currentMillis + 100;
    }
  }

  // manage paddles
  if (pongBallDirection == 0)
  {
    int offset = 0;
    if (pongScoredMinute)
    {
      if (pongPaddleLeftTarget < 3 * X_MAX)
        offset = 2 * X_MAX;
      else
        offset = -2 * X_MAX;
    }
    pongPaddleLeftY = map(ballX, X_MAX, 256 - X_MAX, pongPaddleLeftStart, pongPaddleLeftTarget + offset);
  }
  else
  {
    int offset = 0;
    if (pongScoredHour)
    {
      if (pongPaddleRightTarget < 3 * Y_MAX)
        offset = 2 * Y_MAX;
      else
        offset = -2 * Y_MAX;
    }
    pongPaddleRightY = map(ballX, 256 - Y_MAX, Y_MAX, pongPaddleRightStart, pongPaddleRightTarget + offset);
  }

  // clear screen and draw time
  Screen.clear();
  drawDigits();

  // draw paddles
  Screen.setPixel(PongClockPlugin::X_MAX - 1, getScreenIndex(255, pongPaddleLeftY) / PongClockPlugin::Y_MAX - 1, PongClockPlugin::LED_TYPE_ON, 255);
  Screen.setPixel(PongClockPlugin::X_MAX - 1, getScreenIndex(255, pongPaddleLeftY) / PongClockPlugin::Y_MAX, PongClockPlugin::LED_TYPE_ON, 255);
  Screen.setPixel(PongClockPlugin::X_MAX - 1, getScreenIndex(255, pongPaddleLeftY) / PongClockPlugin::Y_MAX + 1, PongClockPlugin::LED_TYPE_ON, 255);

  Screen.setPixel(0, getScreenIndex(0, pongPaddleRightY) / PongClockPlugin::Y_MAX - 1, PongClockPlugin::LED_TYPE_ON, 255);
  Screen.setPixel(0, getScreenIndex(0, pongPaddleRightY) / PongClockPlugin::Y_MAX, PongClockPlugin::LED_TYPE_ON, 255);
  Screen.setPixel(0, getScreenIndex(0, pongPaddleRightY) / PongClockPlugin::Y_MAX + 1, PongClockPlugin::LED_TYPE_ON, 255);

  // draw ball
  Screen.setPixel(ballX / PongClockPlugin::X_MAX, ballY / PongClockPlugin::Y_MAX, PongClockPlugin::LED_TYPE_ON, ballBrightness);
}

const char *PongClockPlugin::getName() const
{
  return "PongClock";
}