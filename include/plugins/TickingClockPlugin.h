#pragma once

#include "PluginManager.h"

class TickingClockPlugin : public Plugin
{
private:
    struct tm timeinfo;

    int previousMinutes;
    int previousHour;
    int previousSecond;

public:
    void setup() override;
    void loop() override;
    const char *getName() const override;
};
