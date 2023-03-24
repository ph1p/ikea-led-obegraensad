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
- Switch mode by pressing the button
- Modes
  - Game of life
  - Breakout
  - Snake
  - Stars
  - Lines
  - Circle
  - Clock
- Custom Animation with the "Creator"

# Control the board

https://user-images.githubusercontent.com/15351728/202763445-3275e4e9-d976-4e35-b0cf-9550b8561b4c.mp4

You can control the lamp with a supplied web GUI.
You can get the IP via serial output or you can search it in your router settings.

# How to

First of all. This software was written for the ESP32 Dev Board, but it should work with any other Arduino board as well. You just need to remove the WiFi, OTA and web server related code.

The ESP32 I used:

<img src="https://user-images.githubusercontent.com/15351728/200148521-86d0f9e6-2c41-4707-b2d9-8aa24a0e440e.jpg" width="60%" />

Verified to work with TTGO LoRa32 V2.1 (T3_V1.6.1).

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
  - Wifi
  - Upload
  - Your Pins
  - Latitude, Longitude, and OpenWeatherMap API key

Variables can be found inside `include/constants.h`.

### Create `include/secrets.h`

```cpp
#pragma once

#define WIFI_SSID ""
#define WIFI_PASSWORD ""
#define WIFI_HOSTNAME ""

#define OTA_USERNAME ""
#define OTA_PASSWORD ""

#define LATITUDE ""
#define LONGITUDE ""
#define WEATHERKEY ""
```

also set username and password inside `upload.py`, if you want to use OTA Updates.

### PINS

Connect them like this and remember to set them in `include/constants.h` according to your board.

| LCD              | ESP32  | TTGO LoRa32 |
| :----------------|:------:|:-----------:|
| GND              | GND    | GND         |
| VCC              | 5V     | 5V          |
| EN               | GPIO26 | IO22        |
| IN               | GPIO27 | IO23        |
| CLK              | GPIO14 | IO02        |
| CLA              | GPIO12 | IO15        |
| BUTTON one end   | GPIO16 | IO21        |
| BUTTON other end | GND    | GND         |

<img src="https://user-images.githubusercontent.com/86414213/205999001-6213fc4f-be2f-4305-a17a-44fdc9349069.jpg" width="60%" />

# Development

- `src` contains the arduino code.
  - Run it with platform io
  - You can uncomment the OTA lines in `platform.ini` if you want. Replace the IP with your device IP.

- `frontend` contains the web code.
  - First run `npm i`
  - Set your device IP inside the `.env` file
  - Start the server with `npm run dev`
  - Build it with `npm run build`. This command creates the `webgui.cpp` for you.

# Ideas

[] gifs
[] weather
[] animation upload
[] use `<canvas />`

## Credits

Breakout game https://elektro.turanis.de/html/prj104/index.html
