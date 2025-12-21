#pragma once

#include "PluginManager.h"

class DrawPlugin : public Plugin
{
public:
  void setup() override;
  const char *getName() const override;
  void websocketHook(JsonDocument &request) override;
};
