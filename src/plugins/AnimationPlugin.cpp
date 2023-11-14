#include "plugins/AnimationPlugin.h"

void AnimationPlugin::setup()
{
    this->step = 0;
    if (customAnimationFrames.size() == 0)
    {
        Screen.setPixel(7, 4, 1);
        Screen.setPixel(8, 4, 1);
        Screen.setPixel(7, 5, 1);
        Screen.setPixel(8, 5, 1);
        Screen.setPixel(7, 6, 1);
        Screen.setPixel(8, 6, 1);
        Screen.setPixel(7, 7, 1);
        Screen.setPixel(8, 7, 1);
        Screen.setPixel(7, 8, 1);
        Screen.setPixel(8, 8, 1);

        Screen.setPixel(7, 10, 1);
        Screen.setPixel(8, 10, 1);
        Screen.setPixel(7, 11, 1);
        Screen.setPixel(8, 11, 1);
    }
}

void AnimationPlugin::loop()
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

void AnimationPlugin::websocketHook(DynamicJsonDocument &request)
{
    const char *event = request["event"];
    if (!strcmp(event, "upload"))
    {
        int size = (int)request["screens"];

        customAnimationFrames.resize(size);
        for (int i = 0; i < size; i++)
        {
            for (int k = 0; k < 32; k++)
            {
                if (k == 0)
                {
                    customAnimationFrames[i].resize(32);
                }
                customAnimationFrames[i][k] = (int)request["data"][i][k];
            }
        }
    }
}

const char *AnimationPlugin::getName() const
{
    return "Animation";
}
