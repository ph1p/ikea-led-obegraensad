#include <Arduino.h>
#include <BfButton.h>
#include <SPI.h>

#ifdef ESP8266
/* Fix duplicate defs of HTTP_GET, HTTP_POST, ... in ESPAsyncWebServer.h */
#define WEBSERVER_H
#endif

#include <WiFiManager.h>

#ifdef ESP32
#include <ESPmDNS.h>
#endif
#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif

#include "PluginManager.h"
#include "brightness_schedule.h"
#include "config.h"
#include "scheduler.h"
#include "timing.h"

#include "plugins/ArtNet.h"
#include "plugins/BigPongPlugin.h"
#include "plugins/AutoWalkerPlugin.h"
#include "plugins/Blob.h"
#include "plugins/BouncingBallPlugin.h"
#include "plugins/BreakoutPlugin.h"
#include "plugins/BubblesPlugin.h"
#include "plugins/CheckerboardPlugin.h"
#include "plugins/CirclePlugin.h"
#include "plugins/CometPlugin.h"
#include "plugins/DDPPlugin.h"
#include "plugins/DrawPlugin.h"
#include "plugins/FacePlugin.h"
#include "plugins/FallingSandPlugin.h"
#include "plugins/FirefliesPlugin.h"
#include "plugins/FireworkPlugin.h"
#include "plugins/FlappyBirdPlugin.h"
#include "plugins/GameOfLifePlugin.h"
#include "plugins/LinesPlugin.h"
#include "plugins/MatrixRainPlugin.h"
#include "plugins/MeteorShowerPlugin.h"
#include "plugins/PongClockPlugin.h"
#include "plugins/RadarPlugin.h"
#include "plugins/RainPlugin.h"
#include "plugins/ScanlinesPlugin.h"
#include "plugins/SnakePlugin.h"
#include "plugins/SpaceInvadersPlugin.h"
#include "plugins/SparkleFieldPlugin.h"
#include "plugins/SpotlightPlugin.h"
#include "plugins/SpiralPlugin.h"
#include "plugins/StarsPlugin.h"
#include "plugins/TetrisPlugin.h"
#include "plugins/TickingClockPlugin.h"
#include "plugins/TronPlugin.h"
#include "plugins/WaveBarsPlugin.h"
#include "plugins/WavePlugin.h"

#ifdef ENABLE_SERVER
#include "plugins/AnimationPlugin.h"
#include "plugins/BigClockPlugin.h"
#include "plugins/ClockPlugin.h"
#include "plugins/WeatherPlugin.h"
#endif

#include "asyncwebserver.h"
#include "messages.h"
#include "ota.h"
#include "screen.h"
#include "secrets.h"
#include "websocket.h"

BfButton btn(BfButton::STANDALONE_DIGITAL, PIN_BUTTON, true, LOW);

unsigned long previousMillis = 0;
unsigned long interval = 30000;

PluginManager pluginManager;
#ifdef ESP32
DRAM_ATTR volatile SYSTEM_STATUS currentStatus = NONE;
#else
volatile SYSTEM_STATUS currentStatus = NONE;
#endif
WiFiManager wifiManager;

// SNTP stores the pointer it is given instead of copying the string, so these
// have to outlive the configTzTime() call below.
String ntpServer;
String tzInfo;

unsigned long lastConnectionAttempt = 0;
const unsigned long connectionInterval = 10000;
unsigned long reconnectionBackoff = 5000;            // Start with 5 seconds
const unsigned long maxReconnectionBackoff = 300000; // Max 5 minutes
uint8_t reconnectionAttempts = 0;

// Replaces the DNS servers handed out by DHCP while keeping DHCP for the IP
// itself. A DHCP lease renewal resets them, so this is re-applied from loop().
void applyDnsOverride()
{
#if defined(ESP32) && defined(DNS_OVERRIDE_1)
  if (WiFi.status() != WL_CONNECTED)
  {
    return;
  }

  IPAddress dns1;
  dns1.fromString(DNS_OVERRIDE_1);
  if (WiFi.dnsIP(0) == dns1)
  {
    return;
  }

  WiFi.STA.dnsIP(0, dns1);
#ifdef DNS_OVERRIDE_2
  IPAddress dns2;
  dns2.fromString(DNS_OVERRIDE_2);
  WiFi.STA.dnsIP(1, dns2);
#endif
  Serial.printf("DNS set to %s\n", WiFi.dnsIP(0).toString().c_str());
#endif
}

void connectToWiFi()
{
  // if a WiFi setup AP was started, reboot is required to clear routes
  bool wifiWebServerStarted = false;
  wifiManager.setWebServerCallback([&wifiWebServerStarted]() { wifiWebServerStarted = true; });

  wifiManager.setHostname(WIFI_HOSTNAME);

#if defined(IP_ADDRESS) && defined(GWY) && defined(SUBNET) && defined(DNS1)
  auto ip = IPAddress();
  ip.fromString(IP_ADDRESS);

  auto gwy = IPAddress();
  gwy.fromString(GWY);

  auto subnet = IPAddress();
  subnet.fromString(SUBNET);

  auto dns = IPAddress();
  dns.fromString(DNS1);

  wifiManager.setSTAStaticIPConfig(ip, gwy, subnet, dns);
#endif

  wifiManager.setConnectRetries(10);
  wifiManager.setConnectTimeout(10);
  wifiManager.setConfigPortalTimeout(180);
  wifiManager.setWiFiAutoReconnect(true);
  wifiManager.autoConnect(WIFI_MANAGER_SSID);

#ifdef ESP32
  if (MDNS.begin(WIFI_HOSTNAME))
  {
    MDNS.addService("http", "tcp", 80);
    MDNS.setInstanceName(WIFI_HOSTNAME);
  }
  else
  {
    Serial.println("Could not start mDNS!");
  }
#endif

  if (wifiWebServerStarted)
  {
    // Reboot required, otherwise wifiManager server interferes with our server
    Serial.println("Done running WiFi Manager webserver - rebooting");
    ESP.restart();
  }

  applyDnsOverride();

  lastConnectionAttempt = millis();
}

// Holding the button while powering on for WIFI_RESET_HOLD_MS clears the
// stored WiFi credentials, so the WiFiManager setup portal starts afterwards.
void checkWiFiResetOnBoot()
{
  if (digitalRead(PIN_BUTTON) != LOW)
  {
    return;
  }

  Serial.println("Button held on boot, keep holding to reset WiFi...");
  const unsigned long start = millis();
  while (millis() - start < WIFI_RESET_HOLD_MS)
  {
    if (digitalRead(PIN_BUTTON) != LOW)
    {
      Serial.println("Button released, WiFi reset cancelled");
      return;
    }
    delay(10);
  }

  Serial.println("Resetting WiFi settings");
  wifiManager.resetSettings();
}

void pressHandler(BfButton *btn, BfButton::press_pattern_t pattern)
{
  switch (pattern)
  {
  case BfButton::SINGLE_PRESS:
    if (currentStatus != LOADING)
    {
      Scheduler.clearSchedule();
      pluginManager.activateNextPlugin();
    }
    break;

  case BfButton::LONG_PRESS:
    if (currentStatus != LOADING)
    {
      pluginManager.activatePersistedPlugin();
    }
    break;
  }
}

void baseSetup()
{
  Serial.begin(115200);

  pinMode(PIN_LATCH, OUTPUT);
  pinMode(PIN_CLOCK, OUTPUT);
  pinMode(PIN_DATA, OUTPUT);
  pinMode(PIN_ENABLE, OUTPUT);
  // BfButton sets the pull-up in its global constructor, which runs before
  // the core is initialised and can get lost; apply it again here.
  pinMode(PIN_BUTTON, INPUT_PULLUP);

#ifndef ESP8266
  // Keep the matrix dark until Screen.setup(): the shift registers hold random data
  // after power-on, which would light LEDs at full current during WiFi start-up.
  digitalWrite(PIN_ENABLE, HIGH);
#endif
  digitalWrite(PIN_LATCH, LOW);
  for (int i = 0; i < ROWS * COLS; i++)
  {
    digitalWrite(PIN_DATA, LOW);
    digitalWrite(PIN_CLOCK, HIGH);
    digitalWrite(PIN_CLOCK, LOW);
  }
  digitalWrite(PIN_LATCH, HIGH);

#if !defined(ESP32) && !defined(ESP8266)
  Screen.setup();
#endif

  // Initialize configuration system (always safe)
  config.begin();

// server
#ifdef ENABLE_SERVER
  checkWiFiResetOnBoot();
  connectToWiFi();

  // set time server using config values
  // NOTE: lwIP SNTP stores the server-name pointer without copying it, so the
  // String must outlive this call. Keep them static so their buffers persist.
  static String tzInfo = config.getTzInfo();
  static String ntpServer = config.getNtpServer();
  configTzTime(tzInfo.c_str(), ntpServer.c_str());

  initOTA(server);
  initWebsocketServer(server);
  initWebServer();
#endif

  pluginManager.addPlugin(new DrawPlugin());
  pluginManager.addPlugin(new BreakoutPlugin());
  pluginManager.addPlugin(new SnakePlugin());
  pluginManager.addPlugin(new GameOfLifePlugin());
  pluginManager.addPlugin(new StarsPlugin());
  pluginManager.addPlugin(new LinesPlugin());
  pluginManager.addPlugin(new CirclePlugin());
  pluginManager.addPlugin(new RainPlugin());
  pluginManager.addPlugin(new MatrixRainPlugin());
  pluginManager.addPlugin(new FireworkPlugin());
  pluginManager.addPlugin(new BlobPlugin());
  pluginManager.addPlugin(new SpiralPlugin());
  pluginManager.addPlugin(new WavePlugin());
  pluginManager.addPlugin(new CheckerboardPlugin());
  pluginManager.addPlugin(new RadarPlugin());
  pluginManager.addPlugin(new BubblesPlugin());
  pluginManager.addPlugin(new CometPlugin());
  pluginManager.addPlugin(new FirefliesPlugin());
  pluginManager.addPlugin(new MeteorShowerPlugin());
  pluginManager.addPlugin(new ScanlinesPlugin());
  pluginManager.addPlugin(new SparkleFieldPlugin());
  pluginManager.addPlugin(new WaveBarsPlugin());
  pluginManager.addPlugin(new BigPongPlugin());
  pluginManager.addPlugin(new AutoWalkerPlugin());
  pluginManager.addPlugin(new BouncingBallPlugin());
  pluginManager.addPlugin(new FacePlugin());
  pluginManager.addPlugin(new FallingSandPlugin());
  pluginManager.addPlugin(new FlappyBirdPlugin());
  pluginManager.addPlugin(new SpaceInvadersPlugin());
  pluginManager.addPlugin(new SpotlightPlugin());
  pluginManager.addPlugin(new TetrisPlugin());
  pluginManager.addPlugin(new TronPlugin());

#ifdef ENABLE_SERVER
  pluginManager.addPlugin(new BigClockPlugin());
  pluginManager.addPlugin(new ClockPlugin());
  pluginManager.addPlugin(new PongClockPlugin());
  pluginManager.addPlugin(new TickingClockPlugin());
  pluginManager.addPlugin(new WeatherPlugin());
  pluginManager.addPlugin(new AnimationPlugin());
  pluginManager.addPlugin(new DDPPlugin());
  pluginManager.addPlugin(new ArtNetPlugin());
#endif

  Screen.clear();
  pluginManager.init();
  Scheduler.init();

  btn.onPress(pressHandler).onDoublePress(pressHandler).onPressFor(pressHandler, 1000);
}

// everything that draws or switches plugins, run from the same task as the
// active plugin so two tasks never write the screen buffer at the same time
void runScreenJobs()
{
  static uint8_t jobCounter = 0;

  if (currentStatus == NONE)
  {
    Scheduler.update();
    ScheduledBrightness.update();

    if ((jobCounter++ & 0x03) == 0)
    {
      Messages.scrollMessageEveryMinute();
    }
  }
}

#ifdef ESP32
TaskHandle_t screenDrawingTaskHandle = NULL;

void screenDrawingTask(void *parameter)
{
  ScheduledBrightness.init();
  for (;;)
  {
    runScreenJobs();
    pluginManager.runActivePlugin();
    vTaskDelay(1);
  }
}

TaskHandle_t buttonTaskHandle = NULL;

// Polls the button in its own task so it keeps working while loop() is
// blocked, e.g. by WiFiManager (re)connecting or its config portal.
void buttonTask(void *parameter)
{
  for (;;)
  {
    btn.read();
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

void setup()
{
  // started before baseSetup() so it runs during the blocking WiFi connect;
  // press callbacks are only registered at the end of baseSetup()
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  xTaskCreatePinnedToCore(buttonTask,
                          "buttonTask",
                          4096,
                          NULL,
                          1,
                          &buttonTaskHandle,
                          1);
  baseSetup();
  // the timer interrupt is bound to the core it is created on, keep it on
  // core 1 so WiFi interrupts on core 0 do not delay it and jitter the PWM
  Screen.setup();
  // from here on plugin switches and websocket hooks are handed to the screen task
  pluginManager.enableScreenTask();
  xTaskCreatePinnedToCore(screenDrawingTask,
                          "screenDrawingTask",
                          10000,
                          NULL,
                          1,
                          &screenDrawingTaskHandle,
                          0);
}
#endif
#ifdef ESP8266
void setup()
{
  baseSetup();
  // nothing called the old screenDrawingTask(), so the screen never started
  Screen.setup();
  ScheduledBrightness.init();
  Scheduler.start();
}
#endif

void loop()
{
#ifndef ESP32
  btn.read();
#endif

#ifdef ENABLE_SERVER
  ElegantOTA.loop();
#endif

#ifndef ESP32
  runScreenJobs();
  pluginManager.runActivePlugin();
#endif

  // Check WiFi less frequently with exponential backoff
  if (WiFi.status() != WL_CONNECTED)
  {
    unsigned long currentMillis = millis();
    if (currentMillis - lastConnectionAttempt >= reconnectionBackoff)
    {
      Serial.println("WiFi disconnected, attempting reconnection...");
      connectToWiFi();

      // Exponential backoff: double the wait time, up to max
      reconnectionAttempts++;
      reconnectionBackoff = min(reconnectionBackoff * 2, maxReconnectionBackoff);
    }
  }
  else
  {
    if (reconnectionAttempts > 0)
    {
      Serial.println("WiFi reconnected successfully");
      reconnectionAttempts = 0;
      reconnectionBackoff = 5000;
    }
    // a DHCP renewal can reset the DNS servers, a check every few seconds is enough
    static NonBlockingDelay dnsTimer;
    if (dnsTimer.isReady(5000))
    {
      applyDnsOverride();
    }
  }

#ifdef ENABLE_SERVER
  static unsigned long lastLiveFrameMillis = 0;
  const unsigned long currentMillis = millis();
  if (currentMillis - lastLiveFrameMillis >= 100)
  {
    lastLiveFrameMillis = currentMillis;
    sendLiveFrame();
  }

  static NonBlockingDelay cleanupTimer;
  if (cleanupTimer.isReady(1000))
  {
    cleanUpClients();
  }
#endif
#ifdef ESP32
  vTaskDelay(1);
#else
  delay(1);
#endif
}
