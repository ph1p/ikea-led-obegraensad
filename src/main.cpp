#include <Arduino.h>
#include <WiFi.h>

#include "websocket.h"
#include "storage.h"
#include "mode/mode.h"
#include "secrets.h"
#include "ota.h"
#include "webserver.h"

void setup()
{
  Serial.begin(115200);

  pinMode(PIN_LATCH, OUTPUT);
  pinMode(PIN_CLOCK, OUTPUT);
  pinMode(PIN_DATA, OUTPUT);
  pinMode(PIN_ENABLE, OUTPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP);

  clearScreenAndBuffer(render_buffer);

  // https://randomnerdtutorials.com/esp32-save-data-permanently-preferences/
  storage.begin("led-wall", false);
  storage.getBytes("data", render_buffer, sizeof(render_buffer));
  storage.end();
  renderScreen(render_buffer);

  // wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.printf("STA: Failed!\n");
    WiFi.disconnect(false);
    delay(1000);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }

  // server
  initOTA(server);
  initWebsocketServer(server);
  initWebServer();
}

void loop()
{
  modeLoop();
  cleanUpClients();
}
