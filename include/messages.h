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
};

class Messages_
{

private:
  Messages_() = default;
  std::vector<Message> messages;
  int previousMinute;
  int previousSecond;
  int indicatorPixel;


public:
  static Messages_ &getInstance();

  Messages_(const Screen_ &) = delete;
  Messages_ &operator=(const Screen_ &) = delete;

  void add(std::string text, int repeat = 0, int id = 0, int delay = 50, std::vector<int> graph = {}, int miny = 0, int maxy = 15);

  void remove(int id = 0);

  void scroll();
  void scrollMessageEveryMinute();
};

extern Messages_ &Messages;

void handleMessage(AsyncWebServerRequest *request);
void handleRemove(AsyncWebServerRequest *request);
