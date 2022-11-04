#include "mode/mode.h"
#include "mode/breakout.h"
#include "mode/gameoflife.h"
#include "led.h"

MODE current_mode;
Thread modeThread = Thread();
uint8_t mode_buffer[ROWS * COLS];

void setMode(MODE mode)
{
  current_mode = mode;
  if (mode == NONE)
  {
    delay(20);
    renderScreen(render_buffer);
  }
  memset(mode_buffer, 0, sizeof(mode_buffer));
}

void drawLine(uint8_t line, bool isHorizontal)
{
  for (uint8_t i = 0; i < 16; i++)
  {
    mode_buffer[!isHorizontal ? line * 16 + i : i * 16 + line] = 1;
  }
};

void stars()
{
  while (current_mode == STARS)
  {
    for (int row = 0; row < ROWS; row++)
    {
      for (int col = 0; col < COLS; col++)
      {
        int ra = random(20);

        if (ra > 1)
        {
          ra = 0;
        }
        mode_buffer[row * 16 + col] = ra;
      }
    }
    renderScreen(mode_buffer);

    delay(400);
  }
}

void lines()
{
  int count = 0;
  bool forwards = true;
  while (current_mode == LINES)
  {
    drawLine(count, true);
    drawLine(count, false);
    renderScreen(mode_buffer);
    memset(mode_buffer, 0, sizeof(mode_buffer));

    if (count == 15 && forwards)
    {
      forwards = false;
    }
    if (count == 0 && !forwards)
    {
      forwards = true;
      count = 0;
    }

    if (forwards)
    {
      count++;
    }
    else
    {
      count--;
    }
    delay(60);
  }
}

void breakout()
{
  bool init = false;

  while (current_mode == BREAKOUT)
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

  while (current_mode == GAMEOFLIFE)
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
  if (current_mode != UPDATE)
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