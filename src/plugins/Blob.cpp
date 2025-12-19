#include "plugins/Blob.h"
#include "constants.h"
#include "screen.h"

BlobPlugin::BlobPlugin()
{
  // Initialize frame buffer
  for (uint16_t i = 0; i < ROWS * COLS; ++i)
  {
    previousBrightness[i] = 0;
  }
}

void BlobPlugin::setup()
{
  Screen.clear();

  // Initialize balls with random positions and velocities
  for (auto &b : balls)
  {
    b.x = static_cast<float>(rand() % X_MAX);
    b.y = static_cast<float>(rand() % Y_MAX);
    b.vx = ((rand() % 200) / 100.0f - 1.0f) * SPEED * 2;
    b.vy = ((rand() % 200) / 100.0f - 1.0f) * SPEED * 2;
  }

  updateTimer.reset();
}

void BlobPlugin::loop()
{
  if (!updateTimer.isReady(UPDATE_INTERVAL_MS))
  {
    return;
  }

  render();
  updatePositions();
}

void BlobPlugin::render()
{
  // Pre-compute aspect ratio scale
  const float aspect_scale = aspect_ratio;

  for (uint8_t y = 0; y < ROWS; ++y)
  {
    const float scaled_y = static_cast<float>(y) * aspect_scale;

    for (uint8_t x = 0; x < COLS; ++x)
    {
      float value = 0.0f;
      const float fx = static_cast<float>(x);

      for (const auto &b : balls)
      {
        float dx = b.x - fx;
        float dy = b.y - scaled_y;
        float d_sq = dx * dx + dy * dy; // No sqrt needed!
        value += attenuationSquared(d_sq, RADIUS_SQ);
      }

      uint8_t brightness = toneMap(value);
      uint16_t idx = y * COLS + x;

      if (previousBrightness[idx] != brightness)
      {
        Screen.setPixel(x, y, 1, brightness);
        previousBrightness[idx] = brightness;
      }
    }
  }
}

const char *BlobPlugin::getName() const
{
  return "Blobs";
}

float BlobPlugin::attenuationSquared(float d_sq, float radius_sq) const
{
  if (d_sq > radius_sq)
    return 0.0f;

  float ratio = d_sq / radius_sq;
  return (1.0f - ratio) * (1.0f - ratio);
}

uint8_t BlobPlugin::toneMap(float v) const
{
  if (v > CAP_VALUE)
    v = CAP_VALUE;
  float n = v / CAP_VALUE;
  float corrected = std::pow(n, 1.0f / GAMMA);
  return static_cast<uint8_t>(corrected * 255.0f);
}

void BlobPlugin::updatePositions()
{
  for (auto &b : balls)
  {
    b.x += b.vx;
    b.y += b.vy;

    if (b.x < 0)
    {
      b.x = 0;
      b.vx *= -1;
    }
    if (b.x >= X_MAX)
    {
      b.x = X_MAX - 1;
      b.vx *= -1;
    }
    if (b.y < 0)
    {
      b.y = 0;
      b.vy *= -1;
    }
    if (b.y >= Y_MAX)
    {
      b.y = Y_MAX - 1;
      b.vy *= -1;
    }
  }
}
