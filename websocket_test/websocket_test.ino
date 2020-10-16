#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <Ticker.h>
#include "secret.h"
#include "webpage.h"
#include "Q2HX711.h"

// Reference: https://github.com/acrobotic/Ai_Demos_ESP8266/tree/master/bmp180_gui

// Sensor variables and functions
Q2HX711 hx711(4,5);
long offset = 7784000; // Scale at zero lbs
long calib_180lbs = 4500000;
long scale_unit_per_lb = 13500;
unsigned long StartHang = 0;
unsigned long EndHang = 0;
long hang_threshold = 64;
float last_hang_secs = 0;
float cur_hang_secs = 0;

enum state {
  IN_HANG,
  ON_GROUND
};

enum state cur_state = ON_GROUND;

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
  get_data = true;
}

void setup() {
  Serial.begin(115200);
  connect_to_internet();
  setup_server();
  timer.attach_ms(500, getData);
}

void loop() {
  // Websocket 
  webSocket.loop();
  server.handleClient();
  if(get_data) {
    String json = "{\"value\":";
    json += "123";
//    json += ",\"lh\":";
//    json += String(cur_hang_secs, 2);
//    json += ",\"ch\":";
//    json += String(last_hang_secs, 2);
    json += "}";
    webSocket.broadcastTXT(json.c_str(), json.length());
    get_data = false;
  }
  // Digital scale
  long weight = (offset - hx711.read()) / scale_unit_per_lb;
  if( (cur_state == ON_GROUND) && (weight > hang_threshold) ) {
      StartHang = millis();
      Serial.println("Hanging!");
      cur_state = IN_HANG;
      cur_hang_secs = (millis() - StartHang) / 1000.0;
    }
    if( (cur_state == IN_HANG) && weight < hang_threshold - 5) {
      EndHang = millis();
      unsigned long hang_dur = EndHang - StartHang;
      float hang_dur_secs = hang_dur / 1000.0;
      Serial.println("On the ground");
      Serial.println(hang_dur_secs);
      last_hang_secs = hang_dur_secs;
      cur_hang_secs = 0;
      cur_state = ON_GROUND;
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
