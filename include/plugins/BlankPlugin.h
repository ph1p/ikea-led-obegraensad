#pragma once

#include "PluginManager.h"

class BlankPlugin : public Plugin
{
private:
  
public:
  void update();
  void setup() override;
  void loop() override;
  const char *getName() const override;
};
