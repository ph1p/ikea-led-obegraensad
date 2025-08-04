#pragma once
#include <vector>
#include "screen.h"

class Message
{
public:
  int id;
  int repeat;
  int delay;
  std::string text;
  std::vector<int> graph;
  int miny;
  int maxy;

  void reset()
  {
    id = 0;
    repeat = 0;
    delay = 0;
    text.clear();
    graph.clear();
    miny = 0;
    maxy = 0;
  }
};

class MessagePool
{
private:
  static const size_t POOL_SIZE = 10;
  Message pool[POOL_SIZE];
  bool used[POOL_SIZE];

public:
  MessagePool()
  {
    for (size_t i = 0; i < POOL_SIZE; i++)
    {
      used[i] = false;
      pool[i].reset();
    }
  }

  Message *acquire()
  {
    for (size_t i = 0; i < POOL_SIZE; i++)
    {
      if (!used[i])
      {
        used[i] = true;
        pool[i].reset();
        return &pool[i];
      }
    }
    return nullptr;
  }

  void release(Message *msg)
  {
    ptrdiff_t index = msg - pool;
    if (index >= 0 && index < static_cast<ptrdiff_t>(POOL_SIZE))
    {
      used[index] = false;
      pool[index].reset();
    }
  }
};

class Messages_
{
private:
  Messages_() = default;
  MessagePool messagePool;
  std::vector<Message *> activeMessages;

  int previousMinute = -1;
  int previousSecond = -1;
  int indicatorPixel = 0;

public:
  static Messages_ &getInstance();

  Messages_(const Messages_ &) = delete;
  Messages_ &operator=(const Messages_ &) = delete;

  void add(std::string text, int repeat = 0, int id = 0, int delay = 50,
           std::vector<int> graph = {}, int miny = 0, int maxy = 15);
  void remove(int id = 0);
  void scroll();
  void scrollMessageEveryMinute();

  ~Messages_()
  {
    // Clean up any remaining messages
    for (Message *msg : activeMessages)
    {
      messagePool.release(msg);
    }
    activeMessages.clear();
  }
};

extern Messages_ &Messages;