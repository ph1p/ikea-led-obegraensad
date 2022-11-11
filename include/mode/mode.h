#pragma once

#include <Arduino.h>
#include "constants.h"
#include "mode/breakout.h"
#include "mode/gameoflife.h"
#include "mode/stars.h"
#include "mode/lines.h"
#include "screen.h"
#include "circle.h"
#include "signs.h"

extern uint8_t modeBuffer[ROWS * COLS];

extern int buttonModeCount;
extern int modeButtonState;
extern int lastModeButtonState;

enum MODE
{
  NONE,
  STARS,
  LINES,
  BREAKOUT,
  GAMEOFLIFE,
  CIRCLE,
  // SYSTEM
  UPDATE,
  LOADING,
};

extern MODE currentMode;

void setMode(MODE mode);
void setModeByString(String mode);
void setModeByString(String mode, void (*callback)(MODE mode));
void modeLoop();
void listenOnButtonToChangeMode();
void loopOfAllModes();