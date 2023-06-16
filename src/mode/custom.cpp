#include "mode/custom.h"

std::vector<std::vector<int>> customAnimationFrames;

void Custom::setup()
{
    this->step = 0;
}

void Custom::loop()
{
    int size = customAnimationFrames.size();

    if (size > 0)
    {
        std::vector<int> bits = Screen.readBytes(customAnimationFrames[this->step]);

        for (int i = 0; i < bits.size(); i++)
        {
            Screen.setPixelAtIndex(i, bits[i]);
        }

        this->step++;

        if (this->step >= size)
        {
            this->step = 0;
        }
        delay(400);
    }
}
