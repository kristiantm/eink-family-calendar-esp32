//Working sketch for Arduino Nano 33 IoT to pull data from iot-calendar-server
//(done while waiting for my ESP32 to turn up in the mail!)

//sources:
//wifinina stuff: https://github.com/clockspot/arduino-ledclock
//ntp and ica: https://github.com/sourcesimian/uICAL/blob/master/examples/SimpleESP8266/SimpleESP8266.ino
//nws/json: https://arduinojson.org/v6/example/http-client/

#include <SPI.h>
#include <WiFiNINA.h>
#include <ArduinoJson.h> //bblanchon

#include "config.h"

WiFiSSLClient sslClient;

void setup() {
  Serial.begin(9600);
  while(!Serial); //only works on 33 IoT
  delay(1000);

  //Check status of wifi module up front
  if(WiFi.status()==WL_NO_MODULE){ Serial.println(F("Communication with WiFi module failed!")); return; }
  else if(WiFi.firmwareVersion()<WIFI_FIRMWARE_LATEST_VERSION) Serial.println(F("Please upgrade the firmware"));

  //Start wifi
  Serial.print(F("\nConnecting to WiFi SSID "));
  Serial.println(NETWORK_SSID);
  WiFi.begin(NETWORK_SSID, NETWORK_PASS); //WPA - hangs while connecting
  if(WiFi.status()==WL_CONNECTED){ //did it work?
    //Serial.print(millis());
    Serial.println(F("Connected!"));
    //Serial.print(F("SSID: ")); Serial.println(WiFi.SSID());
    Serial.print(F("Signal strength (RSSI): ")); Serial.print(WiFi.RSSI()); Serial.println(F(" dBm"));
    Serial.print(F("Local IP: ")); Serial.println(WiFi.localIP());
  } else {
    Serial.println(F("Wasn't able to connect."));
    return;
  }

  //Connect to data host
  Serial.print(F("\nConnecting to data host "));
  Serial.println(DATA_HOST);
  sslClient.setTimeout(15000);
  if(!sslClient.connect(DATA_HOST, 443)) {
    Serial.println(F("Wasn't able to connect to host."));
    return;
  }

  Serial.println("Connected!");

  //Make an HTTP request
  Serial.print(F("GET "));
  Serial.print(DATA_PATH);
  Serial.println(F(" HTTP/1.1"));
  sslClient.print(F("GET "));
  sslClient.print(DATA_PATH);
  sslClient.println(F(" HTTP/1.1"));

  sslClient.print(F("Host: "));
  sslClient.println(DATA_HOST);

  sslClient.println(F("Connection: close"));

  //the rest adapted from https://arduinojson.org/v6/example/http-sslClient/
  if (sslClient.println() == 0) {
    Serial.println(F("Failed to send request"));
    sslClient.stop();
    return;
  }

  // Check HTTP status
  char status[32] = {0};
  sslClient.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    sslClient.stop();
    return;
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!sslClient.find(endOfHeaders)) {
    Serial.println(F("Invalid response"));
    sslClient.stop();
    return;
  }

  // Allocate the JSON document
  // Use arduinojson.org/v6/assistant to compute the capacity.
  DynamicJsonDocument doc(12288);
  
  // Parse JSON object
  DeserializationError error = deserializeJson(doc, sslClient);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    sslClient.stop();
    return;
  }

  Serial.println();

  // Extract values - expecting an array of days (or empty array on server error)
  // cf. "Pretend display" in iot-calendar-server/docroot/index.php
  for (JsonObject day : doc.as<JsonArray>()) {
    //header
    Serial.print(day["weekdayRelative"].as<char*>());
    if(day["weekdayRelative"]=="Today") {
      Serial.print(F(" - "));
      Serial.print(day["weekdayShort"].as<char*>());
      Serial.print(F(" "));
      Serial.print(day["date"].as<char*>());
      Serial.print(F(" "));
      Serial.print(day["monthShort"].as<char*>());
      Serial.println();
      Serial.print(F("Sunrise "));
      Serial.print(day["sun"]["sunrise"].as<char*>());
      Serial.print(F("  Sunset "));
      Serial.print(day["sun"]["sunset"].as<char*>());
    }
    Serial.println();
    //weather
    for (JsonObject w : day["weather"].as<JsonArray>()) {
      if(w["isDaytime"]) Serial.print(F("High "));
      else Serial.print(F("Low "));
      Serial.print(w["temperature"].as<int>());
      Serial.print(F("º "));
      Serial.print(w["shortForecast"].as<char*>());
      Serial.println();
    }
    // events
    for (JsonObject e : day["events"].as<JsonArray>()) {
      if(e["style"]=="red") Serial.print(F(" ▪ "));
      else Serial.print(F(" • "));
      if(!e["allday"]) { Serial.print(e["timestart"].as<char*>()); Serial.print(F(" ")); }
      Serial.print(e["summary"].as<char*>());
      if(e["allday"] && e["dend"]!=e["dstart"]) { Serial.print(F(" (thru ")); Serial.print(e["dendShort"].as<char*>()); Serial.print(F(")")); }
      Serial.println();
    }
    //end of day
    Serial.println();
  }

  // Disconnect
  sslClient.stop();

  Serial.println(F("Done!"));
  
} //end fn setup

void loop() {
  // To inspect the HTTP response directly, uncomment this code, and in setup(), comment out "Check HTTP Status" and everything afterward

  // // if there are incoming bytes available
  // // from the server, read them and print them:
  // while (sslClient.available()) {
  //   char c = sslClient.read();
  //   Serial.write(c);
  // }

  // // if the server's disconnected, stop the client:
  // if (!sslClient.connected()) {
  //   Serial.println();
  //   Serial.println("disconnecting from server.");
  //   sslClient.stop();

  //   // do nothing forevermore:
  //   while (true);
  // }
}