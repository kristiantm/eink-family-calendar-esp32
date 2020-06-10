/*

This is the main configuration file. 

Make sure you 
1) change your WIFI, 
2) add your own OWM API code, 
3) set your location info for weather, and 
4) set the secret path to your google script that pulls the calendar entries.

Otherwise the project will not work :)

*/

#include <Fonts/FreeSerifBold9pt7b.h>
#include <Fonts/FreeSerifBold12pt7b.h>
#include <Fonts/FreeSerifBold18pt7b.h>
#include <Fonts/FreeSerifBold24pt7b.h>

#include <Fonts/FreeSerif9pt7b.h>
#include <Fonts/FreeSerif12pt7b.h>
#include <Fonts/FreeSerif18pt7b.h>
#include <Fonts/FreeSerif24pt7b.h>

// Change to your WiFi credentials
const char* ssid1     = "[YOUR_NETWORK_SSID]";      // For your network
const char* password1 = "[YOUR_NETWORK_KEY]";  // For your network

// Use your own API key by signing up for a free developer account at https://openweathermap.org/
String OWMapikey       = "[YOUR OPEN WEATHERMAPS API]";                   // See: https://openweathermap.org/
char OWMserver[] = "api.openweathermap.org";

//Set your location according to OWM locations
String City          = "Copenhagen";                      // Your home city See: http://bulk.openweathermap.org/sample/
String Lattitude          = "[YOUR_LATTITUDE]";                      // Your lattitude - use google maps
String Longitude          = "[YOUR_LONGITUDE]";                      // Your longitude - use google maps
String Country       = "Denmark";                            // Your country  
String Language      = "EN";                            // NOTE: Only the weather description (not used) is translated by OWM
                                                        // Arabic (AR) Czech (CZ) English (EN) Greek (EL) Persian(Farsi) (FA) Galician (GL) Hungarian (HU) Japanese (JA)
                                                        // Korean (KR) Latvian (LA) Lithuanian (LT) Macedonian (MK) Slovak (SK) Slovenian (SL) Vietnamese (VI)
String Hemisphere    = "north";                         // or "south"  
String Units         = "M";                             // Use 'M' for Metric or I for Imperial 
const char* Timezone = "CET-1CEST,M3.5.0,M10.5.0/3";  // Choose your time zone from: https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv 

// Get time
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;

// Get calendar
char calendarServer[] = "script.google.com"; 
String calendarRequest = "https://script.google.com/macros/s/[YOUR_UNIQUE_ID]/exec"; // Write the path for your google script to fetch calendar events

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
