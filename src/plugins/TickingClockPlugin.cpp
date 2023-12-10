#include "plugins/TickingClockPlugin.h"

void TickingClockPlugin::setup()
{
  // loading screen
  Screen.setPixel(4, 7, 1);
  Screen.setPixel(5, 7, 1);
  Screen.setPixel(7, 7, 1);
  Screen.setPixel(8, 7, 1);
  Screen.setPixel(10, 7, 1);
  Screen.setPixel(11, 7, 1);

  previousMinutes = -1;
  previousHour = -1;
}

void TickingClockPlugin::loop()
{
  if (getLocalTime(&timeinfo))
  { 

    if ( (timeinfo.tm_hour*60+timeinfo.tm_min) < 6*60+30  || 
         (timeinfo.tm_hour*60+timeinfo.tm_min) > 22*60+30 ) //only between 6:30 and 22:30
    { 
        Screen.clear();
        return;
    }

    if (previousHour != timeinfo.tm_hour || previousMinutes != timeinfo.tm_min)
    {
      if(previousHour==-1)Screen.scrollText("Welcome to Ticking Clock!");

      std::vector<int> hh = {(timeinfo.tm_hour - timeinfo.tm_hour % 10) / 10, timeinfo.tm_hour % 10};
      std::vector<int> mm = {(timeinfo.tm_min - timeinfo.tm_min % 10) / 10, timeinfo.tm_min % 10};
     
      Screen.clear();
     
      Screen.drawCharacter(2,0, Screen.readBytes(system6x7[hh[0]+16]), 8, Screen.getCurrentBrightness());
      Screen.drawCharacter(9,0, Screen.readBytes(system6x7[hh[1]+16]), 8, Screen.getCurrentBrightness());
      Screen.drawCharacter(2,9, Screen.readBytes(system6x7[mm[0]+16]), 8, Screen.getCurrentBrightness());
      Screen.drawCharacter(9,9, Screen.readBytes(system6x7[mm[1]+16]), 8, Screen.getCurrentBrightness());
      previousMinutes = timeinfo.tm_min;
      previousHour = timeinfo.tm_hour;    
    }
    if(previousSecond != timeinfo.tm_sec){
 
      // clear second lane
      Screen.drawRectangle(0,7,16,2,true,0);
      // alternating second pixel
      if ((timeinfo.tm_sec * 32 / 60) % 2 == 0)      
         Screen.setPixel(timeinfo.tm_sec*16/60,7,1, Screen.getCurrentBrightness());
      else 
         Screen.setPixel(timeinfo.tm_sec*16/60,8,1 Screen.getCurrentBrightness());

      previousSecond =  timeinfo.tm_sec;
    }

  }
}

const char *TickingClockPlugin::getName() const
{
  return "Ticking Clock";
}
