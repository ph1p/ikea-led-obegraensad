#pragma once

#include "PluginManager.h"

#include "ArtnetWifi.h"

#define START_UNIVERSE 1

class ArtNetPlugin : public Plugin {
private:
    ArtnetWifi artnet;
    const int startUniverse = 1;

public:
    void setup() override;
    void teardown() override;
    void loop() override;
    const char* getName() const override;
    static void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data);

};

