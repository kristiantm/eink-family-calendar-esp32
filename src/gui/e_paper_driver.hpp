 #ifndef E_PAPER_DRIVER_HPP
 #define E_PAPER_DRIVER_HPP
 #include "Arduino.h"



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
 
 //#include <Fonts/FreeSansBold24pt7b.h>
 //#include <Fonts/FreeSans18pt7b.h>
 //#include <Fonts/FreeSans12pt7b.h>
 //#include <Fonts/FreeSans9pt7b.h>



 
#define ENABLE_GxEPD2_GFX 0
#include <GxEPD2_3C.h>

#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

 class e_paper_driver : GxEPD2
 {
   public:
      e_paper_driver();
   private:
      // Set orientation of calendar 0 = 0% horisontal, 1 = 90% vertical, 2 = 180% horisontal, 3 = 270% vertical
      static const int calendarOrientation = 0;
   
      // Set fonts
      const GFXfont *fontMassiveTitle = &FreeSansBold24pt7b;
      const GFXfont *fontTitle = &FreeSans18pt7b;
      const GFXfont *fontDescription = &FreeSans12pt7b;
      const GFXfont *fontSmallDescription = &FreeSans9pt7b;
 };

 #endif