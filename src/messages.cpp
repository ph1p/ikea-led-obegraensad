#include "messages.h"
#include <SPI.h>

using namespace std;

Messages_ &Messages_::getInstance()
{
    static Messages_ instance;
    return instance;
}

// std::string text, int repeat = 0, int id = 0; std::vector<int> graph = {}
void Messages_::add(std::string text, int repeat, int id, std::vector<int> graph)
{
    messages.emplace_back(Message{id, repeat, text, graph});
    //scroll();
    previousMinute=-1; // force to show message imedeately in next loop. Apparently http handlers cant run to long
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
        if(it->text.length()>0)  Screen.scrollText(it->text.c_str());
        if(it->graph.size()>0) Screen.scrollGraph(it->graph);
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
            // Update the previous minute to the current minute
            previousMinute = timeinfo.tm_min;

            // Call the scroll function to display messages
            scroll();
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

    // Call the add function with the extracted parameters
    Messages.add(text, repeat, id, graph);

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