#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <Ticker.h>
#include "secret.h"
#include "webpage.h"

// Collecting Weight data
Ticker timer;
bool get_data = false;

// Connecting to the Internet
const char* ssid = SECRET_SSID; 
const char* password = SECRET_PASSWORD;

// Web server
ESP8266WebServer server;

// Adding a websocket to the server
WebSocketsServer webSocket = WebSocketsServer(81);

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

void setup_server() {
  Serial.println("");
  server.begin();
  Serial.println("Server started");
  server.on("/",[](){
    server.send_P(200, "text/html", webpage);
  });
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("Serving webpage on text/html");
}

void getData() {
  Serial.println("Getting Data");
  get_data = true;
}

void setup() {
  Serial.begin(115200);
  connect_to_internet();
  setup_server();
  timer.attach_ms(500, getData);
}

void loop() {
  webSocket.loop();
  server.handleClient();
  if(get_data) {
    String json = "{\"valud\":";
    json += "123";
    json += "}";
    webSocket.broadcastTXT(json.c_str(), json.length());
    get_data = false;
  }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length){
  // Do something with the data from the client
  if(type == WStype_TEXT){
    float dataRate = (float) atof((const char *) &payload[0]);
    timer.detach();
    timer.attach_ms(dataRate, getData);
  }
}
