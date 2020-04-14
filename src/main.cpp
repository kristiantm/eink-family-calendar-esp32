/* 

This is the E-Ink Family Calendar project by Argion /  Kristian Thorsted Madsen.
[LINK TO INSTRUCTABLE]

The project was written for a LOLIN 32 ESP32 microcontroller and a 7.5 Waveshare 800x600 e-ink display. 
Also note, that the project is dependent on you uploading a google script, to fetch and sort the calendar entries.

Make sure you configure the credentials.h file before compiling, and have fun.

*/


#include <Arduino.h>
#define ENABLE_GxEPD2_GFX 0
#include <GxEPD2_3C.h>

#include <credentials.h>


#include <WiFi.h>
#include <WifiClientSecure.h>
#include "time.h"
#include "timeheaders.h"
#include <string>
#include <HTTPClient.h>
#include <ArduinoJson.h>              // https://github.com/bblanchon/ArduinoJson needs version v6 or above
#include "Wire.h"



#include <iconsOWM.c>

void drawOWMIcon(String icon);
void drawBitmap(const unsigned char *iconMap, int x, int y, int width, int height);


// ***** for mapping of Waveshare ESP32 Driver Board *****
GxEPD2_3C<GxEPD2_750c_Z08, GxEPD2_750c_Z08::HEIGHT/2> display(GxEPD2_750c_Z08(/*CS=*/ 15, /*DC=*/ 27, /*RST=*/ 26, /*BUSY=*/ 25));
int    wifi_signal, wifisection, displaysection, MoonDay, MoonMonth, MoonYear, start_time;

WiFiClientSecure client; // wifi client object
HTTPClient http;


bool displayCalendar();
String getURL(String url);
bool getRequest(char *urlServer, String urlRequest);
void displayCalendarEntry(String eventTime, String eventName, String eventDesc);
void deepSleepTill(int wakeHour);
bool obtain_wx_data(const String& RequestType);
bool DecodeWeather(WiFiClient& json, String Type);
void drawWind();


// Right now the calendarentries are limited to time and title
struct calendarEntries
{
  String calTime;
  String calTitle;
};

// Main flow of the program. It is designed to boot up, pull the info and refresh the screen, and then go back into deep sleep.
void setup() {

  // Initialize board
  Serial.begin(115200);
  Serial.println("setup");

  //Initialize e-ink display
  display.init(115200); // uses standard SPI pins, e.g. SCK(18), MISO(19), MOSI(23), SS(5)
  SPI.end(); // release standard SPI pins, e.g. SCK(18), MISO(19), MOSI(23), SS(5)
  SPI.begin(13, 12, 14, 15); // Map and init SPI pins SCK(13), MISO(12), MOSI(14), SS(15) - adjusted to the recommended PIN settings from Waveshare - note that this is not the default for most screens

  // Connect for WIFI
  WiFi.begin(ssid1, password1);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println(" CONNECTED");

  // Get time from timeserver - used when going into deep sleep again to ensure that we wake at the right hour
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  //Get and draw calendar on display
  display.setRotation(calendarOrientation);
  displayCalendar(); // Main flow for drawing calendar

  delay(1000);

  // Turn off display before deep sleep
  display.powerOff();
  
  // Deep-sleeping unitl the selected wake-hour - default 5 in the morning
  Serial.println("Refresh done - sleeping");
  deepSleepTill(HOUR_TO_WAKE);

}

// Not used, as we boot up from scratch every time we wake from deep sleep
void loop() {
}

// Main display code - assumes that the display has been initialized
bool displayCalendar()
{

  // Getting calendar from your published google script
  Serial.println("Getting calendar");
  getRequest(calendarServer, calendarRequest);

  String outputStr = client.readString();

  // If redirected - then follow redirect - google always redirect to a temporary URL by default. Note that this at times fail resulting in an empty display. Consider detecting this and retrying.
  if(outputStr.indexOf("Location:") > 0) {
    String newURL = getURL(outputStr);

    getRequest(calendarServer, newURL);

    outputStr = client.readString();
  }

  int indexFrom = 0;
  int indexTo = 0;
  int cutTo = 0;

  String strBuffer = "";

  int count = 0;
  int line = 0;
  struct calendarEntries calEnt[calEntryCount];

  indexFrom = outputStr.lastIndexOf("\n") + 1;

  // Fill calendarEntries with entries from the get-request
  while (indexTo>=0 && line<calEntryCount) {
    count++;
    indexTo = outputStr.indexOf(";",indexFrom);
    cutTo = indexTo;

    if(indexTo != -1) { 
      strBuffer = outputStr.substring(indexFrom, cutTo);
      
      indexFrom = indexTo + 1;
      
      Serial.println(strBuffer);

      if(count == 1) {
        // Set entry time
        calEnt[line].calTime = strBuffer.substring(0,21); //Exclude end date and time to avoid clutter - Format is "Wed Feb 10 2020 10:00"

      } else if(count == 2) {
        // Set entry title
        calEnt[line].calTitle = strBuffer;

      } else {
          count = 0;
          line++;
      }
    }
  }

  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
  }

  String weatherIcon;

  // Get weather info using the OWM API "onecall" which gives you current info, 3-hour forecast and 5-day forecast in one call. We only use the first day of the 5-day forecast
  bool weatherSuccess = obtain_wx_data("onecall");
  if(weatherSuccess) {
    weatherIcon = WxForecast[0].Icon.substring(0,2);

    Serial.println("Weatherinfo");
    Serial.println(WxForecast[0].Icon);
    Serial.println(WxForecast[0].High);
    Serial.println(WxForecast[0].Winddir);
    Serial.println(WxForecast[0].Windspeed);
    Serial.println(WxForecast[0].Rainfall);
  }  


  // All data is now gathered and processed.
  // Prepare to refresh the display with the calendar entries and weather info

  // Turn off text-wrapping
  display.setTextWrap(false);

  display.setRotation(calendarOrientation);

  // Clear the screen with white using full window mode. Not strictly nessecary, but as I selected to use partial window for the content, I decided to do a full refresh first.
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
  }   while(display.nextPage());

  // Print the content on the screen - I use a partial window refresh for the entire width and height, as I find this makes a clearer picture
  display.setPartialWindow(0, 0, display.width(), display.height());
  display.firstPage();
  do {
    int x = calendarPosX;
    int y = calendarPosY;

    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);

    // Print mini-test in top in white (e.g. not visible) - avoids a graphical glitch I observed in all first-lines printed
    display.setCursor(x, 0);
    display.setTextColor(GxEPD_WHITE);
    display.setFont(fontEntryTime);
    display.print(weekday[timeinfo.tm_wday]);



    // Print morning greeting (Happy X-day)
    display.setCursor(x, y);
    display.setTextColor(GxEPD_BLACK);
    display.setFont(fontMainTitle);
    display.print(weekday[timeinfo.tm_wday]);

    // If fetching the weather was a succes, then print the weather
    if(weatherSuccess) {
      drawOWMIcon(weatherIcon);
    }


    // Set position for the first calendar entry
    y = y + 45;
    
    // Print calendar entries from first [0] to the last fetched [line-1] - in case there is fewer events than the maximum allowed
    for(int i=0;  i < line; i++) {

      // Print event time
      display.setCursor(x, y);
      display.setFont(fontEntryTime);
      display.print(calEnt[i].calTime);

      // Print event title
      display.setCursor(x, y + 30);
      display.setFont(fontEntryTitle);
      display.print(calEnt[i].calTitle);

      // Prepare y-position for next event entry
      y = y + calendarSpacing;

    }

  } while(display.nextPage());
  
  client.stop();
  return true;
}

// Generic code for getting requests - doing it a bit pure, as most libraries I tried could not handle the redirects from google
bool getRequest(char *urlServer, String urlRequest) {
  client.stop(); // close connection before sending a new request
  
  if (client.connect(urlServer, 443)) { // if the connection succeeds
    Serial.println("connecting...");
    // send the HTTP PUT request:
    client.println("GET " + urlRequest); // " HTTP/1.1"
    Serial.println("GET " + urlRequest);
    client.println("User-Agent: ESP OWM Receiver/1.1");
    client.println();

    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 10000) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        Serial.println("Connection timeout");
        return false;
      }
    }
  } else {
    Serial.println("Calendar connection did not succeed");
  }

  return true;

}

/* Extract URL from http redirect - used especially for google as they always redirect to a new temporary URL */
String getURL(String url) {
  String tagStr = "Location: ";
  String endStr = "\n";

  int indexFrom = url.indexOf(tagStr) + tagStr.length();
  int indexTo = url.indexOf(endStr, indexFrom);

  return url.substring(indexFrom, indexTo);

}

// Sleep until set wake-hour
void deepSleepTill(int wakeHour)
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
  }

  int wakeInSec = 0;

  if(timeinfo.tm_hour < wakeHour) {
      wakeInSec = (wakeHour - timeinfo.tm_hour - 1) * 60 * 60;
  } else {
      wakeInSec = (wakeHour - timeinfo.tm_hour + 24 - 1) * 60 * 60;
  }

  // Add minutes
  wakeInSec = wakeInSec + (60 - timeinfo.tm_min) * 60;

  // Add seconds
  wakeInSec = wakeInSec + (60 - timeinfo.tm_sec);

  Serial.print("Wake in sec: ");
  Serial.println(wakeInSec);

  esp_sleep_enable_timer_wakeup(wakeInSec * uS_TO_S_FACTOR);
  Serial.flush(); 
  esp_deep_sleep_start();

}

// Get and decode weather data from OWM - credits to ESP32 Weather Station project on GIT, which I have heavily modified from - using onecall instead of forecast and current for instance. I have removed the code for weather and forecast types
bool obtain_wx_data(const String& RequestType) {
  const String units = (Units == "M" ? "metric" : "imperial");
  client.stop(); // close connection before sending a new request
  HTTPClient http;
  String uri = "/data/2.5/" + RequestType + "?APPID=" + OWMapikey + "&mode=json&units=" + units + "&lang=" + Language + "&lat=" + Lattitude + "&lon=" + Longitude;

  Serial.println(uri);

  bool httpSucceeded;
  httpSucceeded = getRequest(OWMserver, uri);

  if(httpSucceeded) {
    if (!DecodeWeather(client, RequestType)) return false;
    client.stop();
    http.end();
    return true;
  }
  else
  {
    client.stop();
    http.end();
    return false;
  }
  http.end();
  return true;
}

// Get and decode weather data from OWM - credits to ESP32 Weather Station project on GIT, which I have heavily modified from - only using onecall instead of forecast and current for instance
bool DecodeWeather(WiFiClient& json, String Type) {
  Serial.print(F("\nCreating object...and "));
  // allocate the JsonDocument
  DynamicJsonDocument doc(35 * 1024);
  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, json);
  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return false;
  }
  // convert it to a JsonObject
  JsonObject root = doc.as<JsonObject>();
  Serial.println(" Decoding " + Type + " data");

  if (Type == "onecall") {
    // Selecting "daily" forecast for the next five days - alternatives are "forecast" for 3-hour forcasts, and "current" for the weather right now
    JsonArray list                  = root["daily"];
    int listLength = list.size();
    Serial.println(listLength);

    // Collecting the whole forecast - even though I am only using the first day list[0]
    for (byte r = 0; r < listLength; r++) {
      Serial.println("\nPeriod-" + String(r) + "--------------");
      WxForecast[r].Dt                = list[r]["dt"].as<char*>();
      WxForecast[r].Temperature       = list[r]["temp"]["day"].as<float>();              Serial.println("Temp: "+String(WxForecast[r].Temperature));
      WxForecast[r].Low               = list[r]["temp"]["min"].as<float>();          Serial.println("TLow: "+String(WxForecast[r].Low));
      WxForecast[r].High              = list[r]["temp"]["max"].as<float>();          Serial.println("THig: "+String(WxForecast[r].High));
      WxForecast[r].Pressure          = list[r]["pressure"].as<float>();          Serial.println("Pres: "+String(WxForecast[r].Pressure));
      WxForecast[r].Humidity          = list[r]["humidity"].as<float>();          Serial.println("Humi: "+String(WxForecast[r].Humidity));
      WxForecast[r].Icon              = list[r]["weather"][0]["icon"].as<char*>();        Serial.println("Icon: "+String(WxForecast[r].Icon));
      WxForecast[r].Description       = list[r]["weather"][0]["description"].as<char*>(); Serial.println("Desc: "+String(WxForecast[r].Description));
      WxForecast[r].Cloudcover        = list[r]["clouds"].as<int>();               Serial.println("CCov: "+String(WxForecast[r].Cloudcover)); // in % of cloud cover
      WxForecast[r].Windspeed         = list[r]["wind_speed"].as<float>();             Serial.println("WSpd: "+String(WxForecast[r].Windspeed));
      WxForecast[r].Winddir           = list[r]["wind_deg"].as<float>();               Serial.println("WDir: "+String(WxForecast[r].Winddir));
      WxForecast[r].Rainfall          = list[r]["rain"].as<float>();                Serial.println("Rain: "+String(WxForecast[r].Rainfall));
    }
  }

  return true;
}

// Draw a weathericon based on the "Icon" string from OWM - we select and print the right one from the iconsOWM.c - see https://openweathermap.org/weather-conditions
void drawOWMIcon(String icon) {
  int x = weatherPosX+5;
  int y = weatherPosY;
  int width = 100;
  int height = 100;
  Serial.println("Icon: ");
  Serial.println(icon);

  if(icon=="01") {
    drawBitmap(icon01Map, x, y, width, height);
  } else if(icon=="02") {
    drawBitmap(icon02Map, x, y, width, height);
  } else if(icon=="03") {
    drawBitmap(icon03Map, x, y, width, height);
  } else if(icon=="04") {
    drawBitmap(icon04Map, x, y, width, height);
  } else if(icon=="09") {
    drawBitmap(icon09Map, x, y, width, height);
  } else if(icon=="10") {
    drawBitmap(icon10Map, x, y, width, height);
  } else if(icon=="11") {
    drawBitmap(icon11Map, x, y, width, height);
  } else if(icon=="13") {
    drawBitmap(icon13Map, x, y, width, height);
  } else if(icon=="50") {
    drawBitmap(icon50Map, x, y, width, height);
  } else {
    Serial.println("No icon selected");
    Serial.println(icon);
  }

  drawWind();

}

// Draw a bitmap - used for the weather icon
void drawBitmap(const unsigned char *iconMap, int x, int y, int width, int height) {

  Serial.println("Drawing bitmap");

  display.drawInvertedBitmap(x, y, iconMap, width, height, GxEPD_BLACK);

  String weatherInfo = "" + String(round(WxForecast[0].Low),0) + "C / " + String(round(WxForecast[0].High),0) + "C";
  
  display.setCursor(x+5,y+height-10);
  display.setTextColor(GxEPD_BLACK);
  display.setFont(fontWeather);

  display.print(weatherInfo);

}

// Combining the wind-info-line
void drawWind() {
  int x = weatherPosX;
  int y = weatherPosY;
  int height = 100;

  float dir = WxForecast[0].Winddir;
  String txtDirection;

  float div = 360/8;
  float start = div / 2;

  if(dir < start  ) {
    txtDirection = "N";
  } else if(dir < (div*1 + start)) {
    txtDirection = "NE";
  } else if(dir < (div*2 + start)) {
    txtDirection = "E";
  } else if(dir < (div*3 + start)) {
    txtDirection = "SE";
  } else if(dir < (div*4 + start)) {
    txtDirection = "S";
  } else if(dir < (div*5 + start)) {
    txtDirection = "SW";
  } else if(dir < (div*6 + start)) {
    txtDirection = "W";
  } else if(dir < (div*7 + start)) {
    txtDirection = "NW";
  } else {
    txtDirection = "N";
  }

  float speed = WxForecast[0].Windspeed;
  String txtSpeed;

  if(speed < 1.5) {
    txtSpeed = "Calm";
  } else if(speed < 3.3) {
    txtSpeed = "Gentle";
  } else if(speed < 7.9) {
    txtSpeed = "Moderate";
  } else if(speed < 10.7) {
    txtSpeed = "Fresh";
  } else if(speed < 13.8) {
    txtSpeed = "Hard";
  } else if(speed < 17.1) {
    txtSpeed = "Near gale";
  } else if(speed < 20.7) {
    txtSpeed = "Gale";
  } else if(speed < 24.4) {
    txtSpeed = "Hard gale";
  } else if(speed < 28.4) {
    txtSpeed = "Storm";
  } else if(speed < 32.6) {
    txtSpeed = "Hard storm";
  } else {
    txtSpeed = "Hurricane";
  }

  String windInfo = "" + String(speed,0) +"ms " + txtDirection + " " + txtSpeed;

  Serial.println("Drawing wind");

  display.setCursor(x-30,y+height+5); // Below temperature settings
  display.setTextColor(GxEPD_BLACK);
  display.setFont(fontWeather);

  display.print(windInfo);

}

