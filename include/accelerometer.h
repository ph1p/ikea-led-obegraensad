#pragma once

#include "constants.h"

#ifdef ENABLE_ACCELEROMETER
#include "websocket.h"
#include "ADXL335.h"

class Accelerometer
{
private:
  uint16_t zero_x, zero_y, zero_z;
  uint16_t max_x, max_y, max_z;
  String prevDirection;
  String direction;
  float sensitivity;
  ADXL335 accelerometer;
  void calibrate(uint16_t *_x, uint16_t *_y, uint16_t *_z);

public:
  void setup();
  void startCalibration();
  void loop();
};

#endif
