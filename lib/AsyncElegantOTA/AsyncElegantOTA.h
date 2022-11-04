#ifndef AsyncElegantOTA_h
#define AsyncElegantOTA_h

#include "Arduino.h"
#include "stdlib_noniso.h"

#if defined(ESP8266)
#include "ESP8266WiFi.h"
#include "ESPAsyncTCP.h"
#include "flash_hal.h"
#include "FS.h"
#elif defined(ESP32)
#include "WiFi.h"
#include "AsyncTCP.h"
#include "Update.h"
#include "esp_int_wdt.h"
#include "esp_task_wdt.h"
#endif

#include "Hash.h"
#include "ESPAsyncWebServer.h"
#include "FS.h"

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