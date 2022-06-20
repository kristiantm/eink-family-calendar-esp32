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

/* 
    The following will be configured using the web-portal

    Use your own API key by signing up for a free developer account at https://openweathermap.org/
    Create a script on scripts.google.com to fetch your calendarrequests (as per guide on instructable)
    Find the lattitude and logitude for your location using google maps
*/
String OWMapikey       = "Write OWM api here";                   // See: https://openweathermap.org/ //FIXME
String googleAPI = "Write Google script api here";
String Lattitude          = "Lattitude";                      // Your lattitude - use google maps //FIXME
String Longitude          = "Longitude";                      // Your longitude - use google maps //FIXME

char OWMserver[] = "api.openweathermap.org";

//Set your location according to OWM locations
String Language      = "EN";                            // NOTE: Only the weather description (not used) is translated by OWM
String Units         = "M";                             // Use 'M' for Metric or I for Imperial 

// Get time
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;

// Get calendar
char calendarServer[] = "script.google.com"; 
String calendarRequest = ""; // This will eventually be an URL in the form of "https://script.google.com/macros/s/" + googleAPI + "/exec"

// Set the number of entries to show - default is 10 - note that spacing between entries can push some out of the viewable area
const int calEntryCount = 10;

// Set the fonts you would like to use
const GFXfont *fontMainTitle = &FreeSerifBold24pt7b;
const GFXfont *fontEntryTime = &FreeSerif12pt7b;
const GFXfont *fontEntryTitle = &FreeSerif18pt7b;
const GFXfont *fontWeather = &FreeSerif9pt7b;

// Set the coordinates for the calendar - the current configuration allows for 9 events to be shown
const int calendarPosX = 45; // Space from the left side of the e-ink screen
const int calendarPosY = 78; // Space from the top of the e-ink-screen
const int calendarSpacing = 70; // Space between each event entry

// Set the coordinates for the weather icon and temp+wind info - right now it is placed in the upper right corner
const int weatherPosX = 380;
const int weatherPosY = 10;

// Set orientation of calendar 0 = 0% horisontal, 1 = 90% vertical, 2 = 180% horisontal, 3 = 270% vertical
const int calendarOrientation = 1;

#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define HOUR_TO_WAKE  5        // Time ESP32 will wake and refresh at a specific hour each day - default is five in the morning

// Daily greetings in hte top line - change "Happy" with something else for variation on your week :)
const char * weekday[] = { "Happy Sunday", "Happy Monday", "Happy Tuesday", "Happy Wednesday", "Happy Thursday", "Happy Friday", "Happy Saturday"};
