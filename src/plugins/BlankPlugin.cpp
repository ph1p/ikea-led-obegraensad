#include "plugins/BlankPlugin.h"


void BlankPlugin::setup()
{
    // loading screen
    Screen.clear();
    currentStatus = NONE;
}

void BlankPlugin::loop()
{

}

void BlankPlugin::update()
{
}

const char *BlankPlugin::getName() const
{
    return "Blank";
}
