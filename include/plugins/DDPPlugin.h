#pragma once

#include "PluginManager.h"
#if __has_include("AsyncUDP.h")
#include "AsyncUDP.h"
#define ASYNC_UDP_ENABLED
#endif

class DDPPlugin : public Plugin
{
private:
#ifdef ASYNC_UDP_ENABLED
  AsyncUDP *udp;
#endif

public:
  void setup() override;
  void teardown() override;
  void loop() override;
  const char *getName() const override;
};
