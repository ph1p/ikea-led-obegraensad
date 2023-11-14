#include "plugins/DrawPlugin.h"

void DrawPlugin::setup()
{
  Screen.clear();
  if (Screen.isCacheEmpty())
  {
    Screen.loadFromStorage();
  }
  else
  {
    Screen.restoreCache();
  }
#ifdef ENABLE_SERVER
  sendInfo();
#endif
}

void DrawPlugin::teardown()
{
  Screen.cacheCurrent();
}

void DrawPlugin::websocketHook(DynamicJsonDocument &request)
{
  const char *event = request["event"];

  if (currentStatus == NONE)
  {
    if (!strcmp(event, "led"))
    {
      Screen.setPixelAtIndex(request["index"], request["status"]);
    }
    else if (!strcmp(event, "clear"))
    {
      Screen.clear();
    }
    else if (!strcmp(event, "screen"))
    {
      uint8_t buffer[ROWS * COLS];
      for (int i = 0; i < ROWS * COLS; i++)
      {
        buffer[i] = request["data"][i];
      }
      Screen.setRenderBuffer(buffer);
    }
    else if (!strcmp(event, "persist"))
    {
      Screen.persist();
    }
    else if (!strcmp(event, "load"))
    {
      Screen.loadFromStorage();

#ifdef ENABLE_SERVER
      sendInfo();
#endif
    }
  }
}

const char *DrawPlugin::getName() const
{
  return "Draw";
}