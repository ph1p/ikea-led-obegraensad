#include "plugins/ArtNet.h"

void ArtNetPlugin::setup() {
    artnet.begin();
    artnet.setArtDmxCallback(onDmxFrame);
    Serial.print("ArtNet server listening at IP: ");
    Serial.print(WiFi.localIP());
    Serial.print(" port: ");
    Serial.println(ART_NET_PORT);
    artnet.setUniverse(1);
    Serial.print("Universe: ");
    Serial.println(artnet.getOutgoing());


}

void ArtNetPlugin::teardown() {
}

void ArtNetPlugin::loop() {
    artnet.read();
}

const char *ArtNetPlugin::getName() const
{
    return "ArtNet";
}

void ArtNetPlugin::onDmxFrame(uint16_t universe, uint16_t length, uint16_t outgoing, uint8_t* data) {
    Serial.print("Universe: ");
    Serial.println(universe);
    if (universe == outgoing) {
        for (int i=0; i < ROWS * COLS; i++) {
                Screen.setPixelAtIndex(i, data[i] > 4, data[i]);
        }
    }
}

