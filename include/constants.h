#pragma once

#define PIN_ENABLE 38
#define PIN_DATA 37
#define PIN_CLOCK 36
#define PIN_LATCH 35
#define PIN_BUTTON 34

// disable if you do not want to have online functionality
#define ENABLE_SERVER

// disable if you do not want to use the internal storage
#define ENABLE_STORAGE

#ifdef ENABLE_SERVER
#define NTP_SERVER "de.pool.ntp.org"
#define TZ_INFO "WEST-1DWEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00"
#endif

#define COLS 16
#define ROWS 16
