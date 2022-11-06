#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <cstdlib>

#include "ws.h"
#include "webgui.h"
#include "led.h"
#include "constants.h"
#include "storage.h"
#include "mode/mode.h"
#include "secrets.h"
#include "ota.h"

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;

AsyncWebServer server(80);

void initWebServer()
{
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Accept, Content-Type, Authorization");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Credentials", "true");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");

  server.on("/", HTTP_GET, startGui);
  server.onNotFound([](AsyncWebServerRequest *request)
                    { request->send(404, "text/plain", "Page not found!"); });

  server.begin();
}

void setup()
{
  Serial.begin(115200);

  pinMode(PIN_LATCH, OUTPUT);
  pinMode(PIN_CLOCK, OUTPUT);
  pinMode(PIN_DATA, OUTPUT);
  pinMode(PIN_ENABLE, OUTPUT);

  clearScreenAndBuffer(render_buffer);

  // https://randomnerdtutorials.com/esp32-save-data-permanently-preferences/
  storage.begin("led-wall", false);
  storage.getBytes("data", render_buffer, sizeof(render_buffer));
  storage.end();
  renderScreen(render_buffer);
  initModes();

  // wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.printf("STA: Failed!\n");
    WiFi.disconnect(false);
    delay(1000);
    WiFi.begin(ssid, password);
  }

  // server
  initOTA(server);
  initWebsocketServer(server);
  initWebServer();
}

void loop()
{
  if (modeThread.shouldRun())
    modeThread.run();

  cleanUpClients();
}
