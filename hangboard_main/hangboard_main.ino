#include "Hangboard.h"
#include "WebServerHelpers.h"
#include <Arduino.h>
#include <Ticker.h>
#include <WebSocketsServer.h>

Hangboard hangboard =
    Hangboard(HX711_CLK_PIN, HX711_DOUT_PIN, HANG_THRESHOLD_DEFAULT_LB);

void setup() { hangboard.setup(); }

void loop() { hangboard.loop(); }
