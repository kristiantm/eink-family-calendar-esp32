/*

Credit to COERT VONK https://coertvonk.com/sw/embedded/esp8266-clock-import-events-from-google-calendar-15809
Have followed his example, and added multi-calendar support, where all events from "selected" calendars are fetched, and sorted before being returned to the caller

You need to put this up as a public script (Deploy > New deployment > Web app, execute as "Me", access "Anyone").
Please note that this will expose your calendar entries to anyone who knows the URL - there is no input or write access.
To protect it, set SCRIPT_KEY below to a random text (letters and digits) and enter the same key in the calendar's web-portal.

The script must use the V8 runtime (Project Settings > "Enable Chrome V8 runtime"), as Google turned off the old Rhino runtime in 2026.
Remember to make a new deployment after changing the script - the calendar needs the new web app URL.

The response is JSON, with the events already sorted and formatted for the display:
{"events":[{"time":"Mon 19 Oct 10:00","title":"Dentist","allDay":false}, ...]}

*/

const SCRIPT_KEY = '';   // Optional - e.g. 'k8Hq2xLm9Pz4'
const DAYS_AHEAD = 14;   // Get appointments for the next 14 days
const MAX_EVENTS = 10;   // Default number of events returned - the calendar asks for the number it can show

function doGet(e) {
  const params = (e && e.parameter) || {};

  if (SCRIPT_KEY && params.key !== SCRIPT_KEY) {
    return json_({ error: 'wrong or missing key' });
  }

  const calendars = CalendarApp.getAllCalendars();
  if (!calendars) {
    return json_({ error: 'no access to calendar' });
  }

  const calendarsSelected = calendars.filter(function (calendar) { return calendar.isSelected(); });
  Logger.log('Calendars: ' + calendars.length + ' Selected: ' + calendarsSelected.length);

  const start = new Date(); start.setHours(0, 0, 0, 0);  // start at midnight
  const stop = new Date(start.getTime() + DAYS_AHEAD * 24 * 3600000);

  const max = Math.min(parseInt(params.max, 10) || MAX_EVENTS, 50);
  const timeZone = Session.getScriptTimeZone();

  // Show just every entry regardless of GuestStatus to also get events from shared calendars where you haven't set up the appointment on your own
  const events = mergeCalendarEvents(calendarsSelected, start, stop).slice(0, max).map(function (event) {
    const allDay = event.isAllDayEvent();
    return {
      time: Utilities.formatDate(event.getStartTime(), timeZone, allDay ? 'EEE d MMM' : 'EEE d MMM HH:mm'),
      title: event.getTitle(),
      allDay: allDay
    };
  });

  return json_({ events: events });
}

function json_(data) {
  return ContentService.createTextOutput(JSON.stringify(data)).setMimeType(ContentService.MimeType.JSON);
}

function mergeCalendarEvents(calendars, startTime, endTime) {

  var params = { start:startTime, end:endTime, uniqueIds:[] };

  return calendars.map(toUniqueEvents_, params)
                  .reduce(toSingleArray_, [])
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
