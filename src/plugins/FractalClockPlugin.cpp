#include "plugins/FractalClockPlugin.h"
#include <math.h>

// --- Tunables ------------------------------------------------------------
static constexpr unsigned long FRAME_MS = 33; // ~30 frames per second

// How fast to zoom: the view shrinks by e^ZOOM_SPEED per frame. 0.012 at
// 30 fps doubles the magnification roughly every 2 seconds.
static constexpr float ZOOM_SPEED = 0.012f;

// Iteration budget. Deeper zoom needs more iterations to show detail, so it
// grows by ITER_PER_OCT for every doubling of the magnification.
static constexpr int BASE_ITER = 32;
static constexpr float ITER_PER_OCT = 10.0f;

// Samples per LED along each axis (2 = 4 samples). Smooths out flicker.
static constexpr int SUPERSAMPLE = 2;

// Width of the brightness bands: smaller = wider, calmer bands.
static constexpr float BANDS = 0.35f;

// How fast the bands drift (radians per frame). 0 = they stand still.
static constexpr float CYCLE = 0.04f;

// This much brightness (0..1) is still drawn fully black. Keeps faint
// haze off the panel so the bands stand out.
static constexpr float BLACK_POINT = 0.05f;

// When the view gets narrower than this, float precision runs out and the
// picture would turn into blocks. Then we fade out and start the next dive.
static constexpr float MIN_WIDTH = 2e-5f;

// Length of the fade in / fade out, in frames.
static constexpr int FADE = 45;

// Width of the view at the start of every dive.
static constexpr float START_WIDTH = 3.5f;

static constexpr uint8_t DIGIT_BRIGHTNESS = 255;
static constexpr unsigned long TIME_POLL_MS = 1000;
static constexpr float TWO_PI_F = 6.28318530718f;

// Spots on the edge of the Mandelbrot set that stay interesting all the way
// down. The zoom always heads straight for one of them.
struct ZoomTarget
{
  float x, y;
};
static const ZoomTarget TARGETS[] = {
    {-0.743643887037151f, 0.131825904205330f}, // seahorse valley
    {-0.101096363845622f, 0.956286510809142f},
    {0.001643721971153f, -0.822467633298876f},
    {-1.250660f, 0.020120f},
    {0.360240443437614f, -0.641313061064803f},
    {-0.170337f, -1.06506f},
};
static constexpr int TARGET_COUNT = sizeof(TARGETS) / sizeof(TARGETS[0]);

void FractalClockPlugin::setup()
{
  Screen.clear();

  for (int i = 0; i < ROWS * COLS; i++)
    digitMask[i] = false;

  colorPhase = 0.0f;
  startDive(random(0, TARGET_COUNT));

  previousMinute = -1;
  hasTime = false;
  lastTimeCheck = millis() - TIME_POLL_MS;
}

void FractalClockPlugin::startDive(int newTarget)
{
  target = newTarget % TARGET_COUNT;
  zoomT = 0.0f;
  fadeOutFrames = -1;
}

// Brightness (0..1) of one point c = cr + ci*i. Repeats z = z^2 + c and
// counts how long z stays small. Points that never escape are inside the
// set and stay black.
float FractalClockPlugin::samplePoint(float cr, float ci, int maxIter) const
{
  float zr = 0.0f, zi = 0.0f;
  float zr2 = 0.0f, zi2 = 0.0f;
  int n = 0;

  while (n < maxIter)
  {
    zr2 = zr * zr;
    zi2 = zi * zi;
    if (zr2 + zi2 >= 16.0f)
      break;
    zi = 2.0f * zr * zi + ci;
    zr = zr2 - zi2 + cr;
    n++;
  }

  if (n >= maxIter)
    return 0.0f;

  // The "smooth" iteration count: a fractional n, so neighbouring pixels
  // blend into each other instead of forming hard steps.
  float mag2 = zr * zr + zi * zi;
  float nu = n + 1.0f - log2f(log2f(mag2) * 0.5f);

  return 0.5f - 0.5f * cosf(nu * BANDS - colorPhase);
}

void FractalClockPlugin::stampDigit(int ox, int digit)
{
  if (digit < 0 || digit > 9)
    return;

  // Same glyphs and layout as PongClock / SmokeClock.
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

void FractalClockPlugin::buildDigitMask()
{
  for (int i = 0; i < ROWS * COLS; i++)
    digitMask[i] = false;

  stampDigit(0, (timeinfo.tm_hour - timeinfo.tm_hour % 10) / 10);
  stampDigit(4, timeinfo.tm_hour % 10);
  stampDigit(9, (timeinfo.tm_min - timeinfo.tm_min % 10) / 10);
  stampDigit(13, timeinfo.tm_min % 10);
}

void FractalClockPlugin::loop()
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

  // Advance the zoom and the band drift.
  zoomT += ZOOM_SPEED;
  colorPhase += CYCLE;
  if (colorPhase >= TWO_PI_F)
    colorPhase -= TWO_PI_F;

  float width = START_WIDTH * expf(-zoomT);
  if (width < MIN_WIDTH && fadeOutFrames < 0)
    fadeOutFrames = FADE;

  // Fade in at the start of a dive, fade out at the end.
  float fade = zoomT / (ZOOM_SPEED * FADE);
  if (fade > 1.0f)
    fade = 1.0f;
  if (fadeOutFrames >= 0)
  {
    float out = (float)fadeOutFrames / FADE;
    if (out < fade)
      fade = out;
  }

  int maxIter = BASE_ITER + (int)(ITER_PER_OCT * log2f(START_WIDTH / width));
  float scale = width / FW;
  float cx = TARGETS[target].x;
  float cy = TARGETS[target].y;
  const float subStep = 1.0f / SUPERSAMPLE;

  for (uint8_t y = 0; y < ROWS; y++)
  {
    for (uint8_t x = 0; x < COLS; x++)
    {
      uint8_t brightness = 0;

      if (y < FRACTAL_TOP)
      {
        // Clock panel: white digits on black (stays black without a time).
        if (hasTime && digitMask[y * COLS + x])
          brightness = DIGIT_BRIGHTNESS;
      }
      else
      {
        // Average several samples spread across this LED.
        float sum = 0.0f;
        int fy = y - FRACTAL_TOP;
        for (int sy = 0; sy < SUPERSAMPLE; sy++)
        {
          float ci = cy + (fy + (sy + 0.5f) * subStep - FH * 0.5f) * scale;
          for (int sx = 0; sx < SUPERSAMPLE; sx++)
          {
            float cr = cx + (x + (sx + 0.5f) * subStep - FW * 0.5f) * scale;
            sum += samplePoint(cr, ci, maxIter);
          }
        }

        float level = sum / (SUPERSAMPLE * SUPERSAMPLE) * fade;
        level = (level - BLACK_POINT) / (1.0f - BLACK_POINT);
        if (level < 0.0f)
          level = 0.0f;
        if (level > 1.0f)
          level = 1.0f;
        brightness = (uint8_t)(level * 255.0f);
      }

      Screen.setPixel(x, y, 1, brightness);
    }
  }

  if (fadeOutFrames >= 0)
  {
    fadeOutFrames--;
    if (fadeOutFrames < 0)
      startDive(target + 1);
  }
}

void FractalClockPlugin::teardown()
{
  Screen.clear();
}

const char *FractalClockPlugin::getName() const
{
  return "FractalClock";
}
