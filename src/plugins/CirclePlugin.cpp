#include "plugins/CirclePlugin.h"

void CirclePlugin::setup()
{
    this->circleStep = 0;
}

void CirclePlugin::loop()
{
    std::vector<int> bits = Screen.readBytes(this->frames[this->circleStep]);

    for (int i = 0; i < bits.size(); i++)
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

const char *CirclePlugin::getName() const
{
    return "Circle";
}
