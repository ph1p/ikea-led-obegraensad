#include "mode/mode.h"

MODE currentMode;

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

  if (currentMode == NONE)
  {
    // save current drawing in cache
    Screen.cacheCurrent();
  }

  Screen.clear();
  currentMode = LOADING;

  // loading screen
  Screen.setPixel(4, 7, 1);
  Screen.setPixel(5, 7, 1);
  Screen.setPixel(7, 7, 1);
  Screen.setPixel(8, 7, 1);
  Screen.setPixel(10, 7, 1);
  Screen.setPixel(11, 7, 1);
  Screen.render();

  if (mode == NONE)
  {
    buttonModeCount = 0;
    Screen.restoreCache();
    Screen.render();
  }
  else if (mode == STARS)
  {
    buttonModeCount = 1;
  }
  else if (mode == LINES)
  {
    buttonModeCount = 2;
  }
  else if (mode == BREAKOUT)
  {
    breakout.setup();
    buttonModeCount = 3;
  }
  else if (mode == GAMEOFLIFE)
  {
    gameOfLife.setup();
    buttonModeCount = 4;
  }
  else if (mode == CIRCLE)
  {
    circle.setup();
    buttonModeCount = 5;
  }
  else if (mode == CLOCK)
  {
#ifdef ENABLE_SERVER
    clockSetup();
#endif
    buttonModeCount = 6;
  }

  delay(1500);
  currentMode = mode;
}

MODE getModeByString(String mode)
{
  if (mode == "stars")
  {
    return STARS;
  }
  else if (mode == "lines")
  {
    return LINES;
  }
  else if (mode == "breakout")
  {
    return BREAKOUT;
  }
  else if (mode == "gameoflife")
  {
    return GAMEOFLIFE;
  }
  else if (mode == "circle")
  {
    return CIRCLE;
  }
  else if (mode == "clock")
  {
    return CLOCK;
  }
  return NONE;
}

void setModeByString(String mode, void (*callback)(MODE mode))
{
  MODE modeAsEnum = getModeByString(mode);
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
      buttonModeCount = 1;
    }
    else
    {
      if (buttonModeCount == 0)
      {
        setMode(NONE);
      }
      else if (buttonModeCount == 1)
      {
        setMode(STARS);
      }
      else if (buttonModeCount == 2)
      {
        setMode(LINES);
      }
      else if (buttonModeCount == 3)
      {
        setMode(BREAKOUT);
      }
      else if (buttonModeCount == 4)
      {
        setMode(GAMEOFLIFE);
      }
      else if (buttonModeCount == 5)
      {
        setMode(CIRCLE);
      }
      else if (buttonModeCount == 6)
      {
        setMode(CLOCK);
      }

      buttonModeCount++;

      if (buttonModeCount > 6)
      {
        buttonModeCount = 0;
      }
    }
  }
  lastModeButtonState = modeButtonState;
  delay(10);
}

void loopOfAllModes()
{
  if (currentMode != UPDATE && currentMode != LOADING)
  {
    if (currentMode == NONE)
    {
      listenOnButtonToChangeMode();
    }
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
    while (currentMode == CLOCK)
    {
#ifdef ENABLE_SERVER
      clockLoop();
#endif
    }
  }
}

void persistMode()
{
  storage.begin("led-wall", false);
  storage.putInt("mode", currentMode);
  storage.end();
}

void loadMode()
{
  storage.begin("led-wall", false);
  setMode((MODE)storage.getInt("mode"));
  storage.end();
}