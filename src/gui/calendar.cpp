// #include "calendar.hpp"

// #include "scale.hpp"

// void calendar::updateCalendar(void)
// {
//   struct tm timeinfo;
//   if (!getLocalTime(&timeinfo))
//   {
//     Serial.println("Failed to obtain time");
//   }

//   // Berechne Tag vom ersten Sonntag im Monat

//   int date_of_first_sunnday = 0;
//   int rest_day = 0;
//   int week_cnt = 0;
//   if (timeinfo.tm_mday <= convertTmWeekday[timeinfo.tm_wday] + 1)
//   {
//     date_of_first_sunnday = timeinfo.tm_mday + (6 - convertTmWeekday[timeinfo.tm_wday]);
//     Serial.print("date_of_first_sunnday = ");
//     Serial.print("timeinfo.tm_mday + (6 - convertTmWeekday[timeinfo.tm_wday = ");
//     Serial.println(timeinfo.tm_mday + (6 - convertTmWeekday[timeinfo.tm_wday]));
//   }
//   else
//   {
//     int date_of_sunday_before = timeinfo.tm_mday - (convertTmWeekday[timeinfo.tm_wday] + 1);
//     Serial.print("date_of_sunday_before = ");
//     Serial.print("timeinfo.tm_mday - (convertTmWeekday[timeinfo.tm_wday] + 1");
//     Serial.println(timeinfo.tm_mday - (convertTmWeekday[timeinfo.tm_wday] + 1));

//     while ((date_of_sunday_before) > 6)
//     {
//       date_of_sunday_before -= 7;
//       // week_cnt++;
//     }
//     date_of_first_sunnday = date_of_sunday_before;
//     // Serial.print("week_cnt: ");
//     // Serial.println(week_cnt);
//     Serial.print("date_of_first_sunnday: ");
//     Serial.println(date_of_first_sunnday);
//   }

//   rest_day = 7 - date_of_first_sunnday;
//   Serial.print("rest_day: ");
//   Serial.println(rest_day);

//   int calendar_week_cnt = 0;
//   if (rest_day != 0)
//   {
//     for (int mday = 0; mday < rest_day; mday++)
//     {
//       dayCalendar[mday + calendar_week_cnt] = ((daysOfMonth[timeinfo.tm_mon - 1] - rest_day) + 1 + mday);
//       Serial.print("[");
//       Serial.print(mday + calendar_week_cnt);
//       Serial.print("] = ");
//       Serial.println(((daysOfMonth[timeinfo.tm_mon - 1] - rest_day) + 1 + mday));
//     }

//     int inc_day_cnt = 1;
//     for (int mday = rest_day; mday < 7; mday++)
//     {
//       dayCalendar[mday + calendar_week_cnt] = inc_day_cnt;
//       Serial.print("[");
//       Serial.print(mday + calendar_week_cnt);
//       Serial.print("] = ");
//       Serial.println(inc_day_cnt);
//       inc_day_cnt++;
//     }
//     calendar_week_cnt = 7;
//   }
//   else
//   {
//     calendar_week_cnt = 0;
//   }

//   for (int mday = date_of_first_sunnday + 1; mday <= daysOfMonth[timeinfo.tm_mon]; mday++)
//   {
//     dayCalendar[calendar_week_cnt] = mday;
//     Serial.print("[");
//     Serial.print(calendar_week_cnt);
//     Serial.print("] = ");
//     Serial.println(mday);
//     calendar_week_cnt++;
//   }
//   rest_day = 35 - calendar_week_cnt;
//   for (int mday = 1; mday <= rest_day; mday++)
//   {
//     dayCalendar[calendar_week_cnt] = mday;
//     Serial.print("[");
//     Serial.print(calendar_week_cnt);
//     Serial.print("] = ");
//     Serial.println(mday);
//     calendar_week_cnt++;
//   }
// }

// void calendar::draw(googleScriptRequest gsr)
// {
//   /*-----------------------------------
//   -- Get a Timestamp
//   -----------------------------------*/
//   struct tm timeinfo;
//   if (!getLocalTime(&timeinfo))
//   {
//     Serial.println("Failed to obtain time");
//   }

//   /*-----------------------------------
//   -- Init E-Paper
//   -----------------------------------*/
//   // Turn off text-wrapping
//   display.setTextWrap(false);
//   display.setRotation(calendarOrientation);
//   // Clear the screen with white using full window mode. Not strictly nessecary, but as I selected to use partial window for the content, I decided to do a full refresh first.
//   display.setFullWindow();
//   display.firstPage();
//   do
//   {
//     display.fillScreen(GxEPD_WHITE);
//   } while (display.nextPage());
//   // Print the content on the screen - I use a partial window refresh for the entire width and height, as I find this makes a clearer picture
//   display.setPartialWindow(0, 0, display.width(), display.height());

//   /*-----------------------------------
//   -- Draw E-Paper
//   -----------------------------------*/
//   display.firstPage();
//   do
//   {
//     // Print mini-test in top in white (e.g. not visible) - avoids a graphical glitch I observed in all first-lines printed
//     // display.setCursor(0, 384); //640x384
//     // display.setTextColor(GxEPD_BLACK);
//     // display.setFont(fontDescription);
//     // display.print("Graphical Flitch");

//     /*-------------------------------------------------------------------------------------
//     -- Kalenderübersicht
//     -------------------------------------------------------------------------------------*/
//     /*-- Titel Kalenderübersicht --*/
//     display.fillRoundRect(scale::x_resMtp(0, display.width()), scale::y_resMtp(0, display.height()), scale::x_resMtp(240, display.width()), scale::y_resMtp(170, display.height()), 10, GxEPD_BLACK);
//     display.setTextColor(GxEPD_WHITE);

//     struct tm timeinfo;
//     if (!getLocalTime(&timeinfo))
//     {
//       Serial.println("Failed to obtain time");
//     }

//     /*-- Wochentag --*/
//     display.setFont(fontTitle);
//     drawCentreString(weekdayNumbers[timeinfo.tm_wday], scale::x_resMtp(120, display.width()), scale::y_resMtp(50, display.height()));

//     /*-- Tag --*/
//     display.setFont(fontMassiveTitle);
//     drawCentreString(String(timeinfo.tm_mday).c_str(), scale::x_resMtp(120, display.width()), scale::y_resMtp(105, display.height()));

//     /*-- Monat + Jahr --*/
//     display.setFont(fontDescription);
//     String monthYear = monthNumbers[timeinfo.tm_mon];
//     monthYear += " ";
//     monthYear += (timeinfo.tm_year + 1900);
//     drawCentreString((monthYear).c_str(), scale::x_resMtp(120, display.width()), scale::y_resMtp(150, display.height()));

//     /*-- Wochentage im Kalender --*/
//     const int dx = scale::x_resMtp(30, display.width());
//     const int dy = scale::y_resMtp(32, display.height());
//     const int x_Start = scale::x_resMtp(28, display.width());
//     int x = x_Start;
//     int y = scale::y_resMtp(230, display.height());

//     display.setFont(fontSmallDescription);
//     display.setTextColor(GxEPD_BLACK);
//     display.setCursor(scale::x_resMtp(10, display.width()), scale::y_resMtp(180, display.height()));
//     for (int weekday = 0; weekday < 7; weekday++)
//     {
//       drawCentreString(String(weekdayNumbersShort[weekday]).c_str(), x, scale::y_resMtp(200, display.height()));
//       x += dx;
//     }

//     /*-- Tage im Kalender --*/
//     updateCalendar();
//     x = x_Start;
//     // bool marked_set = false;
//     for (int column = 0; column < 5; column++)
//     {
//       for (int row = 0; row < 7; row++)
//       {
//         if (dayCalendar[row + 7 * column] == timeinfo.tm_mday)
//         {
//           if ((column == 0) && (dayCalendar[row + 7 * column] > 24))
//           {
//           }
//           else if ((column == 4) && (dayCalendar[row + 7 * column] < 6))
//           {
//           }
//           else
//           {
//             display.setTextColor(GxEPD_WHITE);
//             display.fillRoundRect(x - scale::x_resMtp(7, display.width()), y - scale::y_resMtp(15, display.height()), scale::x_resMtp(20, display.width()), scale::y_resMtp(20, display.height()), 5, GxEPD_RED);
//           }
//         }
//         else
//         {
//           display.setTextColor(GxEPD_BLACK);
//         }

//         drawCentreString(String(dayCalendar[row + 7 * column]).c_str(), x, y);
//         x += dx;
//       }
//       y += dy;
//       x = x_Start;
//     }

//     /*-------------------------------------------------------------------------------------
//     -- Taskübersicht
//     -------------------------------------------------------------------------------------*/
//     /*-- Titel Tasksübersicht --*/
//     display.fillRoundRect(scale::x_resMtp(250, display.width()), scale::y_resMtp(5, display.height()), scale::x_resMtp(390, display.width()), scale::y_resMtp(55, display.height()), 10, GxEPD_BLACK);
//     display.setTextColor(GxEPD_WHITE);
//     display.setCursor(scale::x_resMtp(265, display.width()), scale::y_resMtp(45, display.height()));
//     display.setFont(fontTitle);
//     display.print("Tasks");

//     /*-- Event --*/
//     x = scale::x_resMtp(260, display.width());
//     y = scale::y_resMtp(105, display.height()); // Set position for the first calendar entry

//     for (int i = 0; i < gsr.getEntryCount(); i++)
//     {
//       // Print Event Title
//       display.setCursor(x, y);
//       display.setTextColor(GxEPD_BLACK);
//       display.setFont(fontDescription);
//       display.print(gsr.getTask(i));

//       // Print Event Date
//       display.fillRect(scale::x_resMtp(570, display.width()), y - scale::y_resMtp(15, display.height()), scale::x_resMtp(70, display.width()), scale::y_resMtp(25, display.height()), GxEPD_BLACK);
//       display.setCursor(scale::x_resMtp(580, display.width()), y + scale::y_resMtp(5, display.height()));
//       display.setFont(fontSmallDescription);
//       display.setTextColor(GxEPD_WHITE);
//       String TaskDate = gsr.getDay(i) + " " + monthNumbersShort[(gsr.getMonth(i)).toInt()];
//       display.print(TaskDate);

//       // Heutiges Event markieren
//       if (timeinfo.tm_mday == (gsr.getDay(i)).toInt())
//       {
//         display.fillRect(scale::x_resMtp(570, display.width()), y - scale::y_resMtp(35, display.height()), scale::x_resMtp(70, display.width()), scale::y_resMtp(25, display.height()), GxEPD_RED);
//         display.setCursor(scale::x_resMtp(580, display.width()), y - scale::y_resMtp(15, display.height()));
//         display.setFont(fontSmallDescription);
//         display.setTextColor(GxEPD_WHITE);
//         display.print("Heute");
//       }

//       // Morgiges Event markieren
//       if ((gsr.getTask(i) == "Hauskehricht") || (gsr.getTask(i) == "Altpapiersammlung"))
//       {
//         if ((timeinfo.tm_mday == ((gsr.getDay(i)).toInt() - 1)) || ((timeinfo.tm_mday == daysOfMonth[timeinfo.tm_mon]) && (gsr.getDay(i) == "1")))
//         {
//           display.fillRect(scale::x_resMtp(570, display.width()), y - scale::y_resMtp(35, display.height()), scale::x_resMtp(70, display.width()), scale::y_resMtp(25, display.height()), GxEPD_RED);
//           display.setCursor(scale::x_resMtp(575, display.width()), y - scale::y_resMtp(15, display.height()));
//           display.setFont(fontSmallDescription);
//           display.setTextColor(GxEPD_WHITE);
//           display.print("Morgen");
//         }
//       }
//       // Add Line between Tasks
//       display.fillRect(x, y + scale::y_resMtp(10, display.height()), scale::x_resMtp(380, display.width()), scale::y_resMtp(2, display.height()), GxEPD_BLACK);

//       // Prepare y-position for next event entry
//       y = y + scale::y_resMtp(50, display.height());
//     }

//   } while (display.nextPage());

//   // return true;
// }