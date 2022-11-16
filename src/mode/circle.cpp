#include "mode/circle.h"

void Circle::setup()
{
    Screen.clear();
}

void Circle::loop()
{
    listenOnButtonToChangeMode();

    std::vector<int> bits = Screen.readBytes(circleAnimation[this->circleStep]);

    for (int i = 0; i < bits.size(); i++)
    {
        Screen.setPixelAtIndex(i, bits[i]);
    }

    Screen.render();
    this->circleStep++;
    if (this->circleStep > 14)
    {
        this->circleStep = 7;
    }
    delay(200);
}
