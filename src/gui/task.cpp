// #include "task.hpp"
// //void task::updateCalendar(void) {
    
// //}

// //void task::draw(googleScriptRequest gsr) {

// //}
// void task::drawCentreString(const char *buf, int x, int y)
// {
//   int16_t x1, y1;
//   uint16_t w, h;
//   display.getTextBounds(buf, x, y, &x1, &y1, &w, &h); // calc width of new string
//   display.setCursor(x - w / 2, y);
//   display.print(buf);
// }

// void task::draw(GxEPD2_3C<GxEPD2_750c, GxEPD2_750c::HEIGHT> &display) {
  
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
//     /*-------------------------------------------------------------------------------------
//     -- Kalenderübersicht
//     -------------------------------------------------------------------------------------*/
//     /*-- Roter Rand --*/
//     //display.fillRoundRect(scale::x_resMtp(0,display.width()), 0, scale::x_resMtp(640,display.width()), scale::y_resMtp(65, display.height()), 10, GxEPD_RED);
//     display.fillRect(scale::x_resMtp(0,display.width()), 0, scale::x_resMtp(640,display.width()), scale::y_resMtp(60, display.height()), GxEPD_BLACK);
    
//     /*-- Titel --*/
//     display.setFont(fontTitle);
//     display.setTextColor(GxEPD_WHITE);
//     //drawCentreString("Tasks", scale::x_resMtp(40,display.width()), scale::y_resMtp(52, display.height())); //640x384

//     /*-- Tag --*/
//     display.setFont(fontMassiveTitle);
//     display.setTextColor(GxEPD_WHITE);
//     drawCentreString(String(timeServer::getTimeStruct().tm_mday).c_str(), scale::x_resMtp(560,display.width()), scale::y_resMtp(44, display.height())); //640x384

//     /*-- Monat --*/
//     display.setFont(fontDescription);
//     drawCentreString(monthNumbersShort[timeServer::getTimeStruct().tm_mon], scale::x_resMtp(610,display.width()), scale::y_resMtp(27, display.height()));

//     /*-- Jahr --*/
//     display.setFont(fontSmallDescription);
//     String year = "";
//     year += (timeServer::getTimeStruct().tm_year + 1900);
//     drawCentreString((year).c_str(), scale::x_resMtp(610,display.width()), scale::y_resMtp(47, display.height()));

//     /*-- Wochentag --*/
//     display.setFont(fontTitle);
//     display.setTextColor(GxEPD_WHITE);
//     //drawCentreString(weekdayNumbers[timeServer::getTimeStruct().tm_wday], scale::x_resMtp(450,display.width()), scale::y_resMtp(55, display.height()));
//     //drawCentreString(weekdayNumbers[timeServer::getTimeStruct().tm_wday], scale::x_resMtp(80,display.width()), scale::y_resMtp(52, display.height()));
//     display.setCursor(7, 42);
//     display.print(weekdayNumbers[timeServer::getTimeStruct().tm_wday]);

//     /*-------------------------------------------------------------------------------------
//     -- Taskübersicht
//     -------------------------------------------------------------------------------------*/
//     int x = 7; int y = 100;
//     for (int i = 0; i < GSR.getEntryCount(); i++)
//     {
//       String task_description = GSR.getTask(i);
//       if(task_description.length() >= 45) { //45
//         String sub_str1 = task_description.substring(0, 45);
//         String sub_str2 = task_description.substring(45, task_description.length());

        
//         display.setCursor(x, y);
//         display.setTextColor(GxEPD_BLACK);
//         display.setFont(fontDescription);
//         display.print(sub_str1);
        
//         y = y + scale::y_resMtp(25, display.height());
//         display.setCursor(x, y);
//         display.print(sub_str2);
//       } else {
//           // Print Event Title
//           display.setCursor(x, y);
//           display.setTextColor(GxEPD_BLACK);
//           display.setFont(fontDescription);
//           display.print(GSR.getTask(i));
//       }

      

//       // Print Event Date
//       display.fillRect(scale::x_resMtp(570,display.width()), y - scale::y_resMtp(15, display.height()), scale::x_resMtp(70,display.width()), scale::y_resMtp(25, display.height()), GxEPD_BLACK);
//       display.setCursor(scale::x_resMtp(580,display.width()), y + scale::y_resMtp(5, display.height()));
//       display.setFont(fontSmallDescription);
//       display.setTextColor(GxEPD_WHITE);
//       String TaskDate = GSR.getDay(i) + " " + monthNumbersShort[(GSR.getMonth(i)).toInt()];
//       display.print(TaskDate);

//       // Heutiges Event markieren
//       if (timeServer::getTimeStruct().tm_mday == (GSR.getDay(i)).toInt())
//       {
//         display.fillRect(scale::x_resMtp(570,display.width()), y - scale::y_resMtp(35, display.height()), scale::x_resMtp(70,display.width()), scale::y_resMtp(25, display.height()), GxEPD_RED);
//         display.setCursor(scale::x_resMtp(580,display.width()), y - scale::y_resMtp(15, display.height()));
//         display.setFont(fontSmallDescription);
//         display.setTextColor(GxEPD_WHITE);
//         display.print("Heute");
//       }

//       // Morgiges Event markieren
//       if ((GSR.getTask(i) == "Hauskehricht") || (GSR.getTask(i) == "Altpapiersammlung"))
//       {
//         if ((timeServer::getTimeStruct().tm_mday == ((GSR.getDay(i)).toInt() - 1)) || ((timeServer::getTimeStruct().tm_mday == daysOfMonth[timeServer::getTimeStruct().tm_mon]) && (GSR.getDay(i) == "1")))
//         {
//           display.fillRect(scale::x_resMtp(570,display.width()), y - scale::y_resMtp(35, display.height()), scale::x_resMtp(70,display.width()), scale::y_resMtp(25, display.height()), GxEPD_RED);
//           display.setCursor(scale::x_resMtp(575,display.width()), y - scale::y_resMtp(15, display.height()));
//           display.setFont(fontSmallDescription);
//           display.setTextColor(GxEPD_WHITE);
//           display.print("Morgen");
//         }
//       }
//       // Add Line between Tasks
//       display.fillRect(x, y + scale::y_resMtp(10, display.height()), scale::x_resMtp(640,display.width()), scale::y_resMtp(2, display.height()), GxEPD_BLACK);

//       // Prepare y-position for next event entry
//       y = y + scale::y_resMtp(50, display.height());
//     }

//   } while (display.nextPage());

//   return true;
// }