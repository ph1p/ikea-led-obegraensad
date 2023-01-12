#pragma once

#include <Arduino.h>
#include "constants.h"
#include "mode/breakout.h"
#include "mode/gameoflife.h"
#include "mode/stars.h"
#include "mode/lines.h"
#include "screen.h"
#include "circle.h"
#include "clock.h"
#include "signs.h"
#include "custom.h"
#include "bigclock.h"

extern int buttonModeCount;
extern int modeButtonState;
extern int lastModeButtonState;

enum MODE
{
  // DISPLAY MODES
  NONE = 0,
  CUSTOM = 1,
  STARS = 2,
  LINES = 3,
  BREAKOUT = 4,
  GAMEOFLIFE = 5,
  CIRCLE = 6,
  CLOCK = 7,
  BIGCLOCK = 8,
  NUM_MODES = 9,
  // SYSTEM MODES
  UPDATE = 98,
  LOADING = 99,
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