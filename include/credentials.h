/*

This is the main configuration file.

Wifi, the link to your google script, the optional script key and your location are configured in the WIFI hotspot
that boots up when you turn on the project for the first time - see the README. They are stored in the ESP32 flash (NVS).
The lines below are the settings that rarely change: timezone, layout, fonts and battery measurement.

*/

#include <Fonts/FreeSerifBold9pt7b.h>
#include <Fonts/FreeSerifBold12pt7b.h>
#include <Fonts/FreeSerifBold18pt7b.h>
#include <Fonts/FreeSerifBold24pt7b.h>

#include <Fonts/FreeSerif9pt7b.h>
#include <Fonts/FreeSerif12pt7b.h>
#include <Fonts/FreeSerif18pt7b.h>
#include <Fonts/FreeSerif24pt7b.h>

// Name of the WIFI hotspot used for setup, and the hostname of the calendar on your network
const char* HOSTNAME = "FamilyCalendar";

// Seconds the setup portal stays open after power-on/reset - after that the calendar goes to sleep
const int portalTimeoutTime = 60 * 5;

// Seconds to wait for the wifi when waking up from deep sleep
const int wifiTimeoutTime = 20;

// If wifi, the calendar or the time server fails, try again after this many minutes - at most failedRetryCount times before waiting for the next morning
const int failedRetryMinutes = 60;
const int failedRetryCount = 3;

// Get time - the timezone is a POSIX TZ string, which also handles daylight saving time. The default is Denmark/Central Europe.
// Find yours at https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
const char* timeZone  = "CET-1CEST,M3.5.0,M10.5.0/3";
const char* ntpServer = "pool.ntp.org";

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

// Battery measurement. On LOLIN D32 boards the battery is connected to pin 35 through two 100k resistors (R1 = R2 = 100).
// For other ESP32 boards, insert a voltage divider: battery+ -> R1 -> analogue pin -> R2 -> GND. Use an ADC1 pin (32-39), as ADC2 does not work with wifi on.
// Check the voltage printed on the serial monitor against a multimeter the first time you use it.
const uint8_t batteryPin = 34;
const float batteryR1 = 30;   // kOhm, between battery+ and the pin
const float batteryR2 = 100;  // kOhm, between the pin and GND
const float batteryFullVoltage = 4.1;   // Shown as 100%
const float batteryEmptyVoltage = 3.5;  // Shown as 0% - the calendar then stops waking up to protect the LIPO

// Daily greetings in hte top line - change "Happy" with something else for variation on your week :)
const char * weekday[] = { "Happy Sunday", "Happy Monday", "Happy Tuesday", "Happy Wednesday", "Happy Thursday", "Happy Friday", "Happy Saturday"};
