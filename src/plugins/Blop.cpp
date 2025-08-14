#include "plugins/Blop.h"
#include "screen.h"  // Uses global Screen

BlobPlugin::BlobPlugin()
{
    // Nothing special here, setup() will handle init
}

void BlobPlugin::setup()
{
    Screen.clear();

    for (auto &b : balls) {
        b.x = static_cast<float>(rand() % X_MAX);
        b.y = static_cast<float>(rand() % Y_MAX);
        b.vx = ((rand() % 200) / 100.0f - 1.0f) * SPEED * 2;
        b.vy = ((rand() % 200) / 100.0f - 1.0f) * SPEED * 2;
    }
}

void BlobPlugin::loop()
{
    Screen.clear();

    for (uint8_t y = 0; y < ROWS; ++y) {
        for (uint8_t x = 0; x < COLS; ++x) {
            float value = 0.0f;

            for (const auto &b : balls) {
                float dx = b.x - static_cast<float>(x);
                float dy = b.y - static_cast<float>(y*aspect_ratio);
                float dist = std::sqrt(dx * dx + dy * dy);
                value += attenuation(dist, RADIUS);
            }

            uint8_t brightness = toneMap(value);
            Screen.setPixel(x, y, 1, brightness);
        }
    }

    updatePositions();
    delay(50); // ~20 FPS
}

const char* BlobPlugin::getName() const
{
    return "Blobs";
}

float BlobPlugin::attenuation(float d, float radius) const
{
    if (d > radius) return 0.0f;
    float ratio = d / radius;
    return (1.0f - ratio * ratio) * (1.0f - ratio * ratio);
}

uint8_t BlobPlugin::toneMap(float v) const
{
    if (v > CAP_VALUE) v = CAP_VALUE;
    float n = v / CAP_VALUE;
    float corrected = std::pow(n, 1.0f / GAMMA);
    return static_cast<uint8_t>(corrected * 255.0f);
}

void BlobPlugin::updatePositions()
{
    for (auto &b : balls) {
        b.x += b.vx;
        b.y += b.vy;

        if (b.x < 0) { b.x = 0; b.vx *= -1; }
        if (b.x >= X_MAX) { b.x = X_MAX - 1; b.vx *= -1; }
        if (b.y < 0) { b.y = 0; b.vy *= -1; }
        if (b.y >= Y_MAX) { b.y = Y_MAX - 1; b.vy *= -1; }
    }
}
