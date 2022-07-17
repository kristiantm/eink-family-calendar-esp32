/*

This is the main configuration file. 

I have moved all configuration to the WIFI hotspot that will boot up when you turn on the project for the first time. 
All basic configuration should be managed from there - thus disregard the lines below :)

*/

#include <Fonts/FreeSerifBold9pt7b.h>
#include <Fonts/FreeSerifBold12pt7b.h>
#include <Fonts/FreeSerifBold18pt7b.h>
#include <Fonts/FreeSerifBold24pt7b.h>

#include <Fonts/FreeSerif9pt7b.h>
#include <Fonts/FreeSerif12pt7b.h>
#include <Fonts/FreeSerif18pt7b.h>
#include <Fonts/FreeSerif24pt7b.h>

#include <Fonts/FreeMonoBold24pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>


/* 
    The following will be configured using the web-portal

    Use your own API key by signing up for a free developer account at https://openweathermap.org/
    Create a script on scripts.google.com to fetch your calendarrequests (as per guide on instructable)
    Find the lattitude and logitude for your location using google maps
*/
const char*  ssid = "Salt_2GHz_159505";
const char* password = "MuUY33L6SERtXG4CDT";

// Get time
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;

// Get calendar
char calendarServer[] = "script.google.com"; 
String calendarRequest = "https://script.google.com/macros/s/AKfycbyeL6o3SQeqXu2ki476podsct_IF9J_jOdaBeoyH8ORizb68IphjD7PmhzRVMbX5YKVxw/exec"; // This will eventually be an URL in the form of "https://script.google.com/macros/s/" + googleAPI + "/exec"

// Set the number of entries to show - default is 10 - note that spacing between entries can push some out of the viewable area
const int calEntryCount = 6;

// Set the fonts you would like to use
const GFXfont *fontMassiveTitle = &FreeSansBold24pt7b;
const GFXfont *fontTitle = &FreeSans18pt7b;
const GFXfont *fontDescription = &FreeSans12pt7b;
const GFXfont *fontSmallDescription = &FreeSans9pt7b;

// Set the coordinates for the calendar - the current configuration allows for 9 events to be shown
const int calendarSpacing = 50; // Space between each event entry

// Set orientation of calendar 0 = 0% horisontal, 1 = 90% vertical, 2 = 180% horisontal, 3 = 270% vertical
const int calendarOrientation = 0;

#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define HOUR_TO_WAKE  5        // Time ESP32 will wake and refresh at a specific hour each day - default is five in the morning

// Daily greetings in hte top line - change "Happy" with something else for variation on your week :)
const char * weekdayNumbersShort[] = { "Mo", "Di", "Mi", "Do", "Fr", "Sa", "So"};
const char * weekdayNumbers[] = { "Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"};
const char * monthNumbersShort[] = { "Jan", "Feb", "Mar", "Apr", "Mai", "Jun", "Jul", "Aug", "Sep", "Okt", "Nov", "Dez"};
const char * monthNumbers[] = { "Januar", "Februar", "März", "April", "Mai", "Juni", "Juli", "August", "September", "Oktober", "November", "Dezember"};

int dayCalender[7][5];
int convertTmWeekday[] = {6,0,1,2,3,4,5};
int daysOfMonth[] = {31,28,31,30,31,30,31,31,30,31,30,31};


