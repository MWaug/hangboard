#include "Hangboard.h"
#include "MQTTHelpers.h"
#include "OTAHelpers.h"
#include "WebServerHelpers.h"
#include "WiFiHelpers.h"

Hangboard::Hangboard(uint8_t hx711_clk_pin, uint8_t hx711_dout_pin)
    : _fsm(HANG_THRESHOLD_DEFAULT_LB), _scale(hx711_dout_pin, hx711_clk_pin),
      _do_socket_send(false) {}

Hangboard::Hangboard(uint8_t hx711_clk_pin, uint8_t hx711_dout_pin,
                     uint32_t hang_threshold_lbs)
    : _fsm(hang_threshold_lbs), _scale(hx711_dout_pin, hx711_clk_pin),
      _do_socket_send(false) {}

Hangboard::~Hangboard() {}

void Hangboard::updateScaleCalibration(long c) {
  _scale.updateScaleCalibration(c);
}

void Hangboard::handleWebSocketEvent(uint8_t num, WStype_t type,
                                     uint8_t *payload, size_t length) {
  // Do something with the data from the client
  if (type == WStype_TEXT) {
    // float scale_unit_per_lb_float = (float) atof((const char *) &payload[0]);
    if (receiveWebsocketString((const char *)&payload[0])) {
      if (webSocketRecvJsonDoc["calib"]) {
        updateScaleCalibration((long)webSocketRecvJsonDoc["calib"]);
      }
    };
  }
}

void Hangboard::zeroOutScale() { _scale.zeroOutScale(); }

float Hangboard::readWeight() { return _scale.readWeight(); }

void Hangboard::flagWebsocketSend() { _do_socket_send = true; }

void Hangboard::sendStartHangEvent() {
  mqttSendStartHangEvent(_fsm._StartHang, WiFi.macAddress().c_str(),
                         "start_hang_event");
}

void Hangboard::sendFinishHangEvent() {
  mqttSendFinishHangEvent(_fsm._maxWeight, _fsm._aveWeight, _fsm._StartHang,
                          _fsm._EndHang, millis(), WiFi.macAddress().c_str(),
                          "finish_hang_event");
}

void Hangboard::sendData() {
  // Collect data from the hangboard FSM
  webSocketSendData(_fsm._cur_weight, _fsm._cur_hang_secs, _fsm._last_hang_secs,
                    _fsm._maxWeight, _fsm._aveWeight);
  mqttSendWeight(_fsm._cur_weight, millis(), WiFi.macAddress().c_str(),
                 "weight");
}

void Hangboard::handleMQTT(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (size_t i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void Hangboard::setup() {
  Serial.begin(UART_FREQ_HZ);
  connect_to_internet();
  Serial.println("Connected to the internet");
  setupOTA();
  Serial.println("Setup OTA programming");
  setupWebServer();
  webSocket.onEvent(std::bind(&Hangboard::handleWebSocketEvent, this,
                              std::placeholders::_1, std::placeholders::_2,
                              std::placeholders::_3, std::placeholders::_4));
  Serial.println("Setup web server");
  setupMQTT();
  mqtt_client.setCallback(
      std::bind(&Hangboard::handleMQTT, this, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3));
  Serial.println("Setup MQTT client");
  zeroOutScale();
  Serial.println("Zeroed out the scale");
  _socket_send_timer.attach_ms(WEBSOCKET_SEND_INTERVAL_MS,
                               [this]() -> void { flagWebsocketSend(); });
  Serial.println("Done with setup");
}

void Hangboard::loop() {
  _fsm.tick(_scale.readWeight(), millis());
  if (_fsm.finishedHang()) {
    sendFinishHangEvent();
  }
  if (_fsm.startedHang()) {
    sendStartHangEvent();
  }
  tickMQTTClient();
  tickWebServer();
  if (_do_socket_send) {
    sendData();
    _do_socket_send = false;
  }
};
