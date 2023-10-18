#pragma once

#include <Arduino.h>
#include "constants.h"
#include "mode/breakout.h"
#include "mode/snake.h"
#include "mode/gameoflife.h"
#include "mode/stars.h"
#include "mode/lines.h"
#include "mode/circle.h"
#include "mode/clock.h"
#include "mode/weather.h"
#include "mode/rain.h"
#include "screen.h"
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
  WEATHER,
  RAIN,
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
