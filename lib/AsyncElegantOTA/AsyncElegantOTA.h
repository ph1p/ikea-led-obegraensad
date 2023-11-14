#ifndef AsyncElegantOTA_h
#define AsyncElegantOTA_h

#include <Arduino.h>
#include <stdlib_noniso.h>

#ifdef ESP32
#include <AsyncTCP.h>
#include <Update.h>
#include <esp_int_wdt.h>
#include <esp_task_wdt.h>
#endif
#ifdef ESP8266
#include <ESPAsyncTCP.h>
#include <flash_hal.h>
#include <FS.h>
#endif

#include "Hash.h"
#include "ESPAsyncWebServer.h"
#include "elegantWebpage.h"

class AsyncElegantOtaClass
{

public:
  void
  setID(const char *id),
      begin(AsyncWebServer *server, const char *username = "", const char *password = ""),
      loop(),
      onOTAStart(void callable(void)),
      onOTAProgress(void callable(void)),
      onOTAEnd(void callable(void)),
      restart();

private:
  AsyncWebServer *_server;

  String getID();

  String _id = getID();
  String _username = "";
  String _password = "";
  bool _authRequired = false;
  bool _preUpdateRequired = false;
  bool _progressUpdateRequired = false;
  bool _postUpdateRequired = false;
  void (*preUpdateCallback)();
  void (*progressUpdateCallback)();
  void (*postUpdateCallback)();
};

extern AsyncElegantOtaClass AsyncElegantOTA;

#endif