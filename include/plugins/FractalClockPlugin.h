#pragma once

#include "PluginManager.h"
#include "timing.h"

/**
 * FractalClockPlugin
 *
 * The current time drawn like PongClock / SmokeClock (HH MM in one row at the
 * top, bright digits on a solid black panel), with an endless Mandelbrot zoom
 * underneath.
 *
 * The zoom dives into one of a few hand-picked spots on the edge of the
 * Mandelbrot set. The ESP32 only has a fast FPU for float (32 bit), which
 * runs out of precision at roughly 100000x magnification. So once the view
 * gets that small, the picture fades to black and the next dive starts.
 *
 * Brightness comes from the "smooth" iteration count (how quickly a point
 * escapes), run through a cosine so it forms soft bands. Those bands slowly
 * drift outwards, so the picture keeps moving even between zoom steps.
 * Points inside the set stay black.
 *
 * Every LED is the average of SUPERSAMPLE x SUPERSAMPLE samples, which keeps
 * the 16x10 picture from flickering while it zooms.
 */
class FractalClockPlugin : public Plugin
{
public:
  // Rows 0..5 belong to the clock panel (digits are 5 rows + 1 row padding),
  // the fractal fills the rest of the screen.
  static constexpr int FRACTAL_TOP = 6;
  static constexpr int FW = COLS;               // fractal pixels across
  static constexpr int FH = ROWS - FRACTAL_TOP; // fractal pixels down

private:
  NonBlockingDelay timer;

  float zoomT;       // how far into the current dive (log of magnification)
  float colorPhase;  // drifting offset of the brightness bands
  int target;        // index into the list of zoom targets
  int fadeOutFrames; // counts down while fading out, -1 = not fading

  bool digitMask[ROWS * COLS];
  int previousMinute;
  bool hasTime;
  unsigned long lastTimeCheck;

  struct tm timeinfo;

  void startDive(int newTarget);
  float samplePoint(float cr, float ci, int maxIter) const;

  void buildDigitMask();
  void stampDigit(int ox, int digit);

public:
  void setup() override;
  void loop() override;
  void teardown() override;
  const char *getName() const override;
};
