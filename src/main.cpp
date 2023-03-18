/*-------------------------------------------------------------------------------------
-- Includes
-------------------------------------------------------------------------------------*/
#include <Arduino.h>
#define ENABLE_GxEPD2_GFX 0
#include <GxEPD2_3C.h>
//#include <GxEPD2_BW.h>

#include <credentials.h>

#include "time.h"
#include "timeheaders.h"
#include <string>
#include <ArduinoJson.h> // needs version v6 or above
#include "Wire.h"
#include "gui/scale.hpp"

#include "google/googleScriptRequest.hpp"

#include "gui/task.hpp"
#include "google/timeServer.hpp"

/*-------------------------------------------------------------------------------------
-- Variable
-------------------------------------------------------------------------------------*/
/*------------------------------  GDEW075T8    640x384  -----------------------------*/
/* Development-Kit v4 */
// GxEPD2_3C<GxEPD2_750c, GxEPD2_750c::HEIGHT> display(GxEPD2_750c(/*CS=5*/ SS, /*DC=*/17, /*RST=*/16, /*BUSY=*/4)); // 640x384

/* Camille Chatton E-Paper-Driver */
GxEPD2_3C<GxEPD2_750c, GxEPD2_750c::HEIGHT> display(GxEPD2_750c(/*CS=5*/ 15, /*DC=*/27, /*RST=*/26, /*BUSY=*/25)); // 640x384

const double x_resolution_multipliere = display.width() / 640.0; // Layout defined for 640x384                                               // Layout defined for 640x384
const double y_resolution_multipliere = display.height() / 384.0;

extern int dayCalendar[7 * 5];

float batterylevel = -1;

googleScriptRequest GSR(6);


const int IO16_SetCalendar = 16;
const int IO17_Layout = 17;
const int IO18 = 18;
const int IO19 = 19;
const int IO32_uC_Led = 32;

RTC_DATA_ATTR int day_of_last_update = 0;

#define HOUR_TO_WAKE  1        // Time ESP32 will wake and refresh at a specific hour each day - default is five in the morning
#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define S_TO_H_FACTOR 3600LL  /* Conversion factor for micro seconds to seconds */

/*-------------------------------------------------------------------------------------
-- Function Prototypes
-------------------------------------------------------------------------------------*/
bool displayCalendar();
bool displaytasks();
void deepSleepTill(int wakeHour);
void readBattery();
void drawCentreString(const char *buf, int x, int y);
void initCalendarArray();
void readChargingStatus(void);
void print_wakeup_reason(void); 
void showNoInternetManual(void);

// Main flow of the program. It is designed to boot up, pull the info and refresh the screen, and then go back into deep sleep.
void setup()
{
  pinMode(IO16_SetCalendar, INPUT_PULLUP); // sets the digital pin 13 as output
  pinMode(IO17_Layout, INPUT_PULLUP);  // sets the digital pin 13 as output
  pinMode(IO18, INPUT); // sets the digital pin 13 as output
  pinMode(IO19, INPUT);  // sets the digital pin 13 as output
  pinMode(IO32_uC_Led, OUTPUT);
  // Initialize board
  Serial.begin(115200);
  delay(100);
  Serial.println("Start FW");

  // Print wakeup reason
  print_wakeup_reason();

  // Initialize e-ink display
  display.init(115200);      // uses standard SPI pins, e.g. SCK(18), MISO(19), MOSI(23), SS(5)
  SPI.end();                 // release standard SPI pins, e.g. SCK(18), MISO(19), MOSI(23), SS(5)
  SPI.begin(13, 12, 14, 15); // Map and init SPI pins SCK(18), MISO(12), MOSI(23), SS(15)
  
  
  // I/0 Pins
  digitalWrite(IO32_uC_Led,LOW);
  
  if (GSR.connectToWifi())
  {

    if (GSR.checkInternetConnection())
    {
      // Get time from timeserver - used when going into deep sleep again to ensure that we wake at the right hour
      //configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
      timeServer::getTimefromServer();



      // Read batterylevel and set batterylevel variable
      // readBattery();
      // readChargingStatus();

      // Get Tasks from Google Calendar
      GSR.HTTPRequest();
      if (digitalRead(IO16_SetCalendar)) {
         GSR.sortHTTPRequest(GSR.ID_Calendar1);
      }
      else {
         GSR.sortHTTPRequest(GSR.ID_Calendar2);
      }

     

      // Get and draw calendar on display
      display.setRotation(calendarOrientation);
      if(digitalRead(IO17_Layout)) {
        displayCalendar();
      }
      else { 
        displaytasks();
        //task tk;
        //tk.draw(display);
      }
      
      day_of_last_update = timeServer::getTimeStruct().tm_mday;
    }
  } else 
  {
    showNoInternetManual();
  }
  // Turn off display before deep sleep
  display.powerOff();
  deepSleepTill(HOUR_TO_WAKE);
}

void drawCentreString(const char *buf, int x, int y)
{
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(buf, x, y, &x1, &y1, &w, &h); // calc width of new string
  display.setCursor(x - w / 2, y);
  display.print(buf);
}

void initCalendarArray()
{
  // Berechne Tag vom ersten Sonntag im Monat

  int date_of_first_sunnday = 0;
  int rest_day = 0;
  int week_cnt = 0;
  //Serial.print("timeServer::getTimeStruct().tm_mday  = ");
  //Serial.println(timeServer::getTimeStruct().tm_mday);
  //Serial.print("convertTmWeekday[timeServer::getTimeStruct().tm_wday] + 1  = ");
  //Serial.println(convertTmWeekday[timeServer::getTimeStruct().tm_wday] + 1);

  if (timeServer::getTimeStruct().tm_mday <= convertTmWeekday[timeServer::getTimeStruct().tm_wday] + 1)
  {
    date_of_first_sunnday = timeServer::getTimeStruct().tm_mday + (6 - convertTmWeekday[timeServer::getTimeStruct().tm_wday]);
    //Serial.print("date_of_first_sunnday = ");
    //Serial.print("timeServer::getTimeStruct().tm_mday + (6 - convertTmWeekday[timeServer::getTimeStruct().tm_wday = ");
    Serial.println(timeServer::getTimeStruct().tm_mday + (6 - convertTmWeekday[timeServer::getTimeStruct().tm_wday]));
  }
  else
  {
    int date_of_sunday_before = timeServer::getTimeStruct().tm_mday - (convertTmWeekday[timeServer::getTimeStruct().tm_wday] + 1);
    //Serial.print("date_of_sunday_before = ");
    //Serial.print("timeServer::getTimeStruct().tm_mday - (convertTmWeekday[timeServer::getTimeStruct().tm_wday] + 1");
    Serial.println(timeServer::getTimeStruct().tm_mday - (convertTmWeekday[timeServer::getTimeStruct().tm_wday] + 1));

    while ((date_of_sunday_before) > 6)
    {
      date_of_sunday_before -= 7;
    }
    date_of_first_sunnday = date_of_sunday_before;
    //Serial.print("date_of_first_sunnday: ");
    Serial.println(date_of_first_sunnday);
  }

  rest_day = 7 - date_of_first_sunnday;
  //Serial.print("rest_day: ");
  //Serial.println(rest_day);



  int calendar_week_cnt = 0;
  if (rest_day != 0)
  {
    for (int mday = 0; mday < rest_day; mday++)
    {
      // Spezcial December
      if (timeServer::getTimeStruct().tm_mon == 0) {
        dayCalendar[mday + calendar_week_cnt] = ((daysOfMonth[11] - rest_day) + 1 + mday);
      } else {
        dayCalendar[mday + calendar_week_cnt] = ((daysOfMonth[timeServer::getTimeStruct().tm_mon - 1] - rest_day) + 1 + mday);
      }
      Serial.print("[");
      Serial.print(mday + calendar_week_cnt);
      Serial.print("] = ");
      Serial.println(((daysOfMonth[11] - rest_day) + 1 + mday));
    }


    int inc_day_cnt=1;
    for (int mday = rest_day; mday < 7; mday++)
    {
      dayCalendar[mday + calendar_week_cnt] = inc_day_cnt;
      Serial.print("[");
      Serial.print(mday + calendar_week_cnt);
      Serial.print("] = ");
      Serial.println(inc_day_cnt);
      inc_day_cnt++;
    }
    calendar_week_cnt = 7;
  }
  else
  {
    calendar_week_cnt = 0;
  }

  for (int mday = date_of_first_sunnday + 1; mday <= daysOfMonth[timeServer::getTimeStruct().tm_mon]; mday++)
  {
    dayCalendar[calendar_week_cnt] = mday;
    Serial.print("[");
    Serial.print(calendar_week_cnt);
    Serial.print("] = ");
    Serial.println(mday);
    
    if(calendar_week_cnt>=35) {
      break;
    } else {
      calendar_week_cnt++;
    }
  }
  rest_day = 35 - calendar_week_cnt;
  for (int mday = 1; mday <= rest_day; mday++)
  {
    dayCalendar[calendar_week_cnt] = mday;
    Serial.print("[");
    Serial.print(calendar_week_cnt);
    Serial.print("] = ");
    Serial.println(mday);
    calendar_week_cnt++;
  }
}
// Main display code - assumes that the display has been initialized
bool displayCalendar()
{
  /*-----------------------------------
  -- Init E-Paper
  -----------------------------------*/
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

  /*-----------------------------------
  -- Draw E-Paper
  -----------------------------------*/
  display.firstPage();
  do
  {
    /*-------------------------------------------------------------------------------------
    -- Kalenderübersicht
    -------------------------------------------------------------------------------------*/
    /*-- Titel Kalenderübersicht --*/
    display.fillRoundRect(scale::x_resMtp(0, display.width()), scale::y_resMtp(0, display.height()), scale::x_resMtp(240, display.width()), scale::y_resMtp(170, display.height()), 10, GxEPD_BLACK);
    display.setTextColor(GxEPD_WHITE);

    /*-- Wochentag --*/
    display.setFont(fontTitle);
    drawCentreString(weekdayNumbers[timeServer::getTimeStruct().tm_wday], scale::x_resMtp(120, display.width()), scale::y_resMtp(50, display.height()));

    /*-- Tag --*/
    display.setFont(fontMassiveTitle);
    drawCentreString(String(timeServer::getTimeStruct().tm_mday).c_str(), scale::x_resMtp(120, display.width()), scale::y_resMtp(105, display.height()));

    /*-- Monat + Jahr --*/
    display.setFont(fontDescription);
    String monthYear = monthNumbers[timeServer::getTimeStruct().tm_mon];
    monthYear += " ";
    monthYear += (timeServer::getTimeStruct().tm_year + 1900);
    drawCentreString((monthYear).c_str(), scale::x_resMtp(120, display.width()), scale::y_resMtp(150, display.height()));

    /*-- Wochentage im Kalender --*/
    const int dx = scale::x_resMtp(30, display.width());
    const int dy = scale::y_resMtp(32, display.height());
    const int x_Start = scale::x_resMtp(28, display.width());
    int x = x_Start;
    int y = scale::y_resMtp(230, display.height());

    display.setFont(fontSmallDescription);
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(scale::x_resMtp(10, display.width()), scale::y_resMtp(180, display.height()));
    for (int weekday = 0; weekday < 7; weekday++)
    {
      drawCentreString(String(weekdayNumbersShort[weekday]).c_str(), x, scale::y_resMtp(200, display.height()));
      x += dx;
    }

    /*-- Tage im Kalender --*/
    initCalendarArray();
    x = x_Start;
    // bool marked_set = false;
    for (int column = 0; column < 5; column++)
    {
      for (int row = 0; row < 7; row++)
      {
        if (dayCalendar[row + 7 * column] == timeServer::getTimeStruct().tm_mday)
        {
          if ((column == 0) && (dayCalendar[row + 7 * column] > 24))
          {
          }
          else if ((column == 4) && (dayCalendar[row + 7 * column] < 6))
          {
          }
          else
          {
            display.setTextColor(GxEPD_WHITE);
            display.fillRoundRect(x - scale::x_resMtp(7, display.width()), y - scale::y_resMtp(15, display.height()), scale::x_resMtp(20, display.width()), scale::y_resMtp(20, display.height()), 5, GxEPD_RED);
          }
        }
        else
        {
          display.setTextColor(GxEPD_BLACK);
        }

        drawCentreString(String(dayCalendar[row + 7 * column]).c_str(), x, y);
        x += dx;
      }
      y += dy;
      x = x_Start;
    }

    /*-------------------------------------------------------------------------------------
    -- Taskübersicht
    -------------------------------------------------------------------------------------*/
    /*-- Titel Tasksübersicht --*/
    display.fillRoundRect(scale::x_resMtp(250, display.width()), scale::y_resMtp(5, display.height()), scale::x_resMtp(390, display.width()), scale::y_resMtp(55, display.height()), 10, GxEPD_BLACK);
    display.setTextColor(GxEPD_WHITE);
    display.setCursor(scale::x_resMtp(265, display.width()), scale::y_resMtp(45, display.height()));
    display.setFont(fontTitle);
    display.print("Tasks");

    /*-- Event --*/
    x = scale::x_resMtp(260, display.width());
    y = scale::y_resMtp(105, display.height()); // Set position for the first calendar entry

    for (int i = 0; i < GSR.getEntryCount(); i++)
    {
      if(GSR.getTask(i) != "") {
        // Print Event Title
        display.setCursor(x, y);
        display.setTextColor(GxEPD_BLACK);
        display.setFont(fontDescription);
        display.print(GSR.getTask(i));
        }

        // Print Event Date
        if(GSR.getDay(i) != "") {
          display.fillRect(scale::x_resMtp(570, display.width()), y - scale::y_resMtp(15, display.height()), scale::x_resMtp(70, display.width()), scale::y_resMtp(25, display.height()), GxEPD_BLACK);
          display.setCursor(scale::x_resMtp(580, display.width()), y + scale::y_resMtp(5, display.height()));
          display.setFont(fontSmallDescription);
          display.setTextColor(GxEPD_WHITE);
          String TaskDate = GSR.getDay(i) + " " + monthNumbersShort[(GSR.getMonth(i)).toInt()];
          display.print(TaskDate);

          // Heutiges Event markieren
          if (timeServer::getTimeStruct().tm_mday == (GSR.getDay(i)).toInt())
          {
            display.fillRect(scale::x_resMtp(570, display.width()), y - scale::y_resMtp(35, display.height()), scale::x_resMtp(70, display.width()), scale::y_resMtp(25, display.height()), GxEPD_RED);
            display.setCursor(scale::x_resMtp(580, display.width()), y - scale::y_resMtp(15, display.height()));
            display.setFont(fontSmallDescription);
            display.setTextColor(GxEPD_WHITE);
            display.print("Heute");
          }

          // Morgiges Event markieren
          if ((GSR.getTask(i) == "Hauskehricht") || (GSR.getTask(i) == "Altpapiersammlung"))
          {
            if ((timeServer::getTimeStruct().tm_mday == ((GSR.getDay(i)).toInt() - 1)) || ((timeServer::getTimeStruct().tm_mday == daysOfMonth[timeServer::getTimeStruct().tm_mon]) && (GSR.getDay(i) == "1")))
            {
              display.fillRect(scale::x_resMtp(570, display.width()), y - scale::y_resMtp(35, display.height()), scale::x_resMtp(70, display.width()), scale::y_resMtp(25, display.height()), GxEPD_RED);
              display.setCursor(scale::x_resMtp(575, display.width()), y - scale::y_resMtp(15, display.height()));
              display.setFont(fontSmallDescription);
              display.setTextColor(GxEPD_WHITE);
              display.print("Morgen");
            }
          }
          
        }
        // Add Line between Tasks
        display.fillRect(x, y + scale::y_resMtp(10, display.height()), scale::x_resMtp(380, display.width()), scale::y_resMtp(2, display.height()), GxEPD_BLACK);
        
      // Prepare y-position for next event entry
      y = y + scale::y_resMtp(50, display.height());
    }

  } while (display.nextPage());

 return true;
}

bool displaytasks()
{
  /*-----------------------------------
  -- Init E-Paper
  -----------------------------------*/
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
  
  /*-----------------------------------
  -- Draw E-Paper
  -----------------------------------*/
  display.firstPage();
  do
  {
    /*-------------------------------------------------------------------------------------
    -- Kalenderübersicht
    -------------------------------------------------------------------------------------*/
    /*-- Roter Rand --*/
    //display.fillRoundRect(scale::x_resMtp(0,display.width()), 0, scale::x_resMtp(640,display.width()), scale::y_resMtp(65, display.height()), 10, GxEPD_RED);
    display.fillRect(scale::x_resMtp(0,display.width()), 0, scale::x_resMtp(640,display.width()), scale::y_resMtp(60, display.height()), GxEPD_BLACK);
    
    /*-- Titel --*/
    display.setFont(fontTitle);
    display.setTextColor(GxEPD_WHITE);
    //drawCentreString("Tasks", scale::x_resMtp(40,display.width()), scale::y_resMtp(52, display.height())); //640x384

    /*-- Tag --*/
    display.setFont(fontMassiveTitle);
    display.setTextColor(GxEPD_WHITE);
    drawCentreString(String(timeServer::getTimeStruct().tm_mday).c_str(), scale::x_resMtp(560,display.width()), scale::y_resMtp(44, display.height())); //640x384

    /*-- Monat --*/
    display.setFont(fontDescription);
    drawCentreString(monthNumbersShort[timeServer::getTimeStruct().tm_mon], scale::x_resMtp(610,display.width()), scale::y_resMtp(27, display.height()));

    /*-- Jahr --*/
    display.setFont(fontSmallDescription);
    String year = "";
    year += (timeServer::getTimeStruct().tm_year + 1900);
    drawCentreString((year).c_str(), scale::x_resMtp(610,display.width()), scale::y_resMtp(47, display.height()));

    /*-- Wochentag --*/
    display.setFont(fontTitle);
    display.setTextColor(GxEPD_WHITE);
    //drawCentreString(weekdayNumbers[timeServer::getTimeStruct().tm_wday], scale::x_resMtp(450,display.width()), scale::y_resMtp(55, display.height()));
    //drawCentreString(weekdayNumbers[timeServer::getTimeStruct().tm_wday], scale::x_resMtp(80,display.width()), scale::y_resMtp(52, display.height()));
    display.setCursor(7, 42);
    display.print(weekdayNumbers[timeServer::getTimeStruct().tm_wday]);

    /*-------------------------------------------------------------------------------------
    -- Taskübersicht
    -------------------------------------------------------------------------------------*/
    int x = 7; int y = 100;
    for (int i = 0; i < GSR.getEntryCount(); i++)
    {
      String task_description = GSR.getTask(i);
      if(task_description != "") {
        if(task_description.length() >= 45) 
        {
          String sub_str1 = task_description.substring(0, 45);
          String sub_str2 = task_description.substring(45, task_description.length());
          
          display.setCursor(x, y);
          display.setTextColor(GxEPD_BLACK);
          display.setFont(fontDescription);
          display.print(sub_str1);
          
          y = y + scale::y_resMtp(25, display.height());
          display.setCursor(x, y);
          display.print(sub_str2);
        } else {
          // Print Event Title
          display.setCursor(x, y);
          display.setTextColor(GxEPD_BLACK);
          display.setFont(fontDescription);
          display.print(GSR.getTask(i));
        }
        // Add Line between Tasks
        display.fillRect(x, y + scale::y_resMtp(10, display.height()), scale::x_resMtp(640,display.width()), scale::y_resMtp(2, display.height()), GxEPD_BLACK);
      }
      
      // Print Event Date
      if( GSR.getDay(i) != "") {
        display.fillRect(scale::x_resMtp(570,display.width()), y - scale::y_resMtp(15, display.height()), scale::x_resMtp(70,display.width()), scale::y_resMtp(25, display.height()), GxEPD_BLACK);
        display.setCursor(scale::x_resMtp(580,display.width()), y + scale::y_resMtp(5, display.height()));
        display.setFont(fontSmallDescription);
        display.setTextColor(GxEPD_WHITE);
        String TaskDate = GSR.getDay(i) + " " + monthNumbersShort[(GSR.getMonth(i)).toInt()];
        display.print(TaskDate);

        // Heutiges Event markieren
        if (timeServer::getTimeStruct().tm_mday == (GSR.getDay(i)).toInt())
        {
          display.fillRect(scale::x_resMtp(570,display.width()), y - scale::y_resMtp(35, display.height()), scale::x_resMtp(70,display.width()), scale::y_resMtp(25, display.height()), GxEPD_RED);
          display.setCursor(scale::x_resMtp(580,display.width()), y - scale::y_resMtp(15, display.height()));
          display.setFont(fontSmallDescription);
          display.setTextColor(GxEPD_WHITE);
          display.print("Heute");
        }

        // Morgiges Event markieren
        if ((GSR.getTask(i) == "Hauskehricht") || (GSR.getTask(i) == "Altpapiersammlung"))
        {
          if ((timeServer::getTimeStruct().tm_mday == ((GSR.getDay(i)).toInt() - 1)) || ((timeServer::getTimeStruct().tm_mday == daysOfMonth[timeServer::getTimeStruct().tm_mon]) && (GSR.getDay(i) == "1")))
          {
            display.fillRect(scale::x_resMtp(570,display.width()), y - scale::y_resMtp(35, display.height()), scale::x_resMtp(70,display.width()), scale::y_resMtp(25, display.height()), GxEPD_RED);
            display.setCursor(scale::x_resMtp(575,display.width()), y - scale::y_resMtp(15, display.height()));
            display.setFont(fontSmallDescription);
            display.setTextColor(GxEPD_WHITE);
            display.print("Morgen");
          }
        }
      }
      
      
      // Prepare y-position for next event entry
      y = y + scale::y_resMtp(50, display.height());
    }

  } while (display.nextPage());

  return true;
}

void showNoInternetManual() {
  /*-----------------------------------
  -- Init E-Paper
  -----------------------------------*/
  // Turn off text-wrapping
  display.setTextWrap(false);
  display.setRotation(calendarOrientation);
  // Clear the screen with white using full window mode. Not strictly nessecary, but as I selected to use partial window for the content, I decided to do a full refresh first.
  display.setFullWindow();
  display.firstPage();
  
  // Print the content on the screen - I use a partial window refresh for the entire width and height, as I find this makes a clearer picture
  display.setPartialWindow(0, 0, display.width(), display.height());

  /*-----------------------------------
  -- Draw E-Paper
  -----------------------------------*/
  display.firstPage();
  do
  {
    
      display.setCursor(scale::x_resMtp(60, display.width()),  scale::y_resMtp(30, display.height()));
      display.setTextColor(GxEPD_RED);
      display.setFont(fontTitle);
      display.print("Ke Internetvrbindig vorhande...");
      display.setTextColor(GxEPD_BLACK);
      display.setFont(fontDescription);
      display.setCursor(scale::x_resMtp(10, display.width()),  scale::y_resMtp(90, display.height()));
      display.print("Wenn grad mau Zit hesch, drueck doch dr Blau Knopf uf dr ");
      display.setCursor(scale::x_resMtp(10, display.width()),  scale::y_resMtp(110, display.height()));
      display.print("Ruecksite vo dim E-Paper-Kalender. Sobaud de drueckt ");
      display.setCursor(scale::x_resMtp(10, display.width()),  scale::y_resMtp(130, display.height()));
      display.print("hesch, blibe dr 3 Minute, zum z Wifi ihzrichte.");
      display.setCursor(scale::x_resMtp(10, display.width()),  scale::y_resMtp(150, display.height()));
      display.print("Gang uf dim Smartphone, Tablet oder Laptop ");
      display.setCursor(scale::x_resMtp(10, display.width()),  scale::y_resMtp(170, display.height()));
      display.print("ih dini Wifi-Ihstellige, aus wettsch di mitemne Wifi ");
      display.setCursor(scale::x_resMtp(10, display.width()),  scale::y_resMtp(190, display.height()));
      display.print("vrbinde und suechsch nach <E-Paper-Calendar>. Mit ");
      display.setCursor(scale::x_resMtp(10, display.width()),  scale::y_resMtp(210, display.height()));
      display.print("dem vrbindisch di ize und de geit outomatisch e Site uf,");
      display.setCursor(scale::x_resMtp(10, display.width()),  scale::y_resMtp(230, display.height()));
      display.print("wo de uf <Configure WiFi> kliksch. Iz vrbindisch di ");
      display.setCursor(scale::x_resMtp(10, display.width()),  scale::y_resMtp(250, display.height()));
      display.print("mitem WiFi wo de ou für anderi Graet bruchsch, gisch z ");
      display.setCursor(scale::x_resMtp(10, display.width()),  scale::y_resMtp(270, display.height()));
      display.print("Passowort ih und de de het die E-Paper Kalender wieder");
      display.setCursor(scale::x_resMtp(10, display.width()),  scale::y_resMtp(290, display.height()));
      display.print("Internet. Vrgiss nid dr Switch <Position> uf dr ungersite ");
      display.setCursor(scale::x_resMtp(10, display.width()),  scale::y_resMtp(310, display.height()));
      display.print("vom E-Paper Kalender wieder uf OFF z schiebe!");
      display.setCursor(scale::x_resMtp(10, display.width()),  scale::y_resMtp(330, display.height()));
      display.print("");
      display.setCursor(scale::x_resMtp(10, display.width()),  scale::y_resMtp(350, display.height()));
      display.print("Fauses doch nid klappt: 078 794 02 54");

  } while (display.nextPage());
  
}

// Sleep until set wa§ke-hour
void print_wakeup_reason() {
    esp_sleep_wakeup_cause_t wakeup_reason;

    wakeup_reason = esp_sleep_get_wakeup_cause();

    switch (wakeup_reason)
    {
    case ESP_SLEEP_WAKEUP_EXT0: Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1: Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER: Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD: Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP: Serial.println("Wakeup caused by ULP program"); break;
    default: Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;
    }
}

// Sleep until set wake-hour
void deepSleepTill(int wakeHour)
{
  uint64_t hour_till_wakeup = 0;

  if(day_of_last_update == timeServer::getTimeStruct().tm_mday) {
    
    // Calendar was updated today
    if (timeServer::getTimeStruct().tm_hour < wakeHour)
    {
      hour_till_wakeup = 24 * S_TO_H_FACTOR * uS_TO_S_FACTOR;
    }
    else
    {
      hour_till_wakeup = ((24 - (timeServer::getTimeStruct().tm_hour+1)) + wakeHour) * S_TO_H_FACTOR * uS_TO_S_FACTOR;
    }

  } else {
    // Calendar was not updated today, wakeup in one hour
    hour_till_wakeup = S_TO_H_FACTOR* uS_TO_S_FACTOR;
    
  }
  // If battery is too low (see getBattery code), enter deepSleep and do not wake up
  //if (batterylevel == 0)
  //{
  //  esp_deep_sleep_start();
  //}
  
  Serial.print("Next wakeup in: ");
  Serial.print((hour_till_wakeup / S_TO_H_FACTOR ) / uS_TO_S_FACTOR);
  Serial.println("h");
	
  esp_sleep_enable_timer_wakeup(hour_till_wakeup);
  //esp_sleep_enable_timer_wakeup(60*60* uS_TO_S_FACTOR);
  Serial.flush();
  esp_deep_sleep_start();
}

void readChargingStatus(void)
{
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
// Not used, as we boot up from scratch every time we wake from deep sleep
void loop()
{
}