#ifndef MODE_HEADER_H
#define MODE_HEADER_H

#include <Thread.h>
#include "constants.h"

extern uint8_t mode_buffer[ROWS * COLS];

enum MODE
{
  NONE,
  STARS,
  UPDATE,
  LINES,
  BREAKOUT,
  GAMEOFLIFE,
};

extern MODE currentMode;
extern Thread modeThread;

void setMode(MODE mode);
void setModeByString(String mode);
void initModes();

#endif