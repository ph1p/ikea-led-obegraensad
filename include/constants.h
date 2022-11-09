#ifndef CONSTANTS_HEADER_H
#define CONSTANTS_HEADER_H

#define PIN_ENABLE 26
#define PIN_DATA 27
#define PIN_CLOCK 14
#define PIN_LATCH 12
#define PIN_BUTTON 16

#define ENABLE_SERVER true

#ifndef ENABLE_ACCELEROMETER
#define ENABLE_ACCELEROMETER false
#endif

// define your own
#if ENABLE_ACCELEROMETER
  #define PIN_X_AXIS 35
  #define PIN_Y_AXIS 33
  #define PIN_Z_AXIS 32

  #define ZERO_X 9.21
  #define ZERO_Y 9.25
  #define ZERO_Z 11.28
  #define SENSITIVITY 0.01
#endif

#define COLS 16
#define ROWS 16

#endif