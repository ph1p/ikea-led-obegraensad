#pragma once

#include <Arduino.h>
#include <vector>
#include "constants.h"

extern uint8_t letterU[ROWS * COLS];
extern uint8_t letterR[ROWS * COLS];

extern std::vector<std::vector<int>> circleAnimation;
extern std::vector<std::vector<int>> smallNumbers;
extern std::vector<std::vector<int>> bigNumbers;
extern std::vector<std::vector<int>> lineAnimation;