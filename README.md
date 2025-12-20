# IKEA OBEGRÄNSAD Hack/Mod

Turn your OBEGRÄNSAD LED Wall Lamp into a live drawing canvas

> **⚠ Disclaimer**: Use this code and instructions at your own risk! Improper use may damage the device.
> **Contribute**: Have suggestions or improvements? Feel free to submit a PR or open an issue. 😊

![ezgif-3-2019fca7a4](https://user-images.githubusercontent.com/15351728/200184222-a590575d-983d-4ab8-a322-c6bcf433d364.gif)

## Table of Contents

- [Features](#features)
- [Quick Start](#quick-start)
- [Hardware Setup](#hardware-setup)
  - [Opening the Lamp](#opening-the-lamp)
  - [Understanding the Panels](#understanding-the-panels)
  - [Pin Configuration](#pin-configuration)
  - [Alternate Button Wiring](#alternate-button-wiring)
- [Software Setup](#software-setup)
  - [ESP32 Setup with VS Code and PlatformIO](#esp32-setup-with-vs-code-and-platformio)
  - [WiFi Configuration](#wifi-configuration)
- [OTA Updates](#ota-updates)
  - [Configuration](#configuration)
  - [Upload Methods](#upload-methods)
  - [Visual Feedback](#visual-feedback)
- [HTTP API Reference](#http-api-reference)
  - [Device Information](#device-information)
  - [Plugin Control](#plugin-control)
  - [Display Control](#display-control)
  - [Message Display](#message-display)
  - [Plugin Scheduler](#plugin-scheduler)
  - [Storage Management](#storage-management)
- [DDP (Display Data Protocol)](#ddp-display-data-protocol)
- [Home Assistant Integration](#home-assistant-integration)
- [Development](#development)
  - [Plugin Development](#plugin-development)
- [Troubleshooting](#troubleshooting)

## Features

<details>
  <summary>Click to expand feature list</summary>

**General Features:**

- Persist your drawing
- Rotate image
- Live Drawing
- OTA Update
- WiFi Control
- Web GUI
- Load an image
- Switch plugin by pressing the button
- Schedule Plugins to switch after "n" seconds

**Available Plugins:**

- Draw
- Game of Life
- Breakout
- Snake
- Stars
- Lines
- Circle
- Clock
- Big Clock
- Weather
- Rain
- Animation (with Animation Creator in Web UI)
- Firework
- DDP (Display Data Protocol)
- Pong Clock

</details>

## Quick Start

https://github.com/user-attachments/assets/ddf91be1-2c95-4adc-b178-05b0781683cc

Control the lamp using the built-in web GUI. Find the device IP address via:

- Serial monitor output
- Router admin panel

---

## Hardware Setup

This software is designed for ESP32 Dev Boards but can work with other Arduino boards (WiFi, OTA, and web server features will need to be removed for non-ESP boards).

**Supported Boards:**

- ESP32 Dev Board (recommended)
- TTGO LoRa32 V2.1 (T3_V1.6.1)
- ESP8266 (with limitations: per-pixel brightness only works when storage and global brightness are disabled)

<img src="https://user-images.githubusercontent.com/15351728/200148521-86d0f9e6-2c41-4707-b2d9-8aa24a0e440e.jpg" width="60%" />

### Opening the Lamp

IKEA uses rivets instead of regular screws. To open:

1. Insert a screwdriver between the rivets and back panel
2. Carefully pry open with a second object
3. Alternative: Drill out the rivets (cleaner but permanent)

### Understanding the Panels

<img src="https://user-images.githubusercontent.com/15351728/200183585-39c1668d-665b-4c12-bcbb-387aec1d3874.JPG" width="60%" />

Inside you'll find 4 identical plates with 64 LEDs each (in 4 fields). Focus on the lowest plate:

- 6 connectors at the bottom edge (connection points for your board)
- Original microcontroller at the top (must be removed)

<img src="https://user-images.githubusercontent.com/86414213/205998862-e9962695-1328-49ea-b546-be592cbad3c2.jpg" width="90%" />

### Pin Configuration

Connect the pins as shown below. Remember to configure them in `include/constants.h` according to your board.

|       LCD        | ESP32  | TTGO LoRa32 | NodeMCUv2 | Lolin D32 (Pro) | Xiao ESP32S3 |
| :--------------: | :----: | :---------: | :-------: | :-------------: | :----------: |
|       GND        |  GND   |     GND     |    GND    |       GND       |     GND      |
|       VCC        |   5V   |     5V      |    VIN    |       USB       |     VUSB     |
| EN (PIN_ENABLE)  | GPIO26 |    IO22     | GPIO16 D0 |     GPIO26      |  D4 (GPIO5)  |
|  IN (PIN_DATA)   | GPIO27 |    IO23     | GPIO13 D7 |     GPIO27      |  D10 (MOSI)  |
| CLK (PIN_CLOCK)  | GPIO14 |    IO02     | GPIO14 D5 |     GPIO14      |  D8 (SCK)    |
| CLA (PIN_LATCH)  | GPIO12 |    IO15     | GPIO0 D3  |     GPIO12      |  D5 (GPIO6)  |
|  BUTTON one end  | GPIO16 |    IO21     | GPIO2 D4  |     GPIO25      |  D3 (GPIO4)  |
| BUTTON other end |  GND   |     GND     |    GND    |       GND       |     GND      |

<img src="https://user-images.githubusercontent.com/86414213/205999001-6213fc4f-be2f-4305-a17a-44fdc9349069.jpg" width="60%" />

### Alternate Button Wiring

You can use the original button wiring without adding external connections. See [this issue](https://github.com/ph1p/ikea-led-obegraensad/issues/79) by [RBEGamer](https://github.com/RBEGamer) for details.

---

## Software Setup

### ESP32 Setup with VS Code and PlatformIO

1. **Prerequisites**
   - Install [Visual Studio Code](https://code.visualstudio.com/)
   - Install the PlatformIO IDE extension from VS Code Extensions Marketplace

2. **Clone the Project**

   ```bash
   git clone git@github.com:ph1p/ikea-led-obegraensad.git
   cd ikea-led-obegraensad
   code .
   ```

   PlatformIO will automatically load dependencies.

3. **Connect ESP32**
   - Connect your ESP32 via USB
   - Verify the COM port in the PlatformIO Devices tab

4. **Configure the Project**
   - Run `PlatformIO: Clean` (Recycle bin icon in bottom toolbar)
   - Edit `include/secrets.h` with your WiFi credentials (ESP8266 only; ESP32 can use WiFi Manager)
   - Configure variables in `include/constants.h`

5. **Build the Project**
   - Click the `PlatformIO Build` icon (bottom toolbar)
   - If libraries are missing, install them via the PlatformIO Libraries tab
   - Repeat `Clean` and `Build` until successful

6. **Upload to ESP32**
   - Click `PlatformIO Upload` (bottom toolbar)

### WiFi Configuration

**ESP32 (WiFi Manager - Recommended):**

This project uses [tzapu's WiFiManager](https://github.com/tzapu/WiFiManager). After booting:
1. Device attempts to connect to known access points
2. If none available, creates network named `Ikea Display Setup WiFi`
3. Connect to this network on any device
4. Captive portal guides you through WiFi configuration
5. Device reboots and connects to your network

**Network name can be changed via `WIFI_MANAGER_SSID` in `include/constants.h`.*

**ESP8266 (Manual Configuration):**

For ESP8266, WiFi Manager is not available. Set `WIFI_SSID` and `WIFI_PASSWORD` in `include/secrets.h`.

---

## OTA Updates

Over-The-Air (OTA) updates allow you to upload new firmware wirelessly without a USB connection. This is powered by [ElegantOTA](https://github.com/ayushsharma82/ElegantOTA).

### Configuration

Before using OTA, configure the following:

1. **Set OTA Credentials** in `include/secrets.h`:
   ```cpp
   #define OTA_USERNAME "admin"
   #define OTA_PASSWORD "your-password"
   ```

2. **Configure Upload Settings** in `platformio.ini` (for the `esp32dev` environment):
   ```ini
   extra_scripts = upload.py
   upload_protocol = custom
   custom_upload_url = http://192.168.178.50  # Your device IP
   custom_username = admin
   custom_password = your-password
   ```

**Note:** Replace `192.168.178.50` with your device's actual IP address.

### Upload Methods

#### Method 1: Web Interface (Manual Upload)

1. Navigate to `http://your-device-ip/update` in your browser
2. Login with your configured credentials (default: `admin` / `ikea-led-wall`)
3. Select your firmware file (`.pio/build/esp32dev/firmware.bin`)
4. Click "Update" and wait for completion
5. Device will automatically reboot with new firmware

#### Method 2: PlatformIO (Automated Upload)

Upload directly from PlatformIO via the command line:

```bash
pio run -e esp32dev -t upload
```

Or use the PlatformIO Upload button in VS Code (bottom toolbar).

**Requirements:**
- Python packages: `requests_toolbelt` and `tqdm`
- Install if needed: `pip install requests_toolbelt tqdm`

### Visual Feedback

During OTA updates, the LED matrix provides visual feedback:

- **"U" letter displayed**: Update has started
- **Serial output**: Progress updates every second
- **"R" letter displayed**: Update completed (device will reboot)

Monitor the serial output for detailed progress:
```
OTA update started!
OTA Progress Current: 262144 bytes, Final: 1440655 bytes
OTA Progress Current: 524288 bytes, Final: 1440655 bytes
...
OTA update finished successfully!
```

**Troubleshooting:**
- Ensure device is connected to the same network
- Verify IP address in `platformio.ini` matches device IP
- Check credentials match in both `secrets.h` and `platformio.ini`
- For upload failures, try the web interface method first

---

## HTTP API Reference

Base URL: `http://your-server/api`

### Device Information

**Get device info, current state, and available plugins**

```http
GET /api/info
```

**Example:**
```bash
curl http://your-server/api/info
```

**Response:**
```json
{
  "rows": 16,
  "cols": 16,
  "status": "active",
  "plugin": 3,
  "rotation": 90,
  "brightness": 255,
  "scheduleActive": true,
  "schedule": [
    {"pluginId": 2, "duration": 60},
    {"pluginId": 4, "duration": 120}
  ],
  "plugins": [
    {"id": 1, "name": "Plugin One"},
    {"id": 2, "name": "Plugin Two"}
  ]
}
```

---

### Plugin Control

**Set active plugin by ID**

```http
PATCH /api/plugin?id={plugin_id}
```

**Example:**
```bash
curl -X PATCH "http://your-server/api/plugin?id=7"
```

**Response:**
```json
{
  "status": "success",
  "message": "Plugin set successfully"
}
```

**Error Response:**
```json
{
  "error": true,
  "errormessage": "Could not set plugin to id 7"
}
```

---

### Display Control

#### Set Brightness

```http
PATCH /api/brightness?value={0-255}
```

**Example:**
```bash
curl -X PATCH "http://your-server/api/brightness?value=100"
```

**Response:**
```json
{
  "status": "success",
  "message": "Brightness set successfully"
}
```

#### Get Display Data

Retrieve current display data as a byte-array. Each byte represents pixel brightness (0-255). Global brightness is applied after these values.

```http
GET /api/data
```

**Example:**
```bash
curl http://your-server/api/data
```

**Response:**
```json
[255, 255, 255, 0, 128, 255, 255, 0, ...]
```

---

### Message Display

#### Display a Message

Display scrolling text and/or graphs on the LED matrix.

```http
GET /api/message
```

**Parameters:**
- `text` (optional): Text message to display
- `graph` (optional): Comma-separated integers (0-15) representing a graph
- `miny` (optional): Graph lower bound (default: 0)
- `maxy` (optional): Graph upper bound (default: 15)
- `repeat` (optional): Repetition count (default: 1, use `-1` for infinite)
- `id` (optional): Unique identifier for the message
- `delay` (optional): Scroll delay in ms (default: 50)

**Example:**
```bash
curl "http://your-server/api/message?text=Hello&graph=8,5,2,1,0,0,1,4,7,10,13,14,15,15,14,11&repeat=3&id=1&delay=60"
```

**Response:**
```json
{
  "status": "success",
  "message": "Message received"
}
```

#### Remove a Message

```http
GET /api/removemessage?id={message_id}
```

**Example:**
```bash
curl "http://your-server/api/removemessage?id=1"
```

---

### Plugin Scheduler

Automatically switch between plugins on a schedule.

#### Set Schedule

```http
POST /api/schedule
```

**Example:**
```bash
curl -X POST http://your-server/api/schedule -d 'schedule=[{"pluginId":10,"duration":2},{"pluginId":8,"duration":5}]'
```

#### Start Schedule

```http
GET /api/schedule/start
```

#### Stop Schedule

```http
GET /api/schedule/stop
```

#### Clear Schedule

```http
GET /api/schedule/clear
```

---

### Storage Management

#### Clear Storage

```http
GET /api/clearstorage
```

**Example:**
```bash
curl http://your-server/api/clearstorage
```

---

## DDP (Display Data Protocol)

DDP enables real-time LED matrix control via UDP packets. External applications can send pixel data directly over the network.

### Quick Start

1. **Enable DDP Plugin**
   ```bash
   curl -X PATCH "http://your-server/api/plugin?id=17"
   ```

2. **Send Pixels**
   ```bash
   python3 ddp.py --ip 192.168.178.50 --fill 128
   ```

### Using ddp.py

The included Python script (`ddp.py`) simplifies DDP packet creation.

**Clear all pixels:**
```bash
python3 ddp.py --ip 192.168.178.50 --clear
```

**Fill display with brightness value:**
```bash
python3 ddp.py --ip 192.168.178.50 --fill 128
```

**Set individual pixels (X, Y, brightness):**
```bash
python3 ddp.py --ip 192.168.178.50 --pixel 0 0 255 --pixel 15 15 128
```

**Options:**
- `--ip`: Display IP address (default: 192.168.178.50)
- `--port`: UDP port (default: 4048)
- `--clear`: Clear all pixels
- `--fill BRIGHTNESS`: Fill with brightness (0-255)
- `--pixel X Y BRIGHTNESS`: Set pixel (can be used multiple times)

**Coordinates:**
- Matrix: 16×16 pixels
- X: 0-15 (left to right)
- Y: 0-15 (top to bottom)
- Brightness: 0-255

### Protocol Specification

**Packet Structure:**
```
[Header: 10 bytes][RGB Data: 768 bytes for 16×16]
```

**Header (10 bytes):**
- Byte 0: `0x41` (Version 1)
- Byte 1: `0x00` (Flags)
- Bytes 2-9: `0x00` (Reserved)

**RGB Data:**
- 3 bytes per pixel (R, G, B)
- Total: 16 × 16 × 3 = 768 bytes
- Order: Row-major (left to right, top to bottom)
- Brightness calculated as: `(R + G + B) / 3`

**Single Pixel Mode:**
Send only 3 RGB bytes (total: 13 bytes) to apply the same color to all pixels.

**Example (Python):**
```python
import socket

# Create DDP packet
header = bytearray([0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00])
pixels = bytearray([128, 128, 128] * 256)  # Fill with mid-brightness
packet = header + pixels

# Send packet
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.sendto(packet, ("192.168.178.50", 4048))
sock.close()
```

---

## Home Assistant Integration

### HACS Integration (Recommended)

Use [this Home Assistant Integration](https://github.com/HennieLP/ikea-led-obegraensad-Home-Assistant-Integration) for easy setup with seamless integration and instant updates via websockets.

### HTTP API Integration

Example automation to adjust brightness based on sun position:

**configuration.yaml:**
```yaml
rest_command:
  obegraensad_brightness_high:
    url: "http://your-server/api/brightness/"
    method: PATCH
    content_type: "application/x-www-form-urlencoded"
    payload: "value=100"
  obegraensad_brightness_low:
    url: "http://your-server/api/brightness/"
    method: PATCH
    content_type: "application/x-www-form-urlencoded"
    payload: "value=1"
```

**Automation (Settings → Automations → Edit in YAML):**
```yaml
alias: Obegraensad low brightness
description: ""
triggers:
  - trigger: sun
    event: sunset
    offset: 0
conditions: []
actions:
  - action: rest_command.obegraensad_brightness_low
    data: {}
mode: single
```

Create a second automation or condition to call `rest_command.obegraensad_brightness_high` at sunrise.

---

## Development

### Arduino/C++ Development

**Structure:**
- `src/` - Arduino code
- `platformio.ini` - Build configuration
  - See [OTA Updates](#ota-updates) section for wireless firmware upload configuration

### Frontend Development

**Structure:**
- `frontend/` - Web UI code

**Setup:**
```bash
cd frontend
pnpm install
```

**Configuration:**
- Set device IP in `.env`

**Commands:**
- `pnpm dev` - Start development server
- `pnpm build` - Build and generate `webgui.cpp`

**Docker Build:**
```bash
docker compose run node
```

### Code Quality

**Pre-commit Hooks:**
```bash
# Install pre-commit
pip install pre-commit

# Activate hooks
pre-commit install
```

---

### Plugin Development

**1. Create Plugin Files**

**plugins/MyPlugin.h:**
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

**plugins/MyPlugin.cpp:**
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

**2. Register Plugin in main.cpp**

```cpp
#include "plugins/MyPlugin.h"

pluginManager.addPlugin(new MyPlugin());
```

---

## Troubleshooting

### Flickering Panel

- Verify all soldering points, especially VCC
- Ensure adequate power supply to the board
- Check for loose connections
