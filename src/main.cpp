#include <Arduino.h>
#include <SPI.h>
#ifdef ESP32
#include <WiFi.h>
#include <ESPmDNS.h>
#endif

#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif

#include "constants.h"
#include "mode/mode.h"
#include "websocket.h"
#include "secrets.h"
#include "ota.h"
#include "webserver.h"
#include "screen.h"
#include "mode/mode.h"

unsigned long previousMillis = 0;
unsigned long interval = 30000;

void setup()
{
  Serial.begin(115200);

  pinMode(PIN_LATCH, OUTPUT);
  pinMode(PIN_CLOCK, OUTPUT);
  pinMode(PIN_DATA, OUTPUT);
  pinMode(PIN_ENABLE, OUTPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP);

// server
#ifdef ENABLE_SERVER
  // wifi
  int attempts = 0;
  WiFi.setHostname(WIFI_HOSTNAME);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED && attempts < 7)
  {
    delay(2500);
    Serial.print(".");
    attempts++;
  }
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Couldn't connect to WiFi, resetting");
    ESP.restart();
  }
  else
  {
    Serial.println("");
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
  }

  // mdns
  #ifdef MDNS_HOSTNAME
  if (!MDNS.begin(MDNS_HOSTNAME))
  {
    Serial.println("Error setting up MDNS responder!");
  }
  Serial.println("mDNS responder started");
  #endif

  // set time server
  configTzTime(TZ_INFO, NTP_SERVER);

  initOTA(server);
  initWebsocketServer(server);
  initWebServer();
#endif

  Screen.setup();
  Screen.clear();
  loadMode();
  Screen.loadFromStorage();
}

void loop()
{
#ifdef ESP8266
  listenOnButtonToChangeMode();
#endif
  loopOfAllModes();
  unsigned long currentMillis = millis();
  // if WiFi is down, try reconnecting every CHECK_WIFI_TIME seconds
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >= interval))
  {
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();

    WiFi.reconnect();
    previousMillis = currentMillis;
  }
#ifdef ENABLE_SERVER
  cleanUpClients();
#endif
}
