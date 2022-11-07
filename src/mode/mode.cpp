#include "mode/mode.h"

MODE currentMode;

uint8_t mode_buffer[ROWS * COLS];

int buttonModeCount = -1;

int modeButtonState = 0;
int lastModeButtonState = 0;

void setMode(MODE mode)
{
  if (mode == currentMode || currentMode == LOADING)
    return;

  currentMode = LOADING;

  clearScreenAndBuffer(mode_buffer);
  memset(mode_buffer, 0, sizeof(mode_buffer));
  delay(10);

  if (mode == STARS)
  {
    renderScreen(digitOne);
    buttonModeCount = 0;
  }
  else if (mode == LINES)
  {
    renderScreen(digitTwo);
    buttonModeCount = 1;
  }
  else if (mode == BREAKOUT)
  {
    breakoutSetup();
    renderScreen(digitThree);
    buttonModeCount = 2;
  }
  else if (mode == GAMEOFLIFE)
  {
    gameOfLifeSetup();
    renderScreen(digitFour);
    buttonModeCount = 3;
  }

  if (mode == NONE)
  {
    renderScreen(digitZero);
    delay(1000);
    renderScreen(render_buffer);
    buttonModeCount = 4;
  }
  else
  {
    delay(1000);
  }

  currentMode = mode;
}

void setModeByString(String mode)
{
  if (mode == "stars")
  {
    setMode(STARS);
  }
  else if (mode == "lines")
  {
    setMode(LINES);
  }
  else if (mode == "breakout")
  {
    setMode(BREAKOUT);
  }
  else if (mode == "gameoflife")
  {
    setMode(GAMEOFLIFE);
  }
  else
  {
    setMode(NONE);
  }
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
        setMode(NONE);
      }

      buttonModeCount++;

      if (buttonModeCount >= 5)
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
      breakoutLoop();
    }
    if (currentMode == GAMEOFLIFE)
    {
      gameOfLifeLoop();
    }
  }
  delay(20);
}

void modeLoop()
{
  listenOnButtonToChangeMode();
  loopOfAllModes();
}