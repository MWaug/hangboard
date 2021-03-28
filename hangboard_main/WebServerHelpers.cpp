#include "WebServerHelpers.h"
#include "webpage.h"
#include <WebSocketsServer.h>

ESP8266WebServer web_server;
WebSocketsServer webSocket = WebSocketsServer(81);
StaticJsonDocument<200> webSocketRecvJsonDoc;

void setupWebServer() {
  Serial.println("");
  web_server.begin();
  Serial.println("Server started");
  web_server.on("/", []() { web_server.send_P(200, "text/html", webpage); });
  web_server.begin();
  webSocket.begin();
  Serial.println("Serving webpage on text/html");
}

void tickWebServer() {
  webSocket.loop();
  web_server.handleClient();
}

/*
 * Websocket functions
 */

void webSocketSendString(const char *s, size_t length) {
  webSocket.broadcastTXT(s, length);
}

void webSocketSendData(float w, float ch, float lh, float m, float ave) {
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

int receiveWebsocketString(const char *s) {
  // Root of the object tree.
  //
  // It's a reference to the JsonObject, the actual bytes are inside the
  // JsonBuffer with all the other nodes of the object tree.
  // Memory is freed when jsonDoc goes out of scope.
  Serial.println("received: ");
  Serial.println(s);

  // Test if parsing succeeds.
  auto error = deserializeJson(webSocketRecvJsonDoc, s);
  if (error) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(error.c_str());
    return 1;
  }

  // Fetch values.
  //
  // Most of the time, you can rely on the implicit casts.
  // In other case, you can do root["time"].as<long>();
  return 0;
}