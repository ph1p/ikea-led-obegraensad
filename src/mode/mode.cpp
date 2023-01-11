#include "mode/mode.h"

MODE currentMode;

int buttonModeCount = -1;

int modeButtonState = 0;
int lastModeButtonState = 0;

GameOfLife gameOfLife;
Breakout breakout;
Circle circle;
Lines lines;
BigClock bigClock;
Custom custom;

void setMode(MODE mode, bool selfLoading)
{
  if (mode == currentMode || (currentMode == LOADING && !selfLoading))
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
    buttonModeCount = NONE;
    Screen.restoreCache();
  }
  else if (mode == STARS)
  {
    buttonModeCount = STARS;
  }
  else if (mode == LINES)
  {
    lines.setup();
    buttonModeCount = STARS;
  }
  else if (mode == BREAKOUT)
  {
    breakout.setup();
    buttonModeCount = BREAKOUT;
  }
  else if (mode == GAMEOFLIFE)
  {
    gameOfLife.setup();
    buttonModeCount = GAMEOFLIFE;
  }
  else if (mode == CIRCLE)
  {
    circle.setup();
    buttonModeCount = CIRCLE;
  }
  else if (mode == CLOCK)
  {
#ifdef ENABLE_SERVER
    clockSetup();
#endif
    buttonModeCount = CLOCK;
  }
  else if (mode == BIGCLOCK)
  {
#ifdef ENABLE_SERVER
    bigClock.setup();
#endif
    buttonModeCount = BIGCLOCK;
  }
  else if (mode == CUSTOM)
  {
    custom.setup();
    buttonModeCount = CUSTOM;
  }

  delay(800);
  currentMode = mode;

  if (currentMode == NONE)
  {
    Screen.render();
  }
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
  else if (mode == "bigclock")
  {
    return BIGCLOCK;
  }
  else if (mode == "custom")
  {
    return CUSTOM;
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
      buttonModeCount = NONE;
    }
    else
    {
      if (buttonModeCount == NONE)
      {
        setMode(NONE);
      }
      else if (buttonModeCount == STARS)
      {
        setMode(STARS);
      }
      else if (buttonModeCount == LINES)
      {
        setMode(LINES);
      }
      else if (buttonModeCount == BREAKOUT)
      {
        setMode(BREAKOUT);
      }
      else if (buttonModeCount == GAMEOFLIFE)
      {
        setMode(GAMEOFLIFE);
      }
      else if (buttonModeCount == CIRCLE)
      {
        setMode(CIRCLE);
      }
      else if (buttonModeCount == CLOCK)
      {
        setMode(CLOCK);
      }
      else if (buttonModeCount == BIGCLOCK)
      {
        setMode(BIGCLOCK);
      }
      else if (buttonModeCount == CUSTOM)
      {
        setMode(CUSTOM);
      }

      buttonModeCount++;

      if (buttonModeCount > NUM_MODES)
      {
        buttonModeCount = NONE;
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
      lines.loop();
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
    if (currentMode == CUSTOM)
    {
      custom.loop();
    }
    if (currentMode == CLOCK)
    {
#ifdef ENABLE_SERVER
      clockLoop();
#endif
    }
    if (currentMode == BIGCLOCK)
    {
#ifdef ENABLE_SERVER
      bigClock.loop();
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