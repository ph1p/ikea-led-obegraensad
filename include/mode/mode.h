#ifndef MODE_HEADER_H
#define MODE_HEADER_H

#include <Arduino.h>
#include "constants.h"
#include "mode/breakout.h"
#include "mode/gameoflife.h"
#include "mode/stars.h"
#include "mode/lines.h"
#include "led.h"
#include "signs.h"

extern uint8_t mode_buffer[ROWS * COLS];

enum MODE
{
  NONE,
  STARS,
  LINES,
  BREAKOUT,
  GAMEOFLIFE,
  // SYSTEM
  UPDATE,
  LOADING,
};

extern MODE currentMode;

void setMode(MODE mode);
void setModeByString(String mode);
void modeLoop();

#endif