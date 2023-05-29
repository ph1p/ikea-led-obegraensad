#pragma once

#define PIN_ENABLE 26
#define PIN_DATA 27
#define PIN_CLOCK 14
#define PIN_LATCH 12
#define PIN_BUTTON 16

// disable if you do not want to have online functionality
#define ENABLE_SERVER

// disable if you do not want ALEXA connectivity, only works if ENABLE_SERVER is defined
#define ENABLE_ALEXA

// disable if you do not want to use the internal storage
// https://randomnerdtutorials.com/esp32-save-data-permanently-preferences/
#define ENABLE_STORAGE

#ifdef ENABLE_SERVER
#define NTP_SERVER "de.pool.ntp.org"
#define TZ_INFO "WEST-1DWEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00"
#endif

#define COLS 16
#define ROWS 16
