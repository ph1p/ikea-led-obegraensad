#include "plugins/SmokeClockPlugin.h"
#include <math.h>

// --- Tunables ------------------------------------------------------------
static constexpr unsigned long FRAME_MS = 33; // ~30 simulation steps per second

// Iterations of the pressure solver. More = the flow gets closer to truly
// incompressible (rounder swirls), fewer = cheaper. 20 is plenty for 16x10.
static constexpr int PROJECT_ITERS = 20;

// How strongly smoke rises. This is the main "mood" knob: higher values give
// fast, flickering plumes, lower values a slow, lazy drift.
static constexpr float BUOYANCY = 0.030f;

// Vorticity confinement. Brings back small eddies the coarse grid loses.
// 0 = smooth, laminar blobs; around 0.3 = lively curls; too high = chaos.
static constexpr float VORTICITY = 0.25f;

// How much velocity survives each step (a little friction keeps it calm).
static constexpr float VELOCITY_DAMPING = 0.985f;

// How much smoke survives each step. 0.99 at 30 steps per second means a
// puff has faded to half after roughly 2.3 seconds of flowing around.
static constexpr float SMOKE_DECAY = 0.990f;

// Emitters: how much smoke a source adds per step at full puff, and how hard
// it pushes that smoke upwards.
static constexpr float EMIT_SMOKE = 0.25f;
static constexpr float EMIT_PUSH = 0.12f;

// Speed of the puffing rhythm (radians per step). 0.028 means one puff
// roughly every 7.5 seconds per source.
static constexpr float PUFF_SPEED_1 = 0.028f;
static constexpr float PUFF_SPEED_2 = 0.0223f;

// How fast the two sources wander left and right along the bottom.
static constexpr float WANDER_SPEED_1 = 0.0061f;
static constexpr float WANDER_SPEED_2 = 0.0047f;

// Brightness mapping: 255 * (1 - e^(-smoke * EXPOSURE)). Soft roll-off, so
// thick smoke saturates gently instead of clipping to a flat white blob.
static constexpr float EXPOSURE = 2.5f;

// Black point: this much smoke is still drawn fully black. Keeps the thin
// haze that spreads everywhere over time from turning the panel grey, so only
// the actual clouds light up. 0 = off, ~0.1-0.2 = good contrast.
static constexpr float BLACK_POINT = 0.12f;

// Glow of the empty fluid area. 0 = empty areas are fully off, which gives
// the best contrast on the real panel.
static constexpr float AMBIENT = 0.0f;

static constexpr uint8_t DIGIT_BRIGHTNESS = 255;
static constexpr unsigned long TIME_POLL_MS = 1000;
static constexpr float TWO_PI_F = 6.28318530718f;

static inline float wrapPhase(float p)
{
  while (p >= TWO_PI_F)
    p -= TWO_PI_F;
  return p;
}

void SmokeClockPlugin::setup()
{
  Screen.clear();

  for (int k = 0; k < SIZE; k++)
  {
    u[k] = v[k] = u0[k] = v0[k] = dens[k] = dens0[k] = 0.0f;
  }
  for (int i = 0; i < ROWS * COLS; i++)
    digitMask[i] = false;

  // Random starting points so every activation looks a bit different. The
  // two puff phases start half a cycle apart so the sources take turns.
  emPhase[0] = (random(0, 1000) / 1000.0f) * TWO_PI_F;
  emPhase[1] = (random(0, 1000) / 1000.0f) * TWO_PI_F;
  puffPhase[0] = 1.2f; // close to a puff, so something happens right away
  puffPhase[1] = puffPhase[0] + TWO_PI_F * 0.5f;

  previousMinute = -1;
  hasTime = false;
  lastTimeCheck = millis() - TIME_POLL_MS;
}

// Wall conditions. The fluid is a closed box: velocity pointing into a wall
// is mirrored (b = 1 for x-velocity at the side walls, b = 2 for y-velocity
// at top and bottom), everything else just copies its neighbour.
void SmokeClockPlugin::setBoundary(int b, float *x)
{
  for (int i = 1; i <= NX; i++)
  {
    x[IX(i, 0)] = (b == 2) ? -x[IX(i, 1)] : x[IX(i, 1)];
    x[IX(i, NY + 1)] = (b == 2) ? -x[IX(i, NY)] : x[IX(i, NY)];
  }
  for (int j = 1; j <= NY; j++)
  {
    x[IX(0, j)] = (b == 1) ? -x[IX(1, j)] : x[IX(1, j)];
    x[IX(NX + 1, j)] = (b == 1) ? -x[IX(NX, j)] : x[IX(NX, j)];
  }
  x[IX(0, 0)] = 0.5f * (x[IX(1, 0)] + x[IX(0, 1)]);
  x[IX(0, NY + 1)] = 0.5f * (x[IX(1, NY + 1)] + x[IX(0, NY)]);
  x[IX(NX + 1, 0)] = 0.5f * (x[IX(NX, 0)] + x[IX(NX + 1, 1)]);
  x[IX(NX + 1, NY + 1)] = 0.5f * (x[IX(NX, NY + 1)] + x[IX(NX + 1, NY)]);
}

// Move a quantity along with the flow. Instead of pushing every cell forward
// (which can blow up), each cell looks *backwards* along the velocity and
// takes whatever was there one step ago. That is the "stable" in Stable Fluids.
void SmokeClockPlugin::advect(int b, float *d, const float *d0, const float *uu, const float *vv)
{
  for (int j = 1; j <= NY; j++)
  {
    for (int i = 1; i <= NX; i++)
    {
      float x = i - uu[IX(i, j)];
      float y = j - vv[IX(i, j)];

      if (x < 0.5f)
        x = 0.5f;
      if (x > NX + 0.5f)
        x = NX + 0.5f;
      if (y < 0.5f)
        y = 0.5f;
      if (y > NY + 0.5f)
        y = NY + 0.5f;

      int i0 = (int)x;
      int j0 = (int)y;
      float s1 = x - i0, s0 = 1.0f - s1;
      float t1 = y - j0, t0 = 1.0f - t1;

      // Blend the four cells around the point we landed on.
      d[IX(i, j)] = s0 * (t0 * d0[IX(i0, j0)] + t1 * d0[IX(i0, j0 + 1)]) +
                    s1 * (t0 * d0[IX(i0 + 1, j0)] + t1 * d0[IX(i0 + 1, j0 + 1)]);
    }
  }
  setBoundary(b, d);
}

// Remove every bit of the flow that would squeeze fluid together or pull it
// apart. What is left can only go *around*, which is what creates swirls.
// u0 and v0 are borrowed as scratch space (divergence and pressure).
void SmokeClockPlugin::project()
{
  float *div = u0;
  float *p = v0;

  for (int j = 1; j <= NY; j++)
  {
    for (int i = 1; i <= NX; i++)
    {
      div[IX(i, j)] = -0.5f * (u[IX(i + 1, j)] - u[IX(i - 1, j)] + v[IX(i, j + 1)] - v[IX(i, j - 1)]);
      p[IX(i, j)] = 0.0f;
    }
  }
  setBoundary(0, div);
  setBoundary(0, p);

  // Gauss-Seidel: repeatedly replace each pressure value with the average of
  // its neighbours plus the local divergence, until it settles.
  for (int k = 0; k < PROJECT_ITERS; k++)
  {
    for (int j = 1; j <= NY; j++)
    {
      for (int i = 1; i <= NX; i++)
      {
        p[IX(i, j)] =
            (div[IX(i, j)] + p[IX(i - 1, j)] + p[IX(i + 1, j)] + p[IX(i, j - 1)] + p[IX(i, j + 1)]) *
            0.25f;
      }
    }
    setBoundary(0, p);
  }

  for (int j = 1; j <= NY; j++)
  {
    for (int i = 1; i <= NX; i++)
    {
      u[IX(i, j)] -= 0.5f * (p[IX(i + 1, j)] - p[IX(i - 1, j)]);
      v[IX(i, j)] -= 0.5f * (p[IX(i, j + 1)] - p[IX(i, j - 1)]);
    }
  }
  setBoundary(1, u);
  setBoundary(2, v);
}

// Find where the flow spins and give it a small push to keep spinning.
// u0 is borrowed to hold the spin ("curl") of every cell.
void SmokeClockPlugin::vorticityConfinement()
{
  float *curl = u0;

  for (int j = 1; j <= NY; j++)
  {
    for (int i = 1; i <= NX; i++)
    {
      curl[IX(i, j)] =
          0.5f * ((v[IX(i + 1, j)] - v[IX(i - 1, j)]) - (u[IX(i, j + 1)] - u[IX(i, j - 1)]));
    }
  }

  for (int j = 2; j < NY; j++)
  {
    for (int i = 2; i < NX; i++)
    {
      // Direction towards stronger spin...
      float dx = 0.5f * (fabsf(curl[IX(i + 1, j)]) - fabsf(curl[IX(i - 1, j)]));
      float dy = 0.5f * (fabsf(curl[IX(i, j + 1)]) - fabsf(curl[IX(i, j - 1)]));
      float len = sqrtf(dx * dx + dy * dy) + 1e-5f;
      dx /= len;
      dy /= len;

      // ...and a push at right angles to it, in the direction of the spin.
      float w = curl[IX(i, j)];
      u[IX(i, j)] += VORTICITY * dy * w;
      v[IX(i, j)] -= VORTICITY * dx * w;
    }
  }
}

// Two sources wander along the bottom and let off smoke in slow puffs.
void SmokeClockPlugin::addSources()
{
  for (int k = 0; k < 2; k++)
  {
    // Puff envelope: sin clipped at zero and squared, so each source is quiet
    // for half of its cycle and then swells up smoothly.
    float s = sinf(puffPhase[k]);
    if (s <= 0.0f)
      continue;
    float strength = s * s;

    // Position along the bottom edge, kept a bit away from the side walls.
    float fx = 0.5f * (NX + 1) + (0.5f * NX - 2.5f) * sinf(emPhase[k] + k * 2.1f);
    int i0 = (int)fx;
    float frac = fx - i0;

    for (int n = 0; n < 2; n++)
    {
      int i = i0 + n;
      if (i < 1 || i > NX)
        continue;
      float weight = (n == 0) ? (1.0f - frac) : frac;
      float amount = strength * weight;

      for (int j = NY - 1; j <= NY; j++)
      {
        dens[IX(i, j)] += EMIT_SMOKE * amount;
        v[IX(i, j)] -= EMIT_PUSH * amount; // negative y = upwards
      }
    }
  }

  for (int k = 0; k < 2; k++)
    emPhase[k] = wrapPhase(emPhase[k] + (k == 0 ? WANDER_SPEED_1 : WANDER_SPEED_2));
  puffPhase[0] = wrapPhase(puffPhase[0] + PUFF_SPEED_1);
  puffPhase[1] = wrapPhase(puffPhase[1] + PUFF_SPEED_2);
}

void SmokeClockPlugin::stepSimulation()
{
  addSources();

  // Smoke rises: every cell gets an upward push proportional to its smoke.
  for (int j = 1; j <= NY; j++)
    for (int i = 1; i <= NX; i++)
      v[IX(i, j)] -= BUOYANCY * dens[IX(i, j)];

  vorticityConfinement();
  project();

  // Carry the velocity along with itself. The old velocity is copied to
  // u0/v0 first, because the new one is written in place.
  for (int k = 0; k < SIZE; k++)
  {
    u0[k] = u[k];
    v0[k] = v[k];
  }
  advect(1, u, u0, u0, v0);
  advect(2, v, v0, u0, v0);
  project();

  for (int k = 0; k < SIZE; k++)
  {
    u[k] *= VELOCITY_DAMPING;
    v[k] *= VELOCITY_DAMPING;
    dens0[k] = dens[k];
  }

  // Carry the smoke along with the (now swirl-only) flow, then let it fade.
  advect(0, dens, dens0, u, v);
  for (int k = 0; k < SIZE; k++)
    dens[k] *= SMOKE_DECAY;
}

void SmokeClockPlugin::stampDigit(int ox, int digit)
{
  if (digit < 0 || digit > 9)
    return;

  // Same glyphs and layout as PongClock: smallNumbers, 4 px wide, x offset -1.
  const int width = 4;
  const size_t bitCount = sizeof(smallNumbers[0]) * 8;
  for (size_t bit = 0; bit < bitCount; bit++)
  {
    if (!bitmapBit(smallNumbers[digit], bit))
      continue;
    int x = ox + bit % width - 1;
    int y = bit / width;
    if (x >= 0 && x < COLS && y >= 0 && y < ROWS)
      digitMask[y * COLS + x] = true;
  }
}

void SmokeClockPlugin::buildDigitMask()
{
  for (int i = 0; i < ROWS * COLS; i++)
    digitMask[i] = false;

  stampDigit(0, (timeinfo.tm_hour - timeinfo.tm_hour % 10) / 10);
  stampDigit(4, timeinfo.tm_hour % 10);
  stampDigit(9, (timeinfo.tm_min - timeinfo.tm_min % 10) / 10);
  stampDigit(13, timeinfo.tm_min % 10);
}

void SmokeClockPlugin::loop()
{
  if (!timer.isReady(FRAME_MS))
    return;

  // The time only changes once a minute, so poll the clock once a second.
  if (millis() - lastTimeCheck >= TIME_POLL_MS)
  {
    lastTimeCheck = millis();

    if (getSyncedLocalTime(timeinfo))
    {
      if (timeinfo.tm_min != previousMinute)
      {
        buildDigitMask();
        previousMinute = timeinfo.tm_min;
      }
      hasTime = true;
    }
    else
    {
      hasTime = false;
      previousMinute = -1;
    }
  }

  stepSimulation();

  for (uint8_t y = 0; y < ROWS; y++)
  {
    for (uint8_t x = 0; x < COLS; x++)
    {
      uint8_t brightness = 0;

      if (y < FLUID_TOP)
      {
        // Clock panel: white digits on black (stays black without a time).
        if (hasTime && digitMask[y * COLS + x])
          brightness = DIGIT_BRIGHTNESS;
      }
      else
      {
        float d = dens[IX(x + 1, y - FLUID_TOP + 1)] - BLACK_POINT;
        if (d < 0.0f)
          d = 0.0f;
        float level = AMBIENT + (255.0f - AMBIENT) * (1.0f - expf(-d * EXPOSURE));
        if (level > 255.0f)
          level = 255.0f;
        brightness = (uint8_t)level;
      }

      Screen.setPixel(x, y, 1, brightness);
    }
  }
}

void SmokeClockPlugin::teardown()
{
  Screen.clear();
}

const char *SmokeClockPlugin::getName() const
{
  return "SmokeClock";
}
