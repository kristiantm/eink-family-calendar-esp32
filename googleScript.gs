/*

Credit to COERT VONK https://coertvonk.com/sw/embedded/esp8266-clock-import-events-from-google-calendar-15809
Have followed his example, and added multi-calendar support, where all events from "selected" calendars are fetched, and sorted before being returned to the caller

You need to put this up as a public script. Please note that this will expose your calendar entries to the internet - however, only you have the unique id - and there is no input or write access.

If you find a solution that can avoid it, then please share and I will update the guide

*/

function doGet(e) {

  //var calendars = CalendarApp.getAllCalendars();

  
  var calendars = CalendarApp.getCalendarsByName("WG-Kalender");
  
  if (calendars == undefined) {
    Logger.log("No data");
    return ContentService.createTextOutput("no access to calendar hubba");
  }

  var calendars_selected = [];
  
  for (var ii = 0; ii < calendars.length; ii++) {
    if (calendars[ii].isSelected()) {
      calendars_selected.push(calendars[ii]);
      Logger.log(calendars[ii].getName());
    }
  }
  
  Logger.log("Old: " + calendars.length + " New: " + calendars_selected.length);


  const now = new Date();
  const oneday = 24*3600000; // [msec]

  


  //const start = new Date(now.getTime() + oneday); 
  const start = new Date(now.getFullYear(),now.getMonth(),now.getDate()+1);
  start.setHours(0, 0, 0);  // start at midnightconst 

  
  //const stop = new Date(start.getTime() + 14 * oneday); //get appointments for the next 14 days
  const stop = new Date(now.getFullYear(),now.getMonth(),now.getDate()+14);

  Logger.log(start)
  Logger.log(stop)

  
  //var events = mergeCalendarEvents(calendars_selected, start, stop); //   pull start/stop time
  var events = calendars_selected[0].getEvents(start, stop); //pull start/stop time
  
  
  var str = '';
  for (var ii = 0; ii < events.length; ii++) {

    var event=events[ii];    
    var myStatus = event.getMyStatus();
    
    
    // Define valid entryStatus to populate array
    switch(myStatus) {
      case CalendarApp.GuestStatus.OWNER:
      case CalendarApp.GuestStatus.YES:
      case CalendarApp.GuestStatus.NO:  
      case CalendarApp.GuestStatus.INVITED:
      case CalendarApp.GuestStatus.MAYBE:
      default:
        break;
    }
    
    // Show just every entry regardless of GuestStatus to also get events from shared calendars where you haven't set up the appointment on your own
    //str += event.getStartTime() + ';' +
    //event.getTitle() +';' + 
    //event.isAllDayEvent() + ';';

    var temp_str = event.getStartTime();
    str += temp_str.getDate() + ' ' ;
    str += temp_str.getMonth() + ';' ;

    //str += temp_str.getFullYear() + ' ; ';
    str += event.getTitle() +';' ;
  }
  
  return ContentService.createTextOutput(str);
}

function mergeCalendarEvents(calendars, startTime, endTime) {

  var params = { start:startTime, end:endTime, uniqueIds:[] };

  return calendars.map(toUniqueEvents_, params)
                  .reduce(toSingleArray_)
                  .sort(byStart_);
}

function toUniqueEvents_ (calendar) {
  return calendar.getEvents(this.start, this.end)
                 .filter(onlyUniqueEvents_, this.uniqueIds);
}

function onlyUniqueEvents_(event) {
  var eventId = event.getId();
  var uniqueEvent = this.indexOf(eventId) < 0;
  if(uniqueEvent) this.push(eventId);
  return uniqueEvent;
}

function toSingleArray_(a, b) { return a.concat(b) }

function byStart_(a, b) {
  return a.getStartTime().getTime() - b.getStartTime().getTime();
}