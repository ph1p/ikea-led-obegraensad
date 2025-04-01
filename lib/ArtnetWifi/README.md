# ArtnetWifi

[![arduino-library-badge](https://www.ardu-badge.com/badge/ArtnetWifi.svg?)](https://www.ardu-badge.com/ArtnetWifi)

An Art-Net library for Wifi-Arduino's. Tested on ESP8266, ESP32, Pi Pico W, WiFi101 (e.g. MKR1000) and WiFiNINA (e.g. NANO 33 IoT) devices.

Note: this library assumes you are using a wifi module.

Based on https://github.com/natcl/Artnet [master](https://github.com/natcl/Artnet/archive/master.zip)

## Installation

### Arduino IDE

Navigate to **Sketch** -> **Include Library** -> **Manage Libraries...**,
then search for `ArtnetWifi` and the library will show up. Click **Install** and the library is ready to use.

### PlatformIO Core (CLI)

```
$ pio init --board nodemcuv2
$ pio lib install artnetwifi
```

### Manual

Place this in your `~/Documents/Arduino/libraries` folder.

## Examples

Different examples are provided, here is a summary of what each example does.

### ArtnetWifiDebug

Simple test for WiFi, serial and Art-Net.

Example output (Serial Monitor, 115200 Baud):
```
DMX: Univ: 0, Seq: 0, Data (48): 17 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ...
```

If this example is not working, don't try anything else!

### ArtnetWifiDebug2 (ArtnetWifiDebug with C++11 style)

See **ArtnetWifiDebug**.

**Note:** Not all controllers support this type of code!

### ArtnetWifiFastLED

This example will receive multiple universes via Art-Net and control a strip of WS2812 LEDs via the [FastLED library](https://github.com/FastLED/FastLED). It is similar to the NeoPixel example but it will work on the ESP32 and the ESP8266 controller as well.

### ArtnetWifiNeoPixel

This example will receive multiple universes via Art-Net and control a strip of WS2811 LEDs via Adafruit's [NeoPixel library](https://github.com/adafruit/Adafruit_NeoPixel).

### ArtnetWifiTransmit

This is a simple transmitter. Send 3 byte over into the Art-Net, to make a RGB light ramp-up in white.


### Notes

The examples `FastLED` and `NeoPixel` can utilize many universes to light up hundreds of LEDs.
Normaly Art-Net frame can handle 512 bytes. Divide this by 3 colors, so a single universe can
have 170.66 LEDs. For easy access, only 170 LEDs are used. The last 2 byte per universe are "lost".

**Example:** 240 LEDs, 720 Byte, 2 Universes

**Universe "1"**

|Byte |  1|  2|  3|...|508|509|510|511|512|
|:----|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|Color|  R|  G|  B|...|  R|  G|  B| x | x |
|LED  |  1|  1|  1|...|170|170|170|   |   |

**Universe "2"**

|Byte |  1|  2|  3|...|208|209|210|
|:----|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|Color|  R|  G|  B|...|  R|  G|  B|
|LED  |171|171|171|...|240|240|240|

*You only have to send 510 byte DMX-data per frame. Extra byte(s) at the end will be ignored!*

# Art-Net

Art-Net(tm) is a trademark of Artistic Licence Holdings Ltd. The Art-Net protocol and associated documentation is copyright Artistic Licence Holdings Ltd.

[Art-Net](http://www.artisticlicence.com/WebSiteMaster/User%20Guides/art-net.pdf)
