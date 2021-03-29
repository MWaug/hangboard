#ifndef MQTTHelpers_h
#define MQTTHelpers_h

#include "ArduinoJson.h"
#include "HangboardDefines.h"
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
StaticJsonDocument<MQTT_JSON_SIZE_BYTES> mqttRecvJsonDoc;

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

void setupMQTT() { mqtt_client.setServer(SECRET_MQTT_SERVER_IP, mqtt_port); }

void mqttPublish(const char *topic, const char *payload) {
  if (!mqtt_client.publish(topic, payload)) {
    printf("mqtt failed to publish %s to topic %s\n", payload, topic);
    printf("mqtt_client.state() = %d\n", mqtt_client.state());
  }
}

void mqttSendFinishHangEvent(float max_weight, float ave_weight,
                             unsigned long start_hang_ms,
                             unsigned long end_hang_ms,
                             unsigned long cur_time_ms, const char *device_id,
                             const char *topic) {
  String json = "{\"max_weight\":";
  json += String(max_weight, 2);
  json += ",\"ave_weight\":";
  json += String(ave_weight, 2);
  json += ",\"start_hang_ms\":";
  json += String(start_hang_ms);
  json += ",\"end_hang_ms\":";
  json += String(end_hang_ms);
  json += ",\"cur_time_ms\":";
  json += String(cur_time_ms);
  json += ",\"device_id\":";
  json += String(device_id);
  json += "}";
  mqttPublish(topic, json.c_str());
}

void mqttSendStartHangEvent(unsigned long start_hang_ms, const char *device_id,
                            const char *topic) {
  String json = "{\"start_hang_ms\":";
  json += String(start_hang_ms);
  json += ",\"device_id\":";
  json += String(device_id);
  json += "}";
  mqttPublish(topic, json.c_str());
}

void mqttSendWeight(float weight, long cur_time_ms, const char *device_id,
                    const char *topic) {
  String json = "{\"weight\":";
  json += String(weight, 2);
  json += ",\"cur_time_ms\":";
  json += String(cur_time_ms);
  json += ",\"device_id\":";
  json += String(device_id);
  json += "}";
  mqttPublish(topic, json.c_str());
}

#endif