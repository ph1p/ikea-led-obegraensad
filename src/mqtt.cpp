#include <PubSubClient.h>
#include "secrets.h"
#include "PluginManager.h"
#include "mqtt.h"
#include "plugins/WeatherPlugin.h"

#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif
#ifdef ESP32
#endif

WiFiClient espClient;
PubSubClient MQTTclient(espClient);

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String message;
  for (int i = 0; i < length; i++) {
    //Serial.print((char)payload[i]);
    message += String((char)payload[i]);
  }
  if (topic == "Obegraensad/mode")
  {
    pluginManager.setActivePluginById(message.toInt());
  } else {
    pluginManager.getActivePlugin()->mqttHook(topic, message.toInt());
  }
  Serial.print(message);
  Serial.println();
}

void mqttSetup(){
    MQTTclient.setServer(MQTTSERVERIP, 1883);
    MQTTclient.setCallback(mqttCallback);
    mqttReconnect();
}

void mqttReconnect() {
  // Loop until we're reconnected
  while (!MQTTclient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESPClient-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
#ifdef MQTT_AUTH
    if (MQTTclient.connect(clientId.c_str(),MQTT_USER,MQTT_PASS)) {
#else
    if (MQTTclient.connect(clientId.c_str())) {
#endif
      Serial.println("connected");
      MQTTclient.subscribe("Obegraensad/mode");
      MQTTclient.subscribe("Obegraensad/weatherMode");
      MQTTclient.subscribe("Obegraensad/weatherTime");
    } else {
      Serial.print("failed, rc=");
      Serial.print(MQTTclient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

