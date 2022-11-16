#include <Arduino.h>
#include <WiFi.h>

#include "constants.h"
#include "mode/mode.h"
#include "websocket.h"
#include "secrets.h"
#include "ota.h"
#include "webserver.h"
#include "screen.h"
#include "accelerometer.h"
#include "mode/mode.h"

#ifdef ENABLE_ACCELEROMETER
Accelerometer accelerometer;
#endif

void setup()
{
  Serial.begin(115200);

  pinMode(PIN_LATCH, OUTPUT);
  pinMode(PIN_CLOCK, OUTPUT);
  pinMode(PIN_DATA, OUTPUT);
  pinMode(PIN_ENABLE, OUTPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP);

  Screen.clear();
  loadMode();
  Screen.loadFromStorage();

// server
#ifdef ENABLE_SERVER
  // wifi
  int attempts = 0;
  WiFi.setHostname(WIFI_HOSTNAME);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED && attempts < 7)
  {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Resetting");
    ESP.restart();
  }
  else
  {
    Serial.println("");
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
  }

  // set time server
  configTzTime(TZ_INFO, NTP_SERVER);

  initOTA(server);
  initWebsocketServer(server);
  initWebServer();
#endif

// uncomment to figure out the initial parameters
#ifdef ENABLE_ACCELEROMETER
  // accelerometer.startCalibration();
  accelerometer.setup();
#endif
}

void loop()
{
#ifdef ENABLE_ACCELEROMETER
  accelerometer.loop();
#endif
  loopOfAllModes();

#ifdef ENABLE_SERVER
  cleanUpClients();
#endif
  delay(20);
}
