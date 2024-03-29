#ifndef Hangboard_h
#define Hangboard_h

#include "Arduino.h"
#include "HangboardFSM.h"
#include "HangboardScale.h"
#include "Q2HX711.h"
#include <Ticker.h>
#include <WebSocketsServer.h>

class Hangboard {
private:
  HangboardFSM _fsm;
  HangboardScale _scale;
  bool _do_socket_send;
  Ticker _socket_send_timer;
  void flagWebsocketSend();
  void handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload,
                            size_t length);
  void handleMQTT(char *topic, byte *payload, unsigned int length);
  void sendStartHangEvent();
  void sendFinishHangEvent();
  void sendData();

public:
  Hangboard(uint8_t hx711_clk_pin, uint8_t hx711_dout_pin);
  Hangboard(uint8_t hx711_clk_pin, uint8_t hx711_dout_pin,
            uint32_t hang_threshold_lbs);
  ~Hangboard();
  void zeroOutScale();
  void updateScaleCalibration(long c);
  float readWeight();
  void setup();
  void loop();
};

#endif