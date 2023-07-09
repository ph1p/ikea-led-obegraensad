#pragma once

#include "PluginManager.h"

class StarsPlugin : public Plugin
{
public:
  void setup() override;
  void loop() override;
  const char *getName() const override;
};
