#include "mode/mode.h"

MODE currentMode;

uint8_t modeBuffer[ROWS * COLS];

int buttonModeCount = -1;

int modeButtonState = 0;
int lastModeButtonState = 0;

GameOfLife gameOfLife;
Breakout breakout;
Circle circle;

void setMode(MODE mode)
{
  if (mode == currentMode || currentMode == LOADING)
    return;

  currentMode = LOADING;

  Screen.clear(modeBuffer);
  memset(modeBuffer, 0, sizeof(modeBuffer));
  delay(10);

  if (mode == STARS)
  {
    Screen.render(digitOne);
    buttonModeCount = 0;
  }
  else if (mode == LINES)
  {
    Screen.render(digitTwo);
    buttonModeCount = 1;
  }
  else if (mode == BREAKOUT)
  {
    breakout.setup();
    Screen.render(digitThree);
    buttonModeCount = 2;
  }
  else if (mode == GAMEOFLIFE)
  {
    gameOfLife.setup();
    Screen.render(digitFour);
    buttonModeCount = 3;
  }
  else if (mode == CIRCLE)
  {
    Screen.render(digitFive);
    buttonModeCount = 4;
  }

  if (mode == NONE)
  {
    Screen.render(digitZero);
    delay(1000);
    Screen.render(Screen.renderBuffer);
    buttonModeCount = 5;
  }
  else
  {
    delay(1000);
  }

  currentMode = mode;
}

void setModeByString(String mode, void (*callback)(MODE mode))
{
  MODE modeAsEnum = NONE;
  if (mode == "stars")
  {
    modeAsEnum = STARS;
  }
  else if (mode == "lines")
  {
    modeAsEnum = LINES;
  }
  else if (mode == "breakout")
  {
    modeAsEnum = BREAKOUT;
  }
  else if (mode == "gameoflife")
  {
    modeAsEnum = GAMEOFLIFE;
  }
  else if (mode == "circle")
  {
    modeAsEnum = CIRCLE;
  }
  if (callback)
  {
    callback(modeAsEnum);
  }
  setMode(modeAsEnum);
}

void listenOnButtonToChangeMode()
{
  modeButtonState = digitalRead(PIN_BUTTON);
  if (modeButtonState != lastModeButtonState && modeButtonState == HIGH)
  {
    if (buttonModeCount < 0)
    {
      buttonModeCount++;
    }
    else
    {
      if (buttonModeCount == 0)
      {
        setMode(STARS);
      }
      else if (buttonModeCount == 1)
      {
        setMode(LINES);
      }
      else if (buttonModeCount == 2)
      {
        setMode(BREAKOUT);
      }
      else if (buttonModeCount == 3)
      {
        setMode(GAMEOFLIFE);
      }
      else if (buttonModeCount == 4)
      {
        setMode(CIRCLE);
      }
      else if (buttonModeCount == 5)
      {
        setMode(NONE);
      }

      buttonModeCount++;

      if (buttonModeCount >= 6)
      {
        buttonModeCount = 0;
      }
    }
    delay(10);
  }
  lastModeButtonState = modeButtonState;
}

void loopOfAllModes()
{
  if (currentMode != UPDATE && currentMode != LOADING)
  {
    if (currentMode == STARS)
    {
      stars();
    }
    if (currentMode == LINES)
    {
      lines();
    }
    if (currentMode == BREAKOUT)
    {
      breakout.loop();
    }
    if (currentMode == GAMEOFLIFE)
    {
      gameOfLife.loop();
    }
    if (currentMode == CIRCLE)
    {
      circle.loop();
    }
  }
  delay(20);
}

void modeLoop()
{
  listenOnButtonToChangeMode();
  loopOfAllModes();
}