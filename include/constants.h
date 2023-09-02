#pragma once

#ifdef ESP32
#define PIN_ENABLE 26
#define PIN_DATA 27
#define PIN_CLOCK 14
#define PIN_LATCH 12
#define PIN_BUTTON 19
#endif

#ifdef ESP8266
#define PIN_ENABLE 16
#define PIN_DATA 13
#define PIN_CLOCK 14
#define PIN_LATCH 0
#define PIN_BUTTON 2
#endif

// disable if you do not want to have online functionality
#define ENABLE_SERVER

// disable if you do not want to use the internal storage
// https://randomnerdtutorials.com/esp32-save-data-permanently-preferences/
// timer1 on esp8266 is not compatible with flash file system reads
#ifndef ESP8266
#define ENABLE_STORAGE
#endif

#ifdef ENABLE_SERVER
// MDNS hostname will be whatever is put below + '.local', e.g. obegransad.local
#define MDNS_HOSTNAME "obegransad"
#define NTP_SERVER "de.pool.ntp.org"
#define TZ_INFO "WEST-1DWEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00"
#endif

#define COLS 16
#define ROWS 16

// set your city or coords (https://github.com/chubin/wttr.in)
#define WEATHER_LOCATION "51.233334,6.783333"
