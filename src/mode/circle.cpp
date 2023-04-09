#include "mode/circle.h"

void Circle::setup()
{
    this->circleStep = 0;
}

void Circle::loop()
{
    std::vector<int> bits = Screen.readBytes(this->frames[this->circleStep]);

    for (uint i = 0; i < bits.size(); i++)
    {
        Screen.setPixelAtIndex(i, bits[i]);
    }

    this->circleStep++;
    if (this->circleStep > 14)
    {
        this->circleStep = 7;
    }
    delay(200);
}
