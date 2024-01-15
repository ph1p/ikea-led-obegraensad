# IKEA OBEGRÄNSAD Hack/Mod

Turn your OBEGRÄNSAD LED Wall Lamp into a live drawing canvas

> 👉 This software is in an early stage and is my first of its kind. If you have anything to improve, I would be very happy about a PR or an issue :)

> ⚠ Use this code and instructions at your own risk! The device could be damaged! ⚠

![ezgif-3-2019fca7a4](https://user-images.githubusercontent.com/15351728/200184222-a590575d-983d-4ab8-a322-c6bcf433d364.gif)

## Features

- Persist your drawing
- Rotate image
- Live Drawing
- OTA Update
- Wifi Control
- Web-GUI
- Load an image
- Switch plugin by pressing the button
- Plugins
  - Draw
  - Game of life
  - Breakout
  - Snake
  - Stars
  - Lines
  - Circle
  - Clock
  - Big Clock
  - Weather
  - Rain
  - Animation with the "Creator"
  - Firework

# Control the board

https://user-images.githubusercontent.com/15351728/202763445-3275e4e9-d976-4e35-b0cf-9550b8561b4c.mp4

You can control the lamp with a supplied web GUI.
You can get the IP via serial output or you can search it in your router settings.

# How to

First of all. This software was written for the ESP32 Dev Board, but it should work with any other Arduino board as well. You just need to remove the WiFi, OTA and web server related code.

The ESP32 I used:

<img src="https://user-images.githubusercontent.com/15351728/200148521-86d0f9e6-2c41-4707-b2d9-8aa24a0e440e.jpg" width="60%" />

Verified to work with TTGO LoRa32 V2.1 (T3_V1.6.1).
Note: On esp8266 per pixel brightness only works when storage and global brightness (analogWrite) are disabled.

## Open the lamp

I'm sorry to say this, but you'll have to pry open the back of your Lamp, as IKEA didn't install regular screws here. I lifted the back with a screwdriver between the screws and pried it open with a second object, but you can also drill out the rivets to avoid breaking the backpanel.

## The panels

<img src="https://user-images.githubusercontent.com/15351728/200183585-39c1668d-665b-4c12-bcbb-387aec1d3874.JPG" width="60%" />

After you open the back, you will see 4 identical plates. These are each equipped with 64 Leds in 4 fields. We are only interested in the lowest one. Here you will find 6 connectors at the bottom edge, to which we connect our board.
Above is a microcontroller. You have to remove it, because it contains the standard programs.

<img src="https://user-images.githubusercontent.com/86414213/205998862-e9962695-1328-49ea-b546-be592cbad3c2.jpg" width="90%" />

## Clone repository and set variables

- Open folder with VSCode
- Install platformIO (https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide)
- Set all variables
  - Wifi (on ESP8266)
  - Upload
  - Your Pins
  - Latitude, Longitude, City etc. (https://github.com/chubin/wttr.in)

Variables can be found inside `include/constants.h`.

### Create `include/secrets.h`

```cpp
#pragma once

#define WIFI_HOSTNAME ""

#ifdef ESP8266
#define WIFI_SSID ""
#define WIFI_PASSWORD ""
#endif

#define OTA_USERNAME ""
#define OTA_PASSWORD ""
```

also set username and password inside `upload.py`, if you want to use OTA Updates.

### Configuring WiFi with WiFi manager

_Note:_ The WiFi manager only works on ESP32. For ESP8266, `WIFI_SSID` and `WIFI_PASSWORD` need to be provided in `secrets.h`.

This project uses [tzapu's WiFiManager](https://github.com/tzapu/WiFiManager). After booting up, the device will try
to connect to known access points. If no known access point is available, the device will create a network called
`Ikea Display Setup WiFi`. Connect to this network on any device. A captive portal will pop up and will take you
through the configuration process. After a successful connection, the device will reboot and is ready to go.

The name of the created network can be changed by modifying `WIFI_MANAGER_SSID` in `include/constants.h`.

### PINS

Connect them like this and remember to set them in `include/constants.h` according to your board.

|       LCD        | ESP32  | TTGO LoRa32 | NodeMCUv2 | Lolin D32 (Pro) |
| :--------------: | :----: | :---------: | :-------: | :-------------: |
|       GND        |  GND   |     GND     |    GND    |       GND       |
|       VCC        |   5V   |     5V      |    VIN    |       USB       |
| EN (PIN_ENABLE)  | GPIO26 |    IO22     | GPIO16 D0 |     GPIO26      |
|  IN (PIN_DATA)   | GPIO27 |    IO23     | GPIO13 D7 |     GPIO27      |
| CLK (PIN_CLOCK)  | GPIO14 |    IO02     | GPIO14 D5 |     GPIO14      |
| CLA (PIN_LATCH)  | GPIO12 |    IO15     | GPIO0 D3  |     GPIO12      |
|  BUTTON one end  | GPIO16 |    IO21     | GPIO2 D4  |     GPIO25      |
| BUTTON other end |  GND   |     GND     |    GND    |       GND       |

<img src="https://user-images.githubusercontent.com/86414213/205999001-6213fc4f-be2f-4305-a17a-44fdc9349069.jpg" width="60%" />

# Development

- `src` contains the arduino code.

  - Run it with platform io
  - You can uncomment the OTA lines in `platformio.ini` if you want. Replace the IP with your device IP.

- `frontend` contains the web code.

  - First run `npm i`
  - Set your device IP inside the `.env` file
  - Start the server with `npm run dev`
  - Build it with `npm run build`. This command creates the `webgui.cpp` for you.

- Build frontend using `Docker`
  - From the root of the repo, run `docker compose run node`

## Plugins

1. Start by creating a new C++ file for your plugin. For example, let's call it plugins/MyPlugin.(cpp/h).

**plugins/MyPlugin.h**

```cpp
#pragma once

#include "PluginManager.h"

class MyPlugin : public Plugin {
public:
    MyPlugin();
    ~MyPlugin() override;

    void setup() override;
    void loop() override;
    const char* getName() const override;

    void teardown() override; // optional
    void websocketHook(DynamicJsonDocument &request) override; // optional
};
```

**plugins/MyPlugin.cpp**

```cpp
#include "plugins/MyPlugin.h"

MyPlugin::MyPlugin() {
    // Constructor logic, if needed
}

void MyPlugin::setup() {
    // Setup logic for your plugin
}

void MyPlugin::loop() {
    // Loop logic for your plugin
}

const char* MyPlugin::getName() const {
    return "MyPlugin"; // name in GUI
}

void MyPlugin::teardown() {
  // code if plugin gets deactivated
}

void MyPlugin::websocketHook(DynamicJsonDocument &request) {
  // handle websocket requests
}
```

2. Add your plugin to the `main.cpp`.

```cpp
#include "plugins/MyPlugin.h"

pluginManager.addPlugin(new MyPlugin());
```

# External Call

The LED Display service provides a simple yet powerful external interface that allows users to display messages and graphs on a 16x16 LED display. This functionality can be accessed through HTTP calls to the service endpoint.

## Message Display

To display a message on the LED display, users can make an HTTP GET request to the following endpoint:

```
http://your-server/message
```

### Parameters

- `text` (optional): The text message to be displayed on the LED display.
- `graph` (optional): A comma-separated list of integers representing a graph. The values should be in the range of 0 to 15 and will be visualized as a graph on the LED display.
- `miny` (optional): scaling for lower end of the graph, defaults to 0
- `maxy` (optional): scaling for upper end of the graph, defaults to 15
- `repeat` (optional): The number of times the message should be repeated. If not provided, the default is 1. Set this value to -1 to repeat infinitely. While messages ar pending for display an indicator led in the upper left corner will flash.
- `id` (optional): A unique identifier for the message. This can be used for later removal or modification of the message.
- `delay` (optional): The number of ms of delay between every scroll move. Default is 50 ms.

#### Example

```
GET http://your-server/message?text=Hello&graph=8,5,2,1,0,0,1,4,7,10,13,14,15,15,14,11&repeat=3&id=1;delay=60
```

This example will display the message "Hello" on the LED display with a corresponding graph, repeat it three times, and assign it the identifier 1, waits 60ms while scrolling.

## Message Removal

To remove a message from the display, users can make an HTTP GET request to the following endpoint:

```
http://your-server/removemessage
```

### Parameters

- `id` (required): The unique identifier of the message to be removed.

#### Example

```
GET http://your-server/removemessage?id=1
```

This example will remove the message with the identifier 1 from the LED display.

# Ideas

- [ ] gifs
- [ ] animation upload
- [ ] use `<canvas />`

## Credits

Breakout game https://elektro.turanis.de/html/prj104/index.html
