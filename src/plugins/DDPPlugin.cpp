#include "plugins/DDPPlugin.h"

#ifdef ASYNC_UDP_ENABLED
// packets arrive on the network task, so they are collected here and copied
// into the screen from loop() instead of drawing into a frame mid-present
static uint8_t ddpFrame[ROWS * COLS];
static volatile bool ddpFrameReady = false;
#endif

void DDPPlugin::setup()
{
#ifdef ASYNC_UDP_ENABLED
  udp = new AsyncUDP();
  if (udp->listen(4048))
  {
    Serial.print("DDP server listening at port: 4048");

    udp->onPacket([](AsyncUDPPacket packet) {
      if (packet.length() >= 10)
      {                                           // Basic DDP header check
        const uint8_t *data = packet.data() + 10; // Skip header
        const size_t dataLength = packet.length() - 10;
        int count = std::min((int)(dataLength / 3), ROWS * COLS); // Each pixel is RGB

        if (count == 1)
        { // Single pixel mode
          uint8_t brightness = (data[0] + data[1] + data[2]) / 3;
          for (int i = 0; i < ROWS * COLS; i++)
          {
            ddpFrame[i] = brightness > 4 ? brightness : 0;
          }
        }
        else
        { // Full pixel mapping
          for (int i = 0; i < count; i++)
          {
            uint8_t brightness = (data[i * 3] + data[i * 3 + 1] + data[i * 3 + 2]) / 3;
            ddpFrame[i] = brightness > 4 ? brightness : 0;
          }
        }
        ddpFrameReady = true;
      }
    });
  }
#endif
}

void DDPPlugin::teardown()
{
#ifdef ASYNC_UDP_ENABLED
  if (udp)
  {
    delete udp;
    udp = nullptr;
  }
#endif
}

void DDPPlugin::loop()
{
#ifdef ASYNC_UDP_ENABLED
  if (ddpFrameReady)
  {
    ddpFrameReady = false;
    Screen.setRenderBuffer(ddpFrame, true);
  }
#endif
#ifdef ESP32
  vTaskDelay(1);
#else
  delay(1);
#endif
}

const char *DDPPlugin::getName() const
{
  return "DDP";
}