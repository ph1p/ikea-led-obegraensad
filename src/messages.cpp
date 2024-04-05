#include "messages.h"
#include <SPI.h>

using namespace std;

Messages_ &Messages_::getInstance()
{
    static Messages_ instance;
    return instance;
}

void Messages_::add(std::string text, int repeat, int id, int delay, std::vector<int> graph, int miny, int maxy)
{
    remove(id); // there should be only one message by id
    messages.emplace_back(Message{id, repeat, delay, text, graph, miny, maxy});
    previousMinute = -1; // force the message to be displayed immediately in the next loop. The http handler cannot run for too long
}

void Messages_::remove(int id)
{
    // Use std::remove_if to move the messages to be removed to the end of the vector
    auto it = std::remove_if(messages.begin(), messages.end(), [id](const Message &msg)
                             { return msg.id == id; });

    // Erase the messages marked for removal
    messages.erase(it, messages.end());
}

void Messages_::scroll()
{

    Screen.persist();

    for (auto it = messages.begin(); it != messages.end();)
    {

        // Print the text for each message
        if (it->text.length() > 0)
            Screen.scrollText(it->text.c_str(), it->delay);
        if (it->graph.size() > 0)
            Screen.scrollGraph(it->graph, it->miny, it->maxy, it->delay);

        // if the repeat is not -1, which is the flag for infinite repeat
        if (it->repeat != -1)
        {
            // Decrease repeat and remove if it becomes less than 0
            if (--(it->repeat) < 0)
            {
                it = messages.erase(it);
            }
            else
            {
                ++it;
            }
        }
        else
        {
            ++it;
        }
    }

    // restore old screen
    Screen.loadFromStorage();
}

void Messages_::scrollMessageEveryMinute()
{

    // Structure to hold time information
    struct tm timeinfo;

    // Check if the local time can be obtained
    if (getLocalTime(&timeinfo))
    {
        // Check if the current minute is different from the previous minute
        if (timeinfo.tm_min != previousMinute)
        {
            // Call the scroll function to display messages
            scroll();

            // Update the previous minute to the current minute
            previousMinute = timeinfo.tm_min;
        }

        if (timeinfo.tm_sec != previousSecond)
        {
            if (messages.size() > 0)
            {
                indicatorPixel = previousSecond & 0b00000001; // every other second, bit of a geek style
                Screen.setPixel(0, 0, indicatorPixel);
            }
            else
            { // if the indicator pixel was set, but no message exisits, reset it
                if (indicatorPixel > 0)
                {
                    indicatorPixel = 0;
                    Screen.setPixel(0, 0, indicatorPixel);
                }
            }

            previousSecond = timeinfo.tm_sec;
        }
    }
}

Messages_ &Messages = Messages.getInstance();
