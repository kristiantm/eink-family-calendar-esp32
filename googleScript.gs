/*

Credit to COERT VONK https://coertvonk.com/sw/embedded/esp8266-clock-import-events-from-google-calendar-15809
Have followed his example, and added multi-calendar support, where all events from "selected" calendars are fetched, and sorted before being returned to the caller

You need to put this up as a public script (Deploy > New deployment > Web app, execute as "Me", access "Anyone").
Please note that this will expose your calendar entries to anyone who knows the URL - there is no input or write access.
To protect it, set SCRIPT_KEY below to a random text (letters and digits), and enter the same key in the calendar's web-portal.
Alternatively, leave SCRIPT_KEY empty and add a script property (Project Settings > Script Properties) named SCRIPT_KEY instead -
then you can paste in new versions of this script without editing them. If both are set, the one below is used.

The script must use the V8 runtime (Project Settings > "Enable Chrome V8 runtime"), as Google turned off the old Rhino runtime in 2026.
After changing the script, save it (a deployment uses the last saved version) and use Deploy > Manage deployments > edit > Version: "New version".
That keeps the same web app URL. Archive old deployments you no longer use, as they keep serving the code they were made with.

The response is JSON, with the events already sorted and formatted for the display:
{"events":[{"time":"Mon 19 Oct 10:00","title":"Dentist","allDay":false}, ...]}

The calendar asks for the language of the weekday and month names with the lang parameter (e.g. lang=de gives "Mo 19 Okt 10:00"),
so the script itself needs no changes. As the display fonts only have plain ASCII characters, letters like ae/oe/aa and accents are
spelled out (e.g. "Dalsgaard", "Maerz").

*/

const SCRIPT_KEY = '';   // Optional - e.g. 'k8Hq2xLm9Pz4' - or set it as a script property, see above
const DAYS_AHEAD = 14;   // Get appointments for the next 14 days
const MAX_EVENTS = 10;   // Default number of events returned - the calendar asks for the number it can show

function doGet(e) {
  const params = (e && e.parameter) || {};

  // The key is either set in this file, or as a script property - which lets you paste in new versions without editing them
  const scriptKey = SCRIPT_KEY || PropertiesService.getScriptProperties().getProperty('SCRIPT_KEY');
  if (scriptKey && params.key !== scriptKey) {
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
  const names = dateNames_(params.lang, timeZone);

  // Show just every entry regardless of GuestStatus to also get events from shared calendars where you haven't set up the appointment on your own
  const events = mergeCalendarEvents(calendarsSelected, start, stop).slice(0, max).map(function (event) {
    const allDay = event.isAllDayEvent();
    const startTime = event.getStartTime();
    const time = names.weekday(startTime) + ' ' + Utilities.formatDate(startTime, timeZone, 'd') + ' ' + names.month(startTime) +
                 (allDay ? '' : ' ' + Utilities.formatDate(startTime, timeZone, 'HH:mm'));
    return {
      time: toDisplayText_(time),
      title: toDisplayText_(event.getTitle()),
      allDay: allDay
    };
  });

  return json_({ events: events });
}

// Short weekday and month names in the language asked for, e.g. "de" or "da" - English if none is given or it is not supported
function dateNames_(lang, timeZone) {
  let locale = 'en-GB';
  try {
    if (lang && Intl.DateTimeFormat.supportedLocalesOf([lang]).length > 0) locale = lang;
  } catch (err) {
    Logger.log('Language not supported: ' + lang);
  }

  const weekday = new Intl.DateTimeFormat(locale, { weekday: 'short', timeZone: timeZone });
  const month = new Intl.DateTimeFormat(locale, { month: 'short', timeZone: timeZone });

  // Some languages abbreviate with a dot ("Okt.", "man.") - leave it out, and start the weekday with a capital letter
  const clean = function (text) { return text.replace(/\./g, ''); };
  return {
    weekday: function (date) { const text = clean(weekday.format(date)); return text.charAt(0).toUpperCase() + text.slice(1); },
    month: function (date) { return clean(month.format(date)); }
  };
}

// The display fonts only have plain ASCII characters - spell out the common letters outside it, and remove accents
function toDisplayText_(text) {
  const spelledOut = {
    'æ': 'ae', 'Æ': 'Ae', 'ø': 'oe', 'Ø': 'Oe', 'å': 'aa', 'Å': 'Aa',
    'ä': 'ae', 'Ä': 'Ae', 'ö': 'oe', 'Ö': 'Oe', 'ü': 'ue', 'Ü': 'Ue', 'ß': 'ss',
    'œ': 'oe', 'Œ': 'Oe', '–': '-', '—': '-', '‘': "'", '’': "'", '“': '"', '”': '"',
    '…': '...', ' ': ' '
  };
  return text.replace(/[æÆøØåÅäÄöÖüÜßœŒ–—‘’“”… ]/g,
                      function (c) { return spelledOut[c]; })
             .normalize('NFD').replace(/[̀-ͯ]/g, '')   // e.g. e with accent becomes e
             .replace(/[^\x20-\x7e]/g, '')                         // Remove what is left, e.g. emojis
             .replace(/ {2,}/g, ' ').trim();
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
