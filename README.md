# IKEA OBEGRÄNSAD Hack/Mod

Turn your OBEGRÄNSAD LED Wall Lamp into a live drawing canvas

> **⚠ Disclaimer**: Use this code and instructions at your own risk! Improper use may damage the device.
> **Contribute**: Have suggestions or improvements? Feel free to submit a PR or open an issue. 😊

![ezgif-3-2019fca7a4](https://user-images.githubusercontent.com/15351728/200184222-a590575d-983d-4ab8-a322-c6bcf433d364.gif)

<details>
  <summary><h1>Features</h1></summary>

- Persist your drawing
- Rotate image
- Live Drawing
- OTA Update
- Wifi Control
- Web-GUI
- Load an image
- Switch plugin by pressing the button
- Schedule Plugins to switch after "n" seconds
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
  - Animation with the "Animation Creator in Web UI"
  - Firework
  - DDP
  - Pong Clock

</details>

# Control the board

https://github.com/user-attachments/assets/ddf91be1-2c95-4adc-b178-05b0781683cc

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

### ESP32 Setup with VS Code and PlatformIO

1. **Prerequisites**

   - Install **[Visual Studio Code](https://code.visualstudio.com/)**.
   - Install the **PlatformIO IDE** extension from the VS Code Extensions Marketplace.

2. **Clone the Project**

   - Download the project from GitHub and open it in VS Code. PlatformIO will automatically load dependencies.

```bash
git clone git@github.com:ph1p/ikea-led-obegraensad.git
cd ikea-led-obegraensad
code .
```

3. **Connect ESP32**

   - Connect your ESP32 via USB.
   - Check the COM port in the **PlatformIO Devices** tab.

4. **Prepare the Project**

   - Perform a `PlatformIO: Clean` (Recycle bin icon at the bottom right).
   - Add a `secrets.h` file to the `include` directory. Modify passwords and save the file. Go in the next section for WiFi instructions.

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

- Set variables inside `include/constants.h`.

5. **Build the Project**

   - Click the `PlatformIO Build` icon (bottom right corner).
   - Check the log for missing libraries.
     - Use the **Libraries** icon in PlatformIO to install required libraries.
   - Repeat `Clean` and `Build` until the build succeeds.

6. **Upload to ESP32**
   - Click `PlatformIO Upload` (bottom right) to upload the firmware to the ESP32.

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

### Alternate Button Wiring

Thanks to [RBEGamer](https://github.com/RBEGamer) who is showing in this [issue](https://github.com/ph1p/ikea-led-obegraensad/issues/79) how to use the original button wiring. With this solution you won't need the "BUTTON one end" and "BUTTON other end" soldering from the table above.

# HTTP API Endpoints

## Get Information

Get current values and the (fixed) metadata, like number of rows and columns and a list of available plugins.

```
GET http://your-server/api/info
```

## Set Active Plugin by ID

To set an active plugin by ID, make an HTTP PATCH request to the following endpoint:

```
PATCH http://your-server/api/plugin
```

### Parameters

- `id` (required): The ID of the plugin to set as active.

### Response

- Success: `200 OK` with the message "Plugin Set".
- Not Found: `404 Not Found` with the message "Plugin not found".

## Set Brightness

To set the brightness of the LED display, make an HTTP GET request to the following endpoint:

```
PATCH http://your-server/api/brightness
```

### Parameters

- `value` (required): The brightness value (0..255).

### Response

- Success: `200 OK` with the message "Ok".
- Invalid Value: `404 Not Found` with the message "Invalid Brightness Value".

## Get current display data

To get the current displayed data as an byte-array, each byte representing the brightness value.
Be aware that the global brightness value gets applied AFTER these values, so if you set the global brightness to 16, you will still get values of 255 this way.

```
GET http://your-server/api/data
```

# Plugin Scheduler

It is possible to switch between plugins automatically.
You can define your schedule in the Web UI or just send an API call.

```bash
### set your plugins and duration in seconds
curl -X POST http://x.x.x.x/api/schedule -d 'schedule=[{"pluginId":10,"duration":2},{"pluginId":8,"duration": 5}'

### clear the schedule
curl http://x.x.x.x/api/schedule/clear

### start the schedule
curl http://x.x.x.x/api/schedule/start

### stop the schedule
curl http://x.x.x.x/api/schedule/stop
```

# DDP (Distributed Display Protocol)

You can set the panel to DDP using the button or via the web interface.
This Protocol uses **UDP** and listens on Port **4048**.

**Info**: The DDP Header is 10 Bytes!

In the repository you will find an `ddp.py` as an example.

Change the plugin to `DDP` and run following command with your IP:

```bash
./ddp.py --ip x.x.x.x --pixel 2 2 200 --pixel 0 0 255
```

### Helpful Links

- https://kno.wled.ge/interfaces/ddp/
- http://www.3waylabs.com/ddp/

---

# Messages HTTP API

The LED Display service provides HTTP API to display messages and graphs on a 16x16 LED display. This functionality can be accessed through HTTP calls to the service endpoint.

## Message Display

To display a message on the LED display, users can make an HTTP GET request to the following endpoint:

```
http://your-server/api/message
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
GET http://your-server/api/message?text=Hello&graph=8,5,2,1,0,0,1,4,7,10,13,14,15,15,14,11&repeat=3&id=1&delay=60
```

This example will display the message "Hello" on the LED display with a corresponding graph, repeat it three times, and assign it the identifier 1, waits 60ms while scrolling.

### Message Removal

To remove a message from the display, users can make an HTTP GET request to the following endpoint:

```
http://your-server/api/removemessage
```

### Parameters

- `id` (required): The unique identifier of the message to be removed.

#### Example

```
GET http://your-server/api/removemessage?id=1
```

This example will remove the message with the identifier 1 from the LED display.

# Development

- `src` contains the arduino code.

  - Run it with platform io
  - You can uncomment the OTA lines in `platformio.ini` if you want. Replace the IP with your device IP.

- `frontend` contains the web code.

  - First run `pnpm install`
  - Set your device IP inside the `.env` file
  - Start the server with `pnpm dev`
  - Build it with `pnpm build`. This command creates the `webgui.cpp` for you.

- Build frontend using `Docker`
  - From the root of the repo, run `docker compose run node`

# Plugin Development

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

# Troubleshooting

## Flickering panel

- Check all soldering points, especially VCC
- Check if the board gets enough power
