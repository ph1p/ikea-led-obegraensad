#pragma once

// disable if you do not want to have online functionality
#define ENABLE_SERVER

#ifdef ESP32
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
#define TZ_INFO "GMT0BST,M3.5.0/1,M10.5.0"
#endif

#define COLS 16
#define ROWS 16

// weather update frequency (minutes)
#define WEATHER_UPDATE_FREQ 10


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

extern SYSTEM_STATUS currentStatus;