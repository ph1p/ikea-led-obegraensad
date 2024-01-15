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

// http://your-server/message?text=Hello&repeat=3&id=42&graph=1,2,3,4
void handleMessage(AsyncWebServerRequest *request)
{

    // Extracting parameters from the URL
    std::string text = request->arg("text").c_str();
    int repeat = request->arg("repeat").toInt();
    int id = request->arg("id").toInt();
    int delay = request->arg("delay").toInt();
    int miny = request->arg("miny").toInt();
    int maxy = request->arg("maxy").toInt();

    // if no delay has been passed, use 50 ms
    if (delay <= 0)
        delay = 50;

    // default maxy to 15
    if (maxy == 0)
        maxy = 15;

    // Extracting the 'graph' parameter as a comma-separated list of integers
    std::string graphParam = request->arg("graph").c_str();
    std::vector<int> graph;

    char *token = strtok(const_cast<char *>(graphParam.c_str()), ",");
    while (token != nullptr)
    {
        // Convert the substring to an integer and add it to the vector
        graph.push_back(std::stoi(token));
        token = strtok(nullptr, ",");
    }

    // Uncomment this to thee what charcode comes in
    //  for(int i = 0;i<text.size();i++)Screen.scrollText(std::to_string(text[i]));

    // Call the add function with the extracted parameters
    Messages.add(text, repeat, id, delay, graph, miny, maxy);

    // Send a response to the client
    request->send(200, "text/plain", "Message received");
}

// http://your-server/removemessage?id=42
void handleRemove(AsyncWebServerRequest *request)
{

    int id = request->arg("id").toInt();

    // Call the add function with the extracted parameters
    Messages.remove(id);

    // Send a response to the client
    request->send(200, "text/plain", "Message received");
}