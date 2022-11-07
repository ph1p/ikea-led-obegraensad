#ifndef GOL_HEADER_H
#define GOL_HEADER_H

#include <Arduino.h>
#include "mode/mode.h"
#include "led.h"
#include "constants.h"

uint8_t updateCell(int row, int col);
uint8_t countNeighbours(int row, int col);
void next();
void gameOfLifeSetup();
void gameOfLifeLoop();

#endif