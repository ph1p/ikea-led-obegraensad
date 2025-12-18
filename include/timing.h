#pragma once

#include <Arduino.h>

/**
 * Non-blocking delay helper class
 *
 * Usage:
 *   NonBlockingDelay timer;
 *
 *   void loop() {
 *     if (timer.isReady(1000)) {
 *       // This code runs every 1000ms without blocking
 *     }
 *   }
 */
class NonBlockingDelay
{
private:
  unsigned long previousMillis = 0;
  bool firstRun = true;

public:
  /**
   * Check if the specified interval has elapsed
   * @param interval Time in milliseconds
   * @return true if interval has elapsed, false otherwise
   */
  bool isReady(unsigned long interval)
  {
    unsigned long currentMillis = millis();

    // Allow first run to execute immediately
    if (firstRun)
    {
      previousMillis = currentMillis;
      firstRun = false;
      return true;
    }

    if (currentMillis - previousMillis >= interval)
    {
      previousMillis = currentMillis;
      return true;
    }

    return false;
  }

  /**
   * Reset the timer
   */
  void reset()
  {
    previousMillis = millis();
    firstRun = false;
  }

  /**
   * Force the next isReady() call to return true
   */
  void forceReady()
  {
    firstRun = true;
  }
};
