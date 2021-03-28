#ifndef MQTTHelpers_h
#define MQTTHelpers_h

#include "ArduinoJson.h"
#include "secret.h"
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>

// MQTT PubSub Client
WiFiClient espClient;
PubSubClient mqtt_client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (100)
char msg[MSG_BUFFER_SIZE];
const int MQTT_RECONNECT_DELAY_MS = 5000;
const int mqtt_port = 1883;

// Memory pool for JSON object sent over the WebSocket.
//
// Inside the brackets, 200 is the size of the pool in bytes,
// If the JSON object is more complex, you need to increase that value.
// See https://bblanchon.github.io/ArduinoJson/assistant/
StaticJsonDocument<200> jsonDoc;

/*
 * MQTT Functions
 */
void mqtt_reconnect() {
  // Loop until mqtt is reconnected
  while (!mqtt_client.connected()) {
    Serial.println("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(WiFi.macAddress());
    // Attempt to connect
    if (mqtt_client.connect(clientId.c_str(), SECRET_MQTT_USERNAME,
                            SECRET_MQTT_PASSWORD)) {
      Serial.println("mqtt_client connected");
      // Once connected, publish an announcement
      mqtt_client.publish("connections", "connected");
      Serial.println("Subscribing to `updates`");
      mqtt_client.subscribe("updates");
    } else {
      Serial.print("Falied, rc=");
      Serial.print(mqtt_client.state());
      printf(" mqtt_connect will try again in %f seconds\n",
             MQTT_RECONNECT_DELAY_MS / 1000.0);
      // Wait before retrying
      delay(MQTT_RECONNECT_DELAY_MS);
    }
  }
}

void tickMQTTClient() {
  if (!mqtt_client.connected()) {
    mqtt_reconnect();
  }
  mqtt_client.loop();
}

void mqtt_callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (size_t i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void setupMQTT() {
  mqtt_client.setServer(SECRET_MQTT_SERVER_IP, mqtt_port);
  mqtt_client.setCallback(mqtt_callback);
}

#endif