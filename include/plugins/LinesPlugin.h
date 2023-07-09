#pragma once

#include "PluginManager.h"

class LinesPlugin : public Plugin
{
private:
  uint8_t count = 0;
  std::vector<std::vector<int>> frames = {
      {0xcc, 0xcc},
      {0x66, 0x66},
      {0x33, 0x33},
      {0x99, 0x99}};

public:
  void setup() override;
  void loop() override;
  const char *getName() const override;
};
