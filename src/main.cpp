/*

This is the E-Ink Family Calendar project by Argion /  Kristian Thorsted Madsen.
https://www.instructables.com/id/E-Ink-Family-Calendar-Using-ESP32/

The project was written for a LOLIN 32 ESP32 microcontroller and a 7.5 Waveshare 800x600 e-ink display.
Also note, that the project is dependent on you uploading a google script, to fetch and sort the calendar entries.

The basic configuration now happens in the WIFI hotspot that is booting when you power on the board.
The one exception is in case you either use a tri-colour waveshare e-ink screen, or have a different pin-mapping than in the instructions.
In that case adjust the lines in this file, under the comment "Mapping of Waveshare ESP32 Driver Board"

*/

#include <Arduino.h>
#define ENABLE_GxEPD2_GFX 0
#include <GxEPD2_3C.h>
//#include <GxEPD2_BW.h>

#include <credentials.h>

#include "time.h"
#include "timeheaders.h"
#include <string>
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson needs version v6 or above
#include "Wire.h"

#include <iconsOWM.c>

#include <WiFi.h>
#include <WebServer.h>
#include <WifiClientSecure.h>
#include <HTTPClient.h> // Needs to be from the ESP32 platform version 3.2.0 or later, as the previous has problems with http-redirect (google script)

#include <SPIFFS.h>
using WebServerClass = WebServer;
#include <FS.h>
#include <AutoConnect.h>
#include <webconfig.h>

WebServerClass server;
AutoConnect portal(server);
AutoConnectConfig config;
AutoConnectAux elementsAux;
AutoConnectAux saveAux;

extern int dayCalender[7][5];

void drawBitmap(const unsigned char *iconMap, int x, int y, int width, int height);

/* Mapping of Waveshare ESP32 Driver Board - 3C is tri-color displays, BW is black and white ***** */
// GxEPD2_3C<GxEPD2_750c_Z08, GxEPD2_750c_Z08::HEIGHT/2> display(GxEPD2_750c_Z08(/*CS=*/ 15, /*DC=*/ 27, /*RST=*/ 26, /*BUSY=*/ 25));
// GxEPD2_BW<GxEPD2_750_T7, GxEPD2_750_T7::HEIGHT/2> display(GxEPD2_750_T7(/*CS=*/15, /*DC=*/ 27, /*RST=*/ 26, /*BUSY=*/ 25)); // GDEW075T7 800x480

// GxEPD2_BW<GxEPD2_750, GxEPD2_750::HEIGHT> display(GxEPD2_750(/*CS=5*/ SS, /*DC=*/17, /*RST=*/16, /*BUSY=*/4)); // GDEW075T8   640x384
GxEPD2_3C<GxEPD2_750c, GxEPD2_750c::HEIGHT> display(GxEPD2_750c(/*CS=5*/ SS, /*DC=*/17, /*RST=*/16, /*BUSY=*/4));

int wifi_signal, wifisection, displaysection, MoonDay, MoonMonth, MoonYear, start_time;

HTTPClient http;

bool displayCalendar();
void deepSleepTill(int wakeHour);
void readBattery();
bool internetWorks();

// Right now the calendarentries are limited to time and title
struct calendarEntries
{
  String Month;
  String Day;
  String Task;
};

float batterylevel = -1; // Being set when reading battery level - used to avoid deep sleep when under 0%, and when drawing battery

// Main flow of the program. It is designed to boot up, pull the info and refresh the screen, and then go back into deep sleep.
void setup()
{

  // Initialize board
  Serial.begin(115200);
  Serial.println("setup");

  // Initialize e-ink display
  display.init(115200);      // uses standard SPI pins, e.g. SCK(18), MISO(19), MOSI(23), SS(5)
  SPI.end();                 // release standard SPI pins, e.g. SCK(18), MISO(19), MOSI(23), SS(5)
  SPI.begin(18, 12, 23, 15); // Map and init SPI pins SCK(13), MISO(12), MOSI(14), SS(15) - adjusted to the recommended PIN settings from Waveshare - note that this is not the default for most screens
  // SPI.begin(18, 19, 23, 5); //

  WiFi.setHostname("WG-Kalender");

  bool isWebConnected = false;

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  isWebConnected = internetWorks();

  if ((isWebConnected))
  {
    Serial.println("Configuration exist and internet connection works - displaying calendar");

    // Get time from timeserver - used when going into deep sleep again to ensure that we wake at the right hour
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    // Read batterylevel and set batterylevel variable
    // readBattery();

    // Get and draw calendar on display
    display.setRotation(calendarOrientation);
    displayCalendar(); // Main flow for drawing calendar

    delay(1000);

    // Turn off display before deep sleep
    display.powerOff();
  }

  deepSleepTill(HOUR_TO_WAKE);
}

// Not used, as we boot up from scratch every time we wake from deep sleep
void loop()
{
}

bool internetWorks()
{
  HTTPClient http;
  if (http.begin("script.google.com", 443))
  {
    http.end();
    return true;
  }
  else
  {
    http.end();
    return false;
  }
}

void drawCentreString(const char *buf, int x, int y)
{
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(buf, x, y, &x1, &y1, &w, &h); // calc width of new string
  display.setCursor(x - w / 2, y);
  display.print(buf);
}

void setCalendarArray()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
  }
  int rest_day = timeinfo.tm_mday;

  // Get number of weeks befor today
  int week_cnt = 0;
  while ((rest_day) > 6)
  {
    rest_day -= 7;
    week_cnt++;
  }
  Serial.print("week_cnt: ");
  Serial.println(week_cnt);
  Serial.print("restday_cnt: ");
  Serial.println(rest_day);

  int status = 0;

  for (int mday = 0; mday < 7 - rest_day; mday++)
  {
    dayCalender[mday][0] = daysOfMonth[timeinfo.tm_mon - 1] - (7 - rest_day) + 1 + mday;
    Serial.print("[");
    Serial.print(mday);
    Serial.print("][");
    Serial.print(0);
    Serial.print("] = ");
    Serial.println(daysOfMonth[timeinfo.tm_mon - 1] - (7 - rest_day) + 1 + mday);
  }

  for (int mday = 0; mday < rest_day; mday++)
  {
    dayCalender[convertTmWeekday[timeinfo.tm_wday] - mday][0] = rest_day - mday;
    Serial.print("[");
    Serial.print(convertTmWeekday[timeinfo.tm_wday] - mday);
    Serial.print("][");
    Serial.print(0);
    Serial.print("] = ");
    Serial.println(rest_day - mday);
  }

  int mweek = 0;
  for (int mday = rest_day + 1; mday <= rest_day + 7; mday++)
  {
    dayCalender[mweek][1] = mday;
    Serial.print("[");
    Serial.print(mweek);
    Serial.print("][");
    Serial.print(1);
    Serial.print("] = ");
    Serial.println(mday);
    mweek++;
  }

  mweek = 0;
  for (int mday = rest_day + 7 + 1; mday <= rest_day + 14; mday++)
  {
    dayCalender[mweek][2] = mday;
    Serial.print("[");
    Serial.print(mweek);
    Serial.print("][");
    Serial.print(2);
    Serial.print("] = ");
    Serial.println(mday);
    mweek++;
  }

  mweek = 0;
  for (int mday = rest_day + 14 + 1; mday <= rest_day + 21; mday++)
  {
    dayCalender[mweek][3] = mday;
    Serial.print("[");
    Serial.print(mweek);
    Serial.print("][");
    Serial.print(3);
    Serial.print("] = ");
    Serial.println(mday);
    mweek++;
  }

  mweek = 0;
  for (int mday = rest_day + 21 + 1; mday <= daysOfMonth[timeinfo.tm_mon]; mday++)
  {
    dayCalender[mweek][4] = mday;
    Serial.print("[");
    Serial.print(mweek);
    Serial.print("][");
    Serial.print(4);
    Serial.print("] = ");
    Serial.println(mday);
    mweek++;
  }
}

// Main display code - assumes that the display has been initialized
bool displayCalendar()
{

  // Getting calendar from your published google script
  Serial.println("Getting calendar");
  Serial.println(calendarRequest);

  http.end();
  http.setTimeout(20000);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  if (!http.begin(calendarRequest))
  {
    Serial.println("Cannot connect to google script");
    return false;
  }

  Serial.println("Connected to google script");
  int returnCode = http.GET();
  Serial.print("Returncode: ");
  Serial.println(returnCode);
  String response = http.getString();
  Serial.print("Response: ");
  Serial.println(response);

  int indexFrom = 0;
  int indexTo = 0;
  int cutTo = 0;
  int nextSemiq = 0;
  int cutDateIndex = 0;

  String strBuffer = "";

  int count = 0;
  int line = 0;
  struct calendarEntries calEnt[calEntryCount];

  Serial.println("IntexFrom");
  indexFrom = response.length() - 1;

  // Fill calendarEntries with entries from the get-request
  while (nextSemiq >= 0 && line < calEntryCount)
  {
    nextSemiq = response.indexOf(";", 0);
    strBuffer = response.substring(0, nextSemiq);
    response = response.substring(nextSemiq + 1, indexFrom);

    switch (count)
    {
    case 0:
      cutDateIndex = strBuffer.indexOf(" ", 0);
      calEnt[line].Day = strBuffer.substring(0, cutDateIndex); // Exclude end date and time to avoid clutter - Format is "Wed Feb 10 2020 10:00"
      Serial.print("Day: ");
      Serial.println(calEnt[line].Day);

      calEnt[line].Month = strBuffer.substring(cutDateIndex);
      Serial.print("Mont: ");
      Serial.println(calEnt[line].Month);
      count = 1;
      break;

    case 1:
      calEnt[line].Task = strBuffer;
      Serial.print("Task: ");
      Serial.println(calEnt[line].Task);
      count = 0;
      line++;
      break;

    default:
      count = 0;
      break;
    }
  }

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
  }

  // Turn off text-wrapping
  display.setTextWrap(false);

  display.setRotation(calendarOrientation);

  // Clear the screen with white using full window mode. Not strictly nessecary, but as I selected to use partial window for the content, I decided to do a full refresh first.
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
  } while (display.nextPage());

  // Print the content on the screen - I use a partial window refresh for the entire width and height, as I find this makes a clearer picture
  display.setPartialWindow(0, 0, display.width(), display.height());
  display.firstPage();
  do
  {
    // Print mini-test in top in white (e.g. not visible) - avoids a graphical glitch I observed in all first-lines printed
    // display.setCursor(0, 384); //640x384
    // display.setTextColor(GxEPD_BLACK);
    // display.setFont(fontDescription);
    // display.print("Graphical Flitch");

    // Frame Calendar
    display.fillRect(0, 0, 240, 384, GxEPD_BLACK);
    display.setTextColor(GxEPD_WHITE);

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
      Serial.println("Failed to obtain time");
    }

    display.setFont(fontTitle);
    drawCentreString(weekdayNumbers[timeinfo.tm_wday], 120, 50);

    display.setFont(fontMassiveTitle);
    drawCentreString(String(timeinfo.tm_mday).c_str(), 120, 105);

    display.setFont(fontDescription);
    String monthYear = monthNumbers[timeinfo.tm_mon];
    monthYear += " ";
    monthYear += (timeinfo.tm_year + 1900);
    drawCentreString((monthYear).c_str(), 120, 150);

    const int dx = 30;
    const int dy = 32;
    const int x_Start = 28;
    int x = x_Start;
    int y = 230;

    display.setFont(fontSmallDescription);
    display.setTextColor(GxEPD_WHITE);
    display.setCursor(10, 180);
    for (int weekday = 0; weekday < 7; weekday++)
    {
      drawCentreString(String(weekdayNumbersShort[weekday]).c_str(), x, 200);
      x += dx;
    }

    setCalendarArray();
    x = x_Start;
    for (int column = 0; column < 5; column++)
    {
      for (int row = 0; row < 7; row++)
      {
        if (dayCalender[row][column] == timeinfo.tm_mday)
        {
          display.setTextColor(GxEPD_RED);
          display.fillRoundRect(x - 7, y - 15, 20, 20, 5, GxEPD_WHITE);
        }
        else
        {
          display.setTextColor(GxEPD_WHITE);
        }

        drawCentreString(String(dayCalender[row][column]).c_str(), x, y);
        x += dx;
      }
      y += dy;
      x = x_Start;
    }

    // Frame Tasks
    display.fillRect(250, 5, 400, 55, GxEPD_BLACK);
    display.setTextColor(GxEPD_WHITE);
    display.setCursor(265, 45);
    display.setFont(fontTitle);
    display.print("Tasks");

    display.setTextColor(GxEPD_BLACK);
    x = 260;
    y = 70;

    // Set position for the first calendar entry
    y = y + 35;

    // Print calendar entries from first [0] to the last fetched [line-1] - in case there is fewer events than the maximum allowed
    for (int i = 0; i < line; i++)
    {

      // Print event title
      display.setCursor(x, y);
      display.setTextColor(GxEPD_BLACK);
      display.setFont(fontDescription);
      display.print(calEnt[i].Task);

      display.fillRect(570, y - 15, 70, 25, GxEPD_BLACK);
      display.setCursor(580, y + 5);
      display.setFont(fontSmallDescription);
      display.setTextColor(GxEPD_WHITE);
      String TaskDate = calEnt[i].Day + " " + monthNumbersShort[(calEnt[i].Month).toInt()];
      display.print(TaskDate);

      if (timeinfo.tm_mday == (calEnt[i].Day).toInt())
      {
        display.fillRect(570, y - 35, 70, 25, GxEPD_RED);
        display.setCursor(580, y - 15);
        display.setFont(fontSmallDescription);
        display.setTextColor(GxEPD_WHITE);
        display.print("Heute");
      }

      // Add Line between Tasks
      display.fillRect(x, y + 10, 380, 2, GxEPD_BLACK);

      // Prepare y-position for next event entry
      y = y + calendarSpacing;
    }

  } while (display.nextPage());

  return true;
}

// Sleep until set wake-hour
void deepSleepTill(int wakeHour)
{
  // If battery is too low (see getBattery code), enter deepSleep and do not wake up
  if (batterylevel == 0)
  {
    esp_deep_sleep_start();
  }

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
  }

  int wakeInSec = 0;

  if (timeinfo.tm_hour < wakeHour)
  {
    wakeInSec = (wakeHour - timeinfo.tm_hour - 1) * 60 * 60;
  }
  else
  {
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

void readBattery()
{
  uint8_t percentage = 100;

  // Adjust the pin below depending on what pin you measure your battery voltage on.
  // On LOLIN D32 boards this is build into pin 35 - for other ESP32 boards, you have to manually insert a voltage divider between the battery and an analogue pin
  uint8_t batteryPin = 34; // FIXME

  // Set OHM values based on the resistors used in your voltage divider http://www.ohmslawcalculator.com/voltage-divider-calculator
  float R1 = 30;
  float R2 = 100;

  float voltage = analogRead(batteryPin) / 4096.0 * (1 / (R1 / (R1 + R2)));
  if (voltage > 1)
  { // Only display if there is a valid reading
    Serial.println("Voltage = " + String(voltage));

    if (voltage >= 4.1)
      percentage = 100;
    else if (voltage >= 3.9)
      percentage = 75;
    else if (voltage >= 3.7)
      percentage = 50;
    else if (voltage >= 3.6)
      percentage = 25;
    else if (voltage <= 3.5)
      percentage = 0;
    Serial.println("Batterylevel = " + String(percentage));
    batterylevel = percentage;
  }
}
