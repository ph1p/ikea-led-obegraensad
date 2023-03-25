#include <PubSubClient.h>
#include <Wifi.h>
#include "secrets.h"
#include "mode/mode.h"
#include "mqtt.h"

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
    MODE modeAsEnum = getModeByString(message);
    setMode(modeAsEnum);
  }
  Serial.print(message);
  Serial.println();

}

void mqttSetup(){
    MQTTclient.setServer(MQTTSERVERIP, 1883);
    MQTTclient.setCallback(mqttCallback);
}

void mqttReconnect() {
  // Loop until we're reconnected
  while (!MQTTclient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (MQTTclient.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //MQTTclient.publish("LEDDisplay/mode", "hello world");
      // ... and resubscribe
      MQTTclient.subscribe("LEDDisplay/mode");
    } else {
      Serial.print("failed, rc=");
      Serial.print(MQTTclient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

