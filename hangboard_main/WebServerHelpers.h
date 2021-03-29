#ifndef WebServerHelpers_h
#define WebServerHelpers_h

#include "ArduinoJson.h"
#include "HangboardDefines.h"
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <WiFiUdp.h>

/*
 *  Webserver variables
 */

// Web server
extern ESP8266WebServer web_server;

// Adding a websocket to the server
extern WebSocketsServer webSocket;

// Memory pool for JSON object sent over the WebSocket.
//
// Inside the brackets, 200 is the size of the pool in bytes,
// If the JSON object is more complex, you need to increase that value.
// See https://bblanchon.github.io/ArduinoJson/assistant/
extern StaticJsonDocument<WEBSOCKET_JSON_SIZE_BYTES> webSocketRecvJsonDoc;

void setupWebServer();
void tickWebServer();
void webSocketSendString(const char *s, size_t length);
void webSocketSendData(float w, float ch, float lh, float m, float ave);
int receiveWebsocketString(const char *s);

#endif