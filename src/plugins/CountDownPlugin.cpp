#include "plugins/CountDownPlugin.h"

void CountDownPlugin::setup()
{
  // loading screen
  Screen.setPixel(4, 7, 1);
  Screen.setPixel(5, 7, 1);
  Screen.setPixel(7, 7, 1);
  Screen.setPixel(8, 7, 1);
  Screen.setPixel(10, 7, 1);
  Screen.setPixel(11, 7, 1);

  previousSeconds = -1;

  getLocalTime(&targetTimeInfo);
  targetTimeInfo.tm_hour = 23;
  targetTimeInfo.tm_min = 59;
  targetTimeInfo.tm_sec = 59;
  targetTimeInfo.tm_mon = 11;
  targetTimeInfo.tm_mday = 31;
}

void CountDownPlugin::loop()
{
  if (getLocalTime(&localTimeInfo))
  {
    time_t currentTimeInS = mktime(&localTimeInfo);
    time_t newYearInS = mktime(&targetTimeInfo);

    double secondsLeft = difftime(newYearInS, currentTimeInS);
    if (secondsLeft < 0) 
        secondsLeft *= -1.;

    if (previousSeconds != secondsLeft)
    {
        Screen.clear();

        if (localTimeInfo.tm_year == 124)
        {
            std::vector<int> yearA = {2, 0};
            std::vector<int> yearB = {2, 4};

            Screen.drawBigNumbers(0, 0, yearA);
            Screen.drawBigNumbers(0, ROWS / 2, yearB);

            if ((int)secondsLeft % 2 == 0)
            {
                Screen.invertBuffer();
            }
        }
        else if (secondsLeft < 60)
        {
            std::vector<int> sec = {(((int)secondsLeft) % 60) / 10, (((int)secondsLeft) % 10)};

            Screen.drawBigNumbers(0, ROWS / 2, sec);

            if (((int)secondsLeft % 60 < 11) && (sec[1] % 2 == 0))
            {
                Screen.invertBuffer();
            }
        }
        else 
        {
            int minutes = ((int)secondsLeft)/60;
            std::vector<int> min = {minutes / 100, (minutes % 100) / 10, minutes % 10};
            std::vector<int> sec = {(((int)secondsLeft) % 60) / 10, (((int)secondsLeft) % 10)};

            Screen.drawNumbers(0, 2, min);
            Screen.drawBigNumbers(0, ROWS / 2, sec);
        }
    }

    previousSeconds = secondsLeft;
  }
}

const char *CountDownPlugin::getName() const
{
  return "Count Down";
}
