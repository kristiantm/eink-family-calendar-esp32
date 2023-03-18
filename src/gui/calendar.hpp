// #ifndef CALENDAR_HPP
// #define CALENDAR_HPP
// #include "Arduino.h"
// #include "googleScriptRequest.hpp"

// #include <Fonts/FreeSerifBold9pt7b.h>
// #include <Fonts/FreeSerifBold12pt7b.h>
// #include <Fonts/FreeSerifBold18pt7b.h>
// #include <Fonts/FreeSerifBold24pt7b.h>

// #include <Fonts/FreeSerif9pt7b.h>
// #include <Fonts/FreeSerif12pt7b.h>
// #include <Fonts/FreeSerif18pt7b.h>
// #include <Fonts/FreeSerif24pt7b.h>

// #include <Fonts/FreeMonoBold24pt7b.h>
// #include <Fonts/FreeMonoBold12pt7b.h>
// #include <Fonts/FreeMonoBold9pt7b.h>
// #include <Fonts/FreeSansBold24pt7b.h>
// #include <Fonts/FreeSans18pt7b.h>
// #include <Fonts/FreeSans12pt7b.h>
// #include <Fonts/FreeSans9pt7b.h>

// class calendar
// {
// public:
//     void draw(googleScriptRequest gsr);
//     void updateCalendar(void);

// private:
//     int dayCalendar[7 * 5];

//     // Set fonts
//     const GFXfont *fontMassiveTitle = &FreeSansBold24pt7b;
//     const GFXfont *fontTitle = &FreeSans18pt7b;
//     const GFXfont *fontDescription = &FreeSans12pt7b;
//     const GFXfont *fontSmallDescription = &FreeSans9pt7b;

//     // Set orientation of calendar 0 = 0% horisontal, 1 = 90% vertical, 2 = 180% horisontal, 3 = 270% vertical
//     const int calendarOrientation = 0;

// #define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */
// #define HOUR_TO_WAKE 1            // Time ESP32 will wake and refresh at a specific hour each day - default is five in the morning

//     // Daily greetings in hte top line - change "Happy" with something else for variation on your week :)
//     const char *weekdayNumbersShort[] = {"Mo", "Di", "Mi", "Do", "Fr", "Sa", "So"};
//     const char *weekdayNumbers[] = {"Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"};
//     const char *monthNumbersShort[] = {"Jan", "Feb", "Mar", "Apr", "Mai", "Jun", "Jul", "Aug", "Sep", "Okt", "Nov", "Dez"};
//     const char *monthNumbers[] = {"Januar", "Februar", "März", "April", "Mai", "Juni", "Juli", "August", "September", "Oktober", "November", "Dezember"};

//     const int convertTmWeekday[] = {6, 0, 1, 2, 3, 4, 5};
//     const int daysOfMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
// }

// #endif