#include "mode/mode.h"
#include "mode/breakout.h"
#include "mode/gameoflife.h"
#include "mode/stars.h"
#include "mode/lines.h"
#include "led.h"

MODE currentMode;
Thread modeThread = Thread();
uint8_t mode_buffer[ROWS * COLS];

void setMode(MODE mode)
{
  currentMode = mode;
  if (mode == NONE)
  {
    delay(20);
    renderScreen(render_buffer);
  }
  memset(mode_buffer, 0, sizeof(mode_buffer));
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

void breakout()
{
  bool init = false;

  while (currentMode == BREAKOUT)
  {
    if (!init)
    {
      init = true;
      breakoutSetup();
    }
    breakoutLoop();
  }
}

void gameoflife()
{
  bool init = false;

  while (currentMode == GAMEOFLIFE)
  {
    if (!init)
    {
      init = true;
      gameOfLifeSetup();
    }
    gameOfLifeLoop();
  }
}

void modes()
{
  if (currentMode != UPDATE)
  {
    stars();
    lines();
    breakout();
    gameoflife();
  }
}

void initModes()
{
  modeThread.onRun(modes);
  modeThread.setInterval(100);
}