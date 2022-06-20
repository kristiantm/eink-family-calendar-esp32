//Working sketch for Arduino Nano 33 IoT to pull and process data
//(done while waiting for my ESP32 to turn up in the mail!)

//sources:
//wifinina stuff: https://github.com/clockspot/arduino-ledclock
//ntp and ica: https://github.com/sourcesimian/uICAL/blob/master/examples/SimpleESP8266/SimpleESP8266.ino
//nws/json: https://arduinojson.org/v6/example/http-client/

#include <SPI.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h> //for NTP
#include <EasyNTPClient.h> //aharshac - for NTP
#include <ArduinoJson.h> //bblanchon - for parsing NWS/calendar JSON data
//#include <uICAL.h> //sourcesimian - for parsing ICA data

#include "config.h"

WiFiUDP udp; //for NTP
EasyNTPClient ntpClient(udp, NTP_HOST, NTP_OFFSET);
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

  //Get current time from NTP
  Serial.print(F("\nConnecting to NTP host "));
  Serial.println(NTP_HOST);
  udp.begin(NTP_PORT);
  while(!ntpClient.getUnixTime()) delay(500); //TODO better blocker?
  Serial.print(F("Unix time is now "));
  Serial.println(ntpClient.getUnixTime(),DEC);

  // Allocate the JSON document
  // Use arduinojson.org/v6/assistant to compute the capacity.
  DynamicJsonDocument doc(12288);

  //Get weather JSON data from NWS
  Serial.print(F("\nConnecting to NWS host "));
  Serial.println(NWS_HOST);
  if(sslClient.connect(NWS_HOST, 443)) {
    Serial.println("Connected!");

    //Make an HTTP request
    Serial.print("GET /gridpoints/");
    Serial.print(NWS_LOCATION);
    Serial.println("/forecast HTTP/1.1");
    sslClient.print("GET /gridpoints/");
    sslClient.print(NWS_LOCATION);
    sslClient.println("/forecast HTTP/1.1");

    sslClient.print("Host: ");
    sslClient.println(NWS_HOST);

    sslClient.print("User-Agent: ");
    sslClient.println(NWS_USER_AGENT);

    sslClient.println("Connection: close");
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

    // Parse JSON object
    DeserializationError error = deserializeJson(doc, sslClient);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      sslClient.stop();
      return;
    }

    Serial.println();

    // Extract values
    JsonObject properties = doc["properties"];
    for (JsonObject properties_period : properties["periods"].as<JsonArray>()) {
      if(properties_period["number"]>4) break;

      Serial.println(properties_period["name"].as<char*>());

      if(properties_period["isDaytime"]) Serial.print(F("High "));
      else Serial.print(F("Low "));
      Serial.print(properties_period["temperature"].as<int>());
      Serial.print(F("º"));
      Serial.print(properties_period["temperatureUnit"].as<char*>());
      Serial.print(F(" "));
      Serial.println(properties_period["shortForecast"].as<char*>());

      //Serial.println(properties_period["detailedForecast"].as<char*>());

      Serial.println();
    }

    // Disconnect
    sslClient.stop();
  } //end connection to NWS
  else {
    Serial.println("Wasn't able to connect to NWS");
  }

  // //ICAL Challenge: parse ICAL
  // Serial.print(F("\nConnecting to calendar host "));
  // Serial.println(ICAL_HOST);
  // if(sslClient.connect(ICAL_HOST, 443)) {
  //   Serial.println("Connected!");

  //   //Make an HTTP request
  //   Serial.print("POST ");
  //   Serial.print(ICAL_PATH);
  //   Serial.println(" HTTP/1.1");
  //   sslClient.print("POST ");
  //   sslClient.print(ICAL_PATH);
  //   sslClient.println(" HTTP/1.1");

  //   sslClient.print("Host: ");
  //   sslClient.println(ICAL_HOST);

  //   // sslClient.print("User-Agent: ");
  //   // sslClient.println(NWS_USER_AGENT);

  //   sslClient.println("Connection: close");

  //   Serial.println("Content-Type: application/json");
  //   Serial.println(ICAL_POSTDATAJSON);
  //   sslClient.println("Content-Type: application/json");
  //   sslClient.println(ICAL_POSTDATAJSON);

  //   //the rest adapted from https://arduinojson.org/v6/example/http-sslClient/
  //   if (sslClient.println() == 0) {
  //     Serial.println(F("Failed to send request"));
  //     sslClient.stop();
  //     return;
  //   }

  //   // Check HTTP status
  //   char status[32] = {0};
  //   sslClient.readBytesUntil('\r', status, sizeof(status));
  //   if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
  //     Serial.print(F("Unexpected response: "));
  //     Serial.println(status);
  //     sslClient.stop();
  //     return;
  //   }

  //   // Skip HTTP headers
  //   char endOfHeaders[] = "\r\n\r\n";
  //   if (!sslClient.find(endOfHeaders)) {
  //     Serial.println(F("Invalid response"));
  //     sslClient.stop();
  //     return;
  //   }

  //   // Try to reuse previously allocated document
  //   // Parse JSON object
  //   DeserializationError error = deserializeJson(doc, sslClient);
  //   if (error) {
  //     Serial.print(F("deserializeJson() failed: "));
  //     Serial.println(error.f_str());
  //     sslClient.stop();
  //     return;
  //   }

  //   Serial.println();

  //   // Extract values
  //   for (JsonObject events_event : doc["events"].as<JsonArray>()) {
  //     Serial.print(events_event["ldstart"].as<char*>());
  //     if(!events_event["allday"]) {
  //       Serial.print(" ");
  //       Serial.print(events_event["timestart"].as<char*>());
  //     }
  //     Serial.print(" ");
  //     Serial.println(events_event["summary"].as<char*>());
  //   }

  //   // Disconnect
  //   sslClient.stop();
  // } //end connection to ICS

} //end fn setup

void loop() {}