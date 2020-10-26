#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <Ticker.h>
#include "secret.h"
#include "webpage.h"
#include "Q2HX711.h"

// Reference: https://github.com/acrobotic/Ai_Demos_ESP8266/tree/master/bmp180_gui

/* 
 *  Sensor variables
 */
Q2HX711 hx711(D5,D6);
long offset = 7785000; // Scale at zero lbs
long scale_unit_per_lb = 15600;
unsigned long StartHang = 0;
unsigned long EndHang = 0;
long hang_threshold = 15;
float last_hang_secs = -1;
float cur_hang_secs = -1;
float cur_weight = 0;
float defaultDataRate_ms = 50;
float maxWeight = 0;
float integratedWeight = 0;
float aveWeight = 0;
unsigned long lastMeasuredTime = 0;

enum state {
  IN_HANG,
  ON_GROUND
};

enum state cur_state = ON_GROUND;

/* 
 *  Webserver variables
 */
// Connecting to the Internet
const char* ssid = SECRET_SSID; 
const char* password = SECRET_PASSWORD;

// Web trigger for data transmission
bool get_data = false;
Ticker timer;

// Web server
ESP8266WebServer server;

// Adding a websocket to the server
WebSocketsServer webSocket = WebSocketsServer(81);

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

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length){
  // Do something with the data from the client
  if(type == WStype_TEXT){
    float scale_unit_per_lb_float = (float) atof((const char *) &payload[0]);
    scale_unit_per_lb = (long) scale_unit_per_lb_float;
    // timer.detach();
    // timer.attach_ms(dataRate, getData);
  }
}

void getData() {
  get_data = true;
}

void sendData(float w, float ch, float lh, float m, float ave) {
  String json = "{\"value\":";
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
  webSocket.broadcastTXT(json.c_str(), json.length());
}

/* 
 *  Scale update logic
 */
float update_scale() {
  float weight = (offset - hx711.read()) / scale_unit_per_lb;
  if( (cur_state == ON_GROUND) && (weight > hang_threshold) ) {
      StartHang = millis();
      Serial.println("Hanging!");
      cur_state = IN_HANG;
      maxWeight = 0;
      integratedWeight = 0;
      aveWeight = 0;
    }
  // Calculate hang statistics
  if ( cur_state == IN_HANG ) {
    if( weight > maxWeight ) { maxWeight = weight; }
    integratedWeight = integratedWeight + ( weight * (millis() - lastMeasuredTime) );
    aveWeight = integratedWeight / (millis() - StartHang);
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
  lastMeasuredTime = millis();
  return weight;
}

void zero_out_scale(){
   long zero_value = (offset - hx711.read());
   offset = offset - zero_value;
}

void setup() {
  Serial.begin(115200);
  connect_to_internet();
  setup_server();
  zero_out_scale();
  timer.attach_ms(defaultDataRate_ms, getData);
}

void loop() {
  webSocket.loop();
  server.handleClient();
  if(get_data) {
    sendData(cur_weight, cur_hang_secs, last_hang_secs, maxWeight, aveWeight);
    get_data = false;
  }
  cur_weight = update_scale();
}
