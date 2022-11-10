#include "mode/circle.h"

void Circle::drawCircle(int x, int y, int radius, int onOff)
{
    for (int i = 0; i < 360; i++)
    {
        double radians = i * PI / 180;
        double px = x + radius * cos(radians);
        double py = y + radius * sin(radians);
        setPixel(mode_buffer, px, py, onOff);
    }
}

void Circle::loop()
{
    String key = String(this->circleStep) + "-" + String(this->innerCircleStep) + "-" + String(this->innerStarted);
    bool memoExists = this->memo.count(key) > 0;

    if (this->circleStep > 4 && !this->innerStarted)
    {
        this->innerStarted = true;
    }

    if (this->innerStarted)
    {
        if (!memoExists)
        {
            drawCircle(8, 8, this->innerCircleStep, 0);
        }
        this->innerCircleStep++;
    }
    if (this->innerCircleStep > 10)
    {
        this->innerCircleStep = 0;
        this->innerStarted = false;
    }

    if (!memoExists)
    {
        drawCircle(8, 8, this->circleStep, 1);
    }
    this->circleStep++;

    if (this->circleStep > 10)
    {
        this->circleStep = 0;
    }

    if (this->circleStep % everyN == 0)
    {
        if (!memoExists)
        {
            std::copy(mode_buffer, mode_buffer + (COLS * ROWS), this->memo[key]);
        }
        else
        {
            renderScreen(this->memo[key]);
            delay(200);
        }
    }
}
