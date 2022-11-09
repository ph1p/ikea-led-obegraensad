/*
    ADXL335.h
    Library for accelerometer_ADXL335
    Copyright (c) 2013 seeed technology inc.
    Author        :   FrankieChu
    Create Time   :   Jan 2013
    Change Log    :
    The MIT License (MIT)
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:
    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/

#include <Arduino.h>
#include "ADXL335.h"

void ADXL335::begin(uint16_t pinX, uint16_t pinY, uint16_t pinZ, float sensitivity)
{
  this->pinX = pinX;
  this->pinY = pinY;
  this->pinZ = pinZ;
  this->sensitivity = sensitivity;
  pinMode(this->pinX, INPUT);
  pinMode(this->pinY, INPUT);
  pinMode(this->pinZ, INPUT);
  scale = (float)this->sensitivity * ADC_AMPLITUDE / ADC_REF;
}
void ADXL335::getXYZ(uint16_t *x, uint16_t *y, uint16_t *z)
{
  *x = analogRead(this->pinX);
  *y = analogRead(this->pinY);
  *z = analogRead(this->pinZ);
}
void ADXL335::getAcceleration(float *ax, float *ay, float *az)
{
  uint16_t x, y, z;
  float xvoltage, yvoltage, zvoltage;
  getXYZ(&x, &y, &z);

  xvoltage = (float)x * ADC_REF / ADC_AMPLITUDE;
  yvoltage = (float)y * ADC_REF / ADC_AMPLITUDE;
  zvoltage = (float)z * ADC_REF / ADC_AMPLITUDE;

  *ax = (xvoltage - ZERO_X) / this->sensitivity;
  *ay = (yvoltage - ZERO_Y) / this->sensitivity;
  *az = (zvoltage - ZERO_Z) / this->sensitivity;
}
float ADXL335::getAccelerationX()
{
  uint16_t x, y, z;
  float xvoltage, ax;
  getXYZ(&x, &y, &z);
  xvoltage = (float)x * ADC_REF / ADC_AMPLITUDE;
  ax = (xvoltage - ZERO_X) / this->sensitivity;
  return ax;
}
float ADXL335::getAccelerationY()
{
  uint16_t x, y, z;
  float yvoltage, ay;
  getXYZ(&x, &y, &z);
  yvoltage = (float)y * ADC_REF / ADC_AMPLITUDE;
  ay = (yvoltage - ZERO_Y) / this->sensitivity;
  return ay;
}
float ADXL335::getAccelerationZ()
{
  uint16_t x, y, z;
  float zvoltage, az;
  getXYZ(&x, &y, &z);
  zvoltage = (float)z * ADC_REF / ADC_AMPLITUDE;
  az = (zvoltage - ZERO_Z) / this->sensitivity;
  return az;
}