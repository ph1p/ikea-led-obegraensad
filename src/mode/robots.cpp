#include "mode/robots.h"

void animateRobot(std::vector<std::vector<int>> frames)
{
    for (int count = 0; count < frames.size(); count++){

        listenOnButtonToChangeMode();
        std::vector<int> bits = Screen.readBytes(frames[count]);

        for (int i = 0; i < bits.size(); i++)
        {
            Screen.setPixelAtIndex(i, bits[i]);
        }

        Screen.render();

        delay(400);
    }
}

void Robots::setup()
{
  this->count = 0;
}

void Robots::loop()
{
    int numberOfAnimations = this->robotAnimations.size();

    std::vector<std::vector<int>> frames = this->robotAnimations[this->count];

    animateRobot(frames);
    animateRobot(frames);
    animateRobot(frames);
    animateRobot(frames);
    animateRobot(frames);

    this->count++;
    if (this->count >= numberOfAnimations)
    {
        this->count = 0;
    }
}
