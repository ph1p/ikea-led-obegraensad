#include "accelerometer.h"

#if ENABLE_ACCELEROMETER == true

void Accelerometer::setup()
{
  accelerometer.begin(PIN_X_AXIS, PIN_Y_AXIS, PIN_Z_AXIS, SENSITIVITY);
}

void Accelerometer::loop()
{
  float roll = atan2(accelerometer.getAccelerationX(), accelerometer.getAccelerationZ()) * 180 / M_PI;
  float pitch = atan2(-accelerometer.getAccelerationY(), accelerometer.getAccelerationZ()) * 180 / M_PI;

  if (roll > 30)
  {
    this->direction = "portrait";
  }
  if (roll > 40 && pitch < -45)
  {
    this->direction = "landscape-opposite";
  }
  if (roll > 40 && pitch > -45)
  {
    this->direction = "landscape";
  }
  if (roll > 50)
  {
    this->direction = "portrait-opposite";
  }

  if (this->prevDirection != this->direction)
  {
    if (this->direction == "portrait")
    {
      rotate(positions, 0);
    }
    else if (this->direction == "landscape")
    {
      rotate(positions, -90);
    }
    else if (this->direction == "landscape-opposite")
    {
      rotate(positions, 90);
    }
    else if (this->direction == "portrait-opposite")
    {
      rotate(positions, 180);
    }

    if (currentMode == NONE)
    {
      delay(10);
      renderScreen(Screen.renderBuffer);
    }
    sendStateAndInfo();
  }
  this->prevDirection = this->direction;
}

void Accelerometer::startCalibration()
{
  accelerometer.begin(PIN_X_AXIS, PIN_Y_AXIS, PIN_Z_AXIS, SENSITIVITY);

  uint16_t x, y, z;
  for (uint16_t i = 0; i < 20; i++)
    accelerometer.getXYZ(&x, &y, &z);
  Serial.println("The calibration starts: ");
  Serial.println("First, make sure that Z-axis direction is straight up");
  delay(100);
  while (Serial.available() > 0)
    Serial.read();
  calibrate(&x, &y, &z);

  zero_x = x;
  zero_y = y;
  max_z = z;
  Serial.println("Second, make sure that X-axis direction is straight up");

  delay(100);
  while (Serial.available() > 0)
    Serial.read();
  calibrate(&x, &y, &z);

  zero_z = z;
  float zero_xv, zero_yv, zero_zv;
  zero_xv = (float)zero_x * ADC_REF / ADC_AMPLITUDE;
  zero_yv = (float)zero_y * ADC_REF / ADC_AMPLITUDE;
  zero_zv = (float)zero_z * ADC_REF / ADC_AMPLITUDE;
  sensitivity = (float)(max_z - zero_z) * ADC_REF / ADC_AMPLITUDE;

  Serial.print("ZERO_X = ");
  Serial.println(zero_xv);
  Serial.print("ZERO_Y = ");
  Serial.println(zero_yv);
  Serial.print("ZERO_Z = ");
  Serial.println(zero_zv);
  Serial.print("SENSITIVITY = ");
  Serial.println(sensitivity, 2);
  Serial.println("please modified the macro definitions with these results in ADXL335.h");
}

void Accelerometer::calibrate(uint16_t *_x, uint16_t *_y, uint16_t *_z)
{
  uint16_t x, y, z;
  uint16_t sum_x, sum_y, sum_z;
  accelerometer.getXYZ(&x, &y, &z);
  float ax, ay, az;
  accelerometer.getAcceleration(&ax, &ay, &az);
  if ((abs(ax) < 0.06) && (abs(ay) < 0.06))
  {

    *_x = x;
    *_y = y;
    *_z = z;
  }
  else if ((abs(ax) < 0.06) && (abs(az) < 0.06))
  {
    *_x = x;
    *_y = y;
    *_z = z;
  }
  else if ((abs(az) < 0.06) && (abs(ay) < 0.06))
  {
    *_x = x;
    *_y = y;
    *_z = z;
  }
  else
    Serial.println("calibrate error");
}

#endif