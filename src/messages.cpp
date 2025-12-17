#include "messages.h"
#include <SPI.h>

Messages_ &Messages_::getInstance()
{
  static Messages_ instance;
  return instance;
}

void Messages_::add(std::string text, int repeat, int id, int delay,
                    std::vector<int> graph, int miny, int maxy)
{
  // First remove any existing message with same id
  remove(id);

  // Get a message from the pool
  Message *msg = messagePool.acquire();
  if (msg)
  {
    msg->id = id;
    msg->repeat = repeat;
    msg->delay = delay;
    msg->text = text;
    msg->graph = graph;
    msg->miny = miny;
    msg->maxy = maxy;

    activeMessages.push_back(msg);
    previousMinute = -1; // Force immediate display
	
	// When first message is added, cache current screen and set system status
	if (!screenCached)
	{
		Screen.cacheCurrent();
		screenCached = true;
	}
	currentStatus = MESSAGES;
  }
  else
  {
    Serial.println("Warning: Message pool exhausted!");
  }
}

void Messages_::remove(int id)
{
  // Find and remove message with matching id
  auto it = std::find_if(activeMessages.begin(), activeMessages.end(),
                         [id](const Message *msg)
                         { return msg->id == id; });

  if (it != activeMessages.end())
  {
    messagePool.release(*it);
    activeMessages.erase(it);
    // If no more messages are active, restore screen cache and reset status
    if (activeMessages.empty())
    {
      if (screenCached)
      {
        Screen.restoreCache();
        screenCached = false;
      }
      currentStatus = NONE;
    }
  }
}

void Messages_::scroll()
{
  Screen.persist();

  for (auto it = activeMessages.begin(); it != activeMessages.end();)
  {
    Message *msg = *it;

    // Print text and graph for the message
    if (msg->text.length() > 0)
      Screen.scrollText(msg->text.c_str(), msg->delay);
    if (msg->graph.size() > 0)
      Screen.scrollGraph(msg->graph, msg->miny, msg->maxy, msg->delay);

    if (msg->repeat != -1)
    {
      if (--(msg->repeat) < 0)
      {
        messagePool.release(msg);
        it = activeMessages.erase(it);
        // If we've just removed the last message, restore previous screen
        if (activeMessages.empty())
        {
          if (screenCached)
          {
            Screen.restoreCache();
            screenCached = false;
          }
          currentStatus = NONE;
        }
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
}

void Messages_::scrollMessageEveryMinute()
{
  struct tm timeinfo;

  if (getLocalTime(&timeinfo))
  {
    if (timeinfo.tm_min != previousMinute)
    {
      scroll();
      previousMinute = timeinfo.tm_min;
    }

    if (timeinfo.tm_sec != previousSecond)
    {
      if (!activeMessages.empty())
      {
        indicatorPixel = timeinfo.tm_sec & 0b00000001;
        Screen.setPixel(0, 0, indicatorPixel);
      }
      else if (indicatorPixel > 0)
      {
        indicatorPixel = 0;
        Screen.setPixel(0, 0, indicatorPixel);
      }
      previousSecond = timeinfo.tm_sec;
    }
  }
}

Messages_ &Messages = Messages.getInstance();