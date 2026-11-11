#pragma once

#include "PluginManager.h"
#include "timing.h"

/**
 * SmokeClockPlugin
 *
 * The current time drawn like PongClock (HH MM in one row at the top, bright
 * digits on a solid black panel), with a small fluid simulation
 * underneath: two slowly wandering sources at the bottom let off puffs of
 * smoke, which rise, hit the underside of the clock panel, spread out along
 * it and curl into little eddies before they fade away.
 *
 * The solver is Jos Stam's "Stable Fluids" (the classic real-time fluid
 * method from games), cut down to what a 16x10 grid needs:
 *   - advect:  carry velocity and smoke along with the flow
 *   - project: make the flow incompressible (no fluid appears or vanishes),
 *              which is what turns a straight push into swirls
 *   - vorticity confinement: gives back the small swirls the coarse grid
 *              would otherwise smear out
 *   - buoyancy: smoke rises
 *
 * The fluid only lives below the clock panel, so the panel acts as a lid.
 * All buffers are fixed-size members: nothing is allocated per frame.
 */
class SmokeClockPlugin : public Plugin
{
public:
  // Rows 0..5 belong to the clock panel (digits are 5 rows + 1 row padding),
  // the fluid fills the rest of the screen.
  static constexpr int FLUID_TOP = 6;
  static constexpr int NX = COLS;             // fluid cells across
  static constexpr int NY = ROWS - FLUID_TOP; // fluid cells down
  // Every field has a one-cell border around it for the wall conditions.
  static constexpr int SIZE = (NX + 2) * (NY + 2);

private:
  NonBlockingDelay timer;

  float u[SIZE], v[SIZE];   // velocity (x right, y down), cells per step
  float u0[SIZE], v0[SIZE]; // scratch: previous velocity / pressure solve
  float dens[SIZE];         // amount of smoke per cell
  float dens0[SIZE];        // scratch: previous smoke

  // Emitter state: phases for position and puffing rhythm.
  float emPhase[2];
  float puffPhase[2];

  bool digitMask[ROWS * COLS];
  int previousMinute;
  bool hasTime;
  unsigned long lastTimeCheck;

  struct tm timeinfo;

  static inline int IX(int i, int j)
  {
    return i + (NX + 2) * j;
  }

  void setBoundary(int b, float *x);
  void advect(int b, float *d, const float *d0, const float *uu, const float *vv);
  void project();
  void vorticityConfinement();
  void addSources();
  void stepSimulation();

  void buildDigitMask();
  void stampDigit(int ox, int digit);

public:
  void setup() override;
  void loop() override;
  void teardown() override;
  const char *getName() const override;
};
