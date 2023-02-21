#pragma once

#include <Arduino.h>
#include "constants.h"
#include "mode/breakout.h"
#include "mode/snake.h"
#include "mode/gameoflife.h"
#include "mode/stars.h"
#include "mode/lines.h"
#include "screen.h"
#include "circle.h"
#include "clock.h"
#include "signs.h"
#include "bigclock.h"
#include "custom.h"

extern int buttonModeCount;
extern int modeButtonState;
extern int lastModeButtonState;

enum MODE
{
  NONE,
  STARS,
  LINES,
  BREAKOUT,
  SNAKE,
  GAMEOFLIFE,
  CIRCLE,
  CLOCK,
  BIGCLOCK,
  CUSTOM,
  // SYSTEM
  UPDATE,
  LOADING,
};

extern MODE currentMode;

void setMode(MODE mode, bool selfLoading = false);
void setModeByString(String mode);
MODE getModeByString(String mode);
void setModeByString(String mode, void (*callback)(MODE mode));
void modeLoop();
void listenOnButtonToChangeMode();
void loopOfAllModes();
void persistMode();
void loadMode();