
#pragma once

#include "constants.h"

#ifdef ENABLE_SERVER
#include "screen.h"
#include "mode/mode.h"

extern struct tm timeinfo;

extern int previousMinutes;
extern int previousHour;

void clockSetup();
void clockLoop();

#endif
