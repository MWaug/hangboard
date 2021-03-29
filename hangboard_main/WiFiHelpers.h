#ifndef WiFiHelpers_h
#define WiFiHelpers_h

#include "secret.h"
#include <ESP8266WiFi.h>

/*
 *  Webserver variables
 */
// Connecting to the Internet
const char *ssid = SECRET_SSID;
const char *password = SECRET_PASSWORD;

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

#endif