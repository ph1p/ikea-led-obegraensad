#pragma once

#include "PluginManager.h"
#include "AsyncUDP.h"

class DDPPlugin : public Plugin
{
private:
  AsyncUDP *udp;

public:
  void setup() override;
  void teardown() override;
  void loop() override;
  const char *getName() const override;
};
