#ifndef GOOGLESCRIPTREQUEST
#define GOOGLESCRIPTREQUEST
#include "Arduino.h"
#include <WiFi.h>
#include <WebServer.h>
#include <WifiClientSecure.h>
#include <HTTPClient.h> // Needs to be from the ESP32 platform version 3.2.0 or later, as the previous has problems with http-redirect (google script)

#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include "filter/stringFilter.hpp"
 

struct calendarEntries
{
  String Month;
  String Day;
  String Task;
};

class googleScriptRequest
{
public:
    googleScriptRequest(int size);
    void HTTPRequest(void);
    void sortHTTPRequest(String ID_Calendar);
    bool connectToWifi(void);
    bool checkInternetConnection(void);
    bool waitForInternetConnection(int wait_s);

    String getTask(int index);
    String getDay(int index);
    String getMonth(int index);
    int getEntryCount();

    const String ID_Calendar1 = "#C1;;";
    const String ID_Calendar2 = "#C2;;";

private:
    WiFiManager manager; 
    const char*  ssid = "Salt_2GHz_159505";
    const char* password = "MuUY33L6SERtXG4CDT";

    String response;

    const int calEntryCount;
    struct calendarEntries calEnt[6];

    HTTPClient http;
    const String calendarServer = "script.google.com"; 

   const String calendarRequest = ""

       
};
#endif
