#include <WebSocketsServer.h>
#include <Ticker.h>

// Reference: https://github.com/acrobotic/Ai_Demos_ESP8266/tree/master/bmp180_gui
// Hangboard data
unsigned long StartHang = 0;
unsigned long EndHang = 0;
long hang_threshold = 15;
float last_hang_secs = 0;
float cur_hang_secs = 0;
float cur_weight = 0;
float defaultDataRate_ms = 50; // Todo: Change this value
float maxWeight = 0;
float integratedWeight = 0;
float aveWeight = 0;

// Application data sent from webpage
const char* person = "Max";
long calib = 21100;
const char* rung = "middle-edge";
const char* note = "";
int dataLogEnable = 1;

// Testing mode
bool test_mode = true; // Disable HX711 interaction. Print some extra statements

// Web trigger for data transmission
bool do_socket_send = false;
Ticker timer;

// Useful JSON parsing example: https://wandbox.org/permlink/ZgwgYgfuAUXTPFUs
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length){
  // Do something with the data from the client
  if(type == WStype_TEXT){
    // float scale_unit_per_lb_float = (float) atof((const char *) &payload[0]);
    receiveWebsocketString( (const char *) &payload[0]);
    update_scale_calibration((long) calib);
  }
}

void flagWebsocketSend() {
  do_socket_send = true;
}

void setup() {
  Serial.begin(115200);
  connect_to_internet();
  Serial.println("Connected to the internet");
  setup_servers(webSocketEvent);
  Serial.println("Setup the server");
  zero_out_scale();
  Serial.println("Zeroed out the scale");
  timer.attach_ms(defaultDataRate_ms, flagWebsocketSend);
  Serial.println("Done with setup");
  if(setup_sheets_connection() != 0) {
    Serial.println("Connection to google sheets failed");
  }
}

void loop() {
  tickServer();
  tick_hangboard();
  if(do_socket_send) {
    sendData(cur_weight, cur_hang_secs, last_hang_secs, maxWeight, aveWeight);
    do_socket_send = false;
  }
}
