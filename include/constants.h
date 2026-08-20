#pragma once

#include <Arduino.h>

// disable if you do not want to have online functionality
#define ENABLE_SERVER

// The ESP32 variants keep their SPI flash on different pins, so the generic
// assignment further down is not usable everywhere. On the C3 the flash sits
// on GPIO12-17 (HD/WP/CS0/CLK/MOSI/MISO) and nothing above GPIO21 exists; on
// the S3 it occupies GPIO26-37. Driving any of those pins cuts the CPU off
// from the flash it executes from.
#if defined(CONFIG_IDF_TARGET_ESP32C3)
// Free pins on the C3: GPIO2/8/9 are strapping, 18/19 native USB, 20/21 UART0.
#define PIN_ENABLE 3
#define PIN_DATA 4
#define PIN_CLOCK 5
#define PIN_LATCH 6
#define PIN_BUTTON 7
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
// Matches the Xiao ESP32S3 column of the wiring table in the README:
// D4=GPIO5, D10/MOSI=GPIO9, D8/SCK=GPIO7, D5=GPIO6, D3=GPIO4.
#define PIN_ENABLE 5
#define PIN_DATA 9
#define PIN_CLOCK 7
#define PIN_LATCH 6
#define PIN_BUTTON 4
#elif defined(ESP32)
#define PIN_ENABLE 26
#define PIN_DATA 27
#define PIN_CLOCK 14
#define PIN_LATCH 12
#define PIN_BUTTON 16
#endif

#ifdef ESP8266
#define PIN_ENABLE 16
#define PIN_DATA 13
#define PIN_CLOCK 14
#define PIN_LATCH 0
#define PIN_BUTTON 2
#endif

// disable if you do not want to use the internal storage
// https://randomnerdtutorials.com/esp32-save-data-permanently-preferences/
// timer1 on esp8266 is not compatible with flash file system reads
#ifndef ESP8266
#define ENABLE_STORAGE
#endif

#ifdef ENABLE_SERVER
// https://github.com/nayarsystems/posix_tz_db/blob/master/zones.json
#define NTP_SERVER "de.pool.ntp.org"
#define TZ_INFO "CET-1CEST,M3.5.0,M10.5.0/3"
#endif

#define COLS 16
#define ROWS 16

// Display constants
constexpr uint8_t MAX_BRIGHTNESS = 255;
constexpr uint16_t TOTAL_PIXELS = ROWS * COLS;

// set your city or coords (https://github.com/chubin/wttr.in)
#define WEATHER_LOCATION "Hamburg"

// name of WiFi created by the device if no known WiFi is available
#define WIFI_MANAGER_SSID "IKEA"

// use ALL of the following to use static IP config
/*
#define IP_ADDRESS "192.168.0.250"
#define SUBNET "255.255.255.0"
#define DNS1 "1.1.1.1"
#define DNS2 "8.8.8.8"
#define GWY "192.168.0.1"
*/

// ---------------

enum SYSTEM_STATUS
{
  NONE,
  WSBINARY,
  UPDATE,
  LOADING,
};

extern volatile SYSTEM_STATUS currentStatus;
