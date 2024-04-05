#include "plugins/DDPPlugin.h"

void DDPPlugin::setup()
{
    udp = new AsyncUDP();
    if (udp->listen(4048))
    {
        Serial.print("DDP server at IP: ");
        Serial.print(WiFi.localIP());
        Serial.println(" port: 4048");

        // Network runs on Core 0, renderBuffer writes are thus concurrent with onScreenTimer's SPI writing.
        udp->onPacket([](AsyncUDPPacket packet)
                      {
            // We could check the protocol but there is no real harm if not, display might look funny that's all.
/*
            // Check header flags
            uint8_t *hdr = packet.data();
            if (hdr[0] & 0xa0 != 0x40)
            {
                return; // expected protocol version 01
            }
            if (hdr[0] & 0x06 != 0)
            {
                return; // unexpected reply or query
            }

            // Check header data type
            int data_type = hdr[2];
            if (data_type != 0x00 && data_type != 0x08 && data_type != 0x20)
            {
                return; // unexpected data type
            }
*/
            int count = (packet.length() - 10) / 3; // assumes 8 bit per channel RGB format
            count = std::max(0, std::min(count, COLS * ROWS));
            uint8_t *data = packet.data() + 10;

            if (count == 1)
            {
                // virtual single pixel mode
                uint8_t brightness = (data[0] + data[1] + data[2]) / 3; // average RGB
                // uint8_t brightness = std::max(data[0], std::max(data[1], data[2])); // maximum brightness
                for (unsigned i = 0; i < COLS * ROWS; i++)
                {
                    Screen.setPixelAtIndex(i, brightness > 4, brightness);
                }
            }

            else if (count == ROWS)
            {
                // virtual row mode
                for (unsigned i = 0; i < count; i++)
                {
                    uint8_t brightness = (data[i * 3] + data[i * 3 + 1] + data[i * 3 + 2]) / 3; // average RGB
                    // uint8_t brightness = std::max(data[i * 3], std::max(data[i * 3 + 1], data[i * 3 + 2])); // maximum brightness
                    for (unsigned k = 0; k < COLS; k++)
                    {
                        Screen.setPixelAtIndex(i * COLS + k, brightness > 4, brightness);
                    }
                }
            }

            else
            {
                for (unsigned i = 0; i < count; i++)
                {
                    uint8_t brightness = (data[i * 3] + data[i * 3 + 1] + data[i * 3 + 2]) / 3; // average RGB
                    // uint8_t brightness = std::max(data[i * 3], std::max(data[i * 3 + 1], data[i * 3 + 2])); // maximum brightness
                    Screen.setPixelAtIndex(i, brightness > 4, brightness);
                }
            } });
    }
}

void DDPPlugin::teardown()
{
    delete udp;
}

void DDPPlugin::loop()
{
    delay(1000);
}

const char *DDPPlugin::getName() const
{
    return "DDP";
}
