#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "secret.h"
#include "webpage.h"
#include "ArduinoJson.h"
#include <PubSubClient.h>

/* 
 *  Webserver variables
 */
// Connecting to the Internet
const char* ssid = SECRET_SSID; 
const char* password = SECRET_PASSWORD;
const char* mqtt_server = SECRET_MQTT_SERVER_IP;
const char* mqtt_username = SECRET_MQTT_USERNAME;
const char* mqtt_password = SECRET_MQTT_PASSWORD;
const int mqtt_port = 1883;

// Web server
ESP8266WebServer server;

// Adding a websocket to the server
WebSocketsServer webSocket = WebSocketsServer(81);

// MQTT PubSub Client
WiFiClient espClient;
PubSubClient mqtt_client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (100)
char msg[MSG_BUFFER_SIZE];
const int MQTT_RECONNECT_DELAY_MS = 5000;

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
    if(mqtt_client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("mqtt_client connected");
      // Once connected, publish an announcement
      mqtt_client.publish("connections", "connected");
      Serial.println("Subscribing to `updates`");
      mqtt_client.subscribe("updates");
    } else {
      Serial.print("Falied, rc=");
      Serial.print(mqtt_client.state());
      printf(" mqtt_connect will try again in %f seconds\n", MQTT_RECONNECT_DELAY_MS/1000.0);
      // Wait before retrying
      delay(MQTT_RECONNECT_DELAY_MS);
    }
  }
}

void mqtt_tick_client() {
  if(!mqtt_client.connected()) {
    mqtt_reconnect();
  }
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (size_t i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void mqtt_setup() {
  mqtt_client.setServer(mqtt_server, mqtt_port);
  mqtt_client.setCallback(mqtt_callback);
}

/*
 * OTA functions
 */
void OTA_setup() {
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword("admin"_mqttcallback_mqtt);

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
}

/* 
 *  Internet Connectivity
 */
void connect_to_internet() {
  Serial.print("Connecting to ");
  Serial.println(ssid); 
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println("");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void setup_servers( void (*webSocketEventHandler)(uint8_t, WStype_t, uint8_t *, size_t) ) {
  OTA_setup();
  mqtt_setup();
  Serial.println("");
  server.begin();
  Serial.println("Server started");
  server.on("/",[](){
    server.send_P(200, "text/html", webpage);
  });
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEventHandler);
  Serial.println("Serving webpage on text/html");
}

void tickServer() {
  mqtt_tick_client();
  ArduinoOTA.handle();
  webSocket.loop();
  server.handleClient();
}

/*
 * Websocket functions
 */

void webSocketSendString(const char * s, size_t length) {
  webSocket.broadcastTXT(s, length); 
}

void sendData(float w, float ch, float lh, float m, float ave) {
  String json = "{\"w\":";
    json += String(w, 2);
    json += ",\"ch\":";
    json += String(ch, 2);
    json += ",\"lh\":";
    json += String(lh, 2);
    json += ",\"max\":";
    json += String(m, 2);
    json += ",\"ave\":";
    json += String(ave, 2);
    json += "}";
  webSocketSendString(json.c_str(), json.length());
}

int receiveWebsocketString(const char * s) {
  // Root of the object tree.
  //
  // It's a reference to the JsonObject, the actual bytes are inside the
  // JsonBuffer with all the other nodes of the object tree.
  // Memory is freed when jsonDoc goes out of scope.
  Serial.println("received: "); Serial.println(s);

  // Test if parsing succeeds.
  auto error = deserializeJson(jsonDoc, s);
  if (error) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(error.c_str());
    return 1;
  }

  // Fetch values.
  //
  // Most of the time, you can rely on the implicit casts.
  // In other case, you can do root["time"].as<long>();
  person = jsonDoc["person"];
  calib = jsonDoc["calib"];
  rung = jsonDoc["rung"];
  note = jsonDoc["note"];
  dataLogEnable = jsonDoc["dataLogEnable"];
  return 0;
}
