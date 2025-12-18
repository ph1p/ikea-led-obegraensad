#pragma once

#include "constants.h"
#include <Arduino.h>
#include <string>
#include <vector>

extern std::vector<int> letterU;
extern std::vector<int> letterR;
extern std::vector<int> degreeSymbol;
extern std::vector<int> minusSymbol;

extern std::vector<std::vector<int>> circleAnimation;
extern std::vector<std::vector<int>> smallNumbers;
extern std::vector<std::vector<int>> bigNumbers;
extern std::vector<std::vector<int>> lineAnimation;
extern std::vector<std::vector<int>> weatherIcons;

struct font
{
  std::string name;
  int sizeX;
  int sizeY;
  int offset;
  std::vector<std::vector<int>> data;
};

extern std::vector<font> fonts;
