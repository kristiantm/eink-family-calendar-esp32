/*

This is the E-Ink Family Calendar project by Argion /  Kristian Thorsted Madsen.
https://www.instructables.com/id/E-Ink-Family-Calendar-Using-ESP32/

The project was written for a LOLIN 32 ESP32 microcontroller and a 7.5 Waveshare 800x480 e-ink display.
Also note, that the project is dependent on you uploading a google script, to fetch and sort the calendar entries.

The basic configuration now happens in the WIFI hotspot that is booting when you power on the board.
The one exception is in case you either use a tri-colour waveshare e-ink screen, or have a different pin-mapping than in the instructions.
In that case adjust the lines in this file, under the comment "Mapping of Waveshare ESP32 Driver Board"

Weather is fetched from Open-Meteo (https://open-meteo.com), which is free for non-commercial use and needs no API key.

*/


#include <Arduino.h>
#define ENABLE_GxEPD2_GFX 0
#include <GxEPD2_3C.h>
//#include <GxEPD2_BW.h>

#include <credentials.h>


#include <time.h>
#include <ArduinoJson.h>              // https://github.com/bblanchon/ArduinoJson needs version v7 or above

#include <iconsOWM.c>

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <WiFiManager.h>             // https://github.com/tzapu/WiFiManager


/* Mapping of Waveshare ESP32 Driver Board - 3C is tri-color displays, BW is black and white ***** */
GxEPD2_3C<GxEPD2_750c_Z08, GxEPD2_750c_Z08::HEIGHT/2> display(GxEPD2_750c_Z08(/*CS=*/ 15, /*DC=*/ 27, /*RST=*/ 26, /*BUSY=*/ 25));
//GxEPD2_BW<GxEPD2_750_T7, GxEPD2_750_T7::HEIGHT/2> display(GxEPD2_750_T7(/*CS=*/15, /*DC=*/ 27, /*RST=*/ 26, /*BUSY=*/ 25)); // GDEW075T7 800x480
const int8_t spiSCK = 13, spiMISO = 12, spiMOSI = 14, spiSS = 15; // Recommended PIN settings from Waveshare - note that this is not the default for most screens

/* Mapping of Generic ESP32 Driver Board - 3C is tri-color displays, BW is black and white ***** */
//GxEPD2_3C<GxEPD2_750c_Z08, GxEPD2_750c_Z08::HEIGHT/2> display(GxEPD2_750c_Z08(/*CS=*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4));
//GxEPD2_BW<GxEPD2_750_T7, GxEPD2_750_T7::HEIGHT/2> display(GxEPD2_750_T7(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // GDEW075T7 800x480
//const int8_t spiSCK = 18, spiMISO = 19, spiMOSI = 23, spiSS = 5;


// Settings entered in the web-portal - stored in the ESP32 flash (NVS) using Preferences
struct calendarConfig
{
  String script;    // Either the full web app URL of the google script, or just its deployment id
  String scriptKey; // Optional - must match SCRIPT_KEY in the google script
  String latitude;
  String longitude;
};

calendarConfig config;
Preferences preferences;

WiFiManager wifiManager;
WiFiManagerParameter scriptParam("script", "Google script web app URL (or deployment id)", "", 160);
WiFiManagerParameter scriptKeyParam("key", "Google script key (optional)", "", 64);
WiFiManagerParameter latitudeParam("lat", "Latitude, e.g. 55.68", "", 16);
WiFiManagerParameter longitudeParam("lon", "Longitude, e.g. 12.57", "", 16);
bool configSaved = false;

// Right now the calendarentries are limited to time and title
struct calendarEntries
{
  String calTime;
  String calTitle;
};

struct calendarEntries calEnt[calEntryCount];
int calEntryFetched = 0;

// Today's weather from Open-Meteo
struct weatherInfo
{
  int code; // WMO weather code - see https://open-meteo.com/en/docs
  float tempMin;
  float tempMax;
  float windSpeed;
  float windDir;
};

struct weatherInfo weather;

float batterylevel = -1; // Being set when reading battery level - used to avoid deep sleep when under 0%, and when drawing battery

// Number of failed refreshes in a row - kept in RTC memory, which survives deep sleep
RTC_DATA_ATTR int failedAttempts = 0;

void initDisplay();
void loadConfig();
void saveConfig();
bool isConfigured();
void setupWifiManager();
bool connectWifi();
void runWebPortal();
bool refreshCalendar();
bool getCalendar();
bool getWeather();
void drawCalendar(bool timeKnown, const struct tm& timeinfo, bool weatherSuccess);
void drawMessage(const String& title, const String& message);
void drawWeather();
void drawWind();
const unsigned char* weatherIcon(int code);
void readBattery();
void deepSleepFor(uint64_t seconds);
void deepSleepTill(int wakeHour);
String urlEncode(const String& text);

// Main flow of the program. It is designed to boot up, pull the info and refresh the screen, and then go back into deep sleep.
void setup() {

  // Initialize board
  Serial.begin(115200);
  Serial.println("setup");

  // Power-on and reset (not waking up from deep sleep) is when we show the setup portal
  bool coldBoot = esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_UNDEFINED;

  // The clock keeps running in deep sleep, but the timezone has to be set again after every wake-up
  setenv("TZ", timeZone, 1);
  tzset();

  // Read batterylevel before turning on wifi, as the radio pulls the voltage down
  readBattery();

  initDisplay();
  loadConfig();
  setupWifiManager();

  bool isConnected = false;
  if (coldBoot) {
    // Connects to the saved wifi - or opens the "FamilyCalendar" hotspot for setup, if there is none or it cannot be reached
    wifiManager.setConfigPortalTimeout(portalTimeoutTime);
    wifiManager.setConnectTimeout(wifiTimeoutTime);
    isConnected = wifiManager.autoConnect(HOSTNAME);
  } else {
    isConnected = connectWifi();
  }

  if (isConnected) Serial.println("Wifi connected on IP " + WiFi.localIP().toString()); else Serial.println("Wifi not connected");

  bool refreshed = false;
  if (isConnected && isConfigured()) {
    Serial.println("Configuration exist and wifi works - displaying calendar");
    refreshed = refreshCalendar();
  }

  if (isConnected && coldBoot) {
    if (!isConfigured()) {
      drawMessage("Almost there", "Open http://" + WiFi.localIP().toString() + "/param\nin a browser on your wifi, and enter\nthe link to your google script\nand your location.");
    }

    // If the calendar is not waking from sleep, spend 5 minutes displaying the web-portal, so the settings can be changed
    runWebPortal();

    if (configSaved && isConfigured()) {
      refreshed = refreshCalendar();
    }
  }

  // Turn off display before deep sleep
  display.hibernate();

  if (refreshed || failedAttempts >= failedRetryCount) {
    failedAttempts = 0;

    // Deep-sleeping until the selected wake-hour - default 5 in the morning
    Serial.println("Refresh done - sleeping");
    deepSleepTill(HOUR_TO_WAKE);
  }

  // Something failed - the screen keeps showing the last calendar, and we try again a bit later
  failedAttempts++;
  Serial.println("Refresh failed (" + String(failedAttempts) + ") - trying again in " + String(failedRetryMinutes) + " minutes");
  deepSleepFor(failedRetryMinutes * 60);

}

// Not used, as we boot up from scratch every time we wake from deep sleep
  void loop() {

  }

void initDisplay() {
  // Map and init SPI pins before the display, so it uses them instead of the standard SPI pins, e.g. SCK(18), MISO(19), MOSI(23), SS(5)
  SPI.begin(spiSCK, spiMISO, spiMOSI, spiSS);
  display.init(115200, true, 20, false, SPI, SPISettings(4000000, MSBFIRST, SPI_MODE0));
}

void loadConfig() {
  preferences.begin("calendar", true);
  config.script = preferences.getString("script", "");
  config.scriptKey = preferences.getString("key", "");
  config.latitude = preferences.getString("lat", "");
  config.longitude = preferences.getString("lon", "");
  preferences.end();

  // Show the current values in the web-portal
  scriptParam.setValue(config.script.c_str(), 160);
  scriptKeyParam.setValue(config.scriptKey.c_str(), 64);
  latitudeParam.setValue(config.latitude.c_str(), 16);
  longitudeParam.setValue(config.longitude.c_str(), 16);

  if(isConfigured()) Serial.println("Configuration loaded"); else Serial.println("Configuration not loaded");
}

// Called by the web-portal when the settings are saved
void saveConfig() {
  config.script = scriptParam.getValue();
  config.scriptKey = scriptKeyParam.getValue();
  config.latitude = latitudeParam.getValue();
  config.longitude = longitudeParam.getValue();

  config.script.trim();
  config.scriptKey.trim();
  config.latitude.trim();
  config.longitude.trim();

  // Allow a comma as decimal separator, e.g. 55,68
  config.latitude.replace(',', '.');
  config.longitude.replace(',', '.');

  preferences.begin("calendar", false);
  preferences.putString("script", config.script);
  preferences.putString("key", config.scriptKey);
  preferences.putString("lat", config.latitude);
  preferences.putString("lon", config.longitude);
  preferences.end();

  Serial.println("Configuration saved");
  configSaved = true;
}

bool isConfigured() {
  return config.script.length() > 0 && config.latitude.length() > 0 && config.longitude.length() > 0;
}

void setupWifiManager() {
  WiFi.setHostname(HOSTNAME);
  wifiManager.setHostname(HOSTNAME);
  wifiManager.setTitle("Family Calendar");

  wifiManager.addParameter(&scriptParam);
  wifiManager.addParameter(&scriptKeyParam);
  wifiManager.addParameter(&latitudeParam);
  wifiManager.addParameter(&longitudeParam);
  wifiManager.setSaveParamsCallback(saveConfig);

  std::vector<const char*> menu = { "wifi", "param", "info", "restart" };
  wifiManager.setMenu(menu);

  // Show how to get started on the e-ink screen, when the setup hotspot opens
  wifiManager.setAPCallback([](WiFiManager*) {
    drawMessage("Welcome", String("Connect your phone to the wifi\n\"") + HOSTNAME + "\" to set up the calendar.\n\nIf it does not open by itself,\ngo to http://192.168.4.1");
  });
}

// Connect using the wifi saved by the web-portal - used when waking from deep sleep
bool connectWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin();

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < wifiTimeoutTime * 1000UL) {
    delay(250);
    Serial.print(".");
  }
  Serial.println();

  return WiFi.status() == WL_CONNECTED;
}

// Serve the settings on the local network for portalTimeoutTime seconds, or until they are saved
void runWebPortal() {
  Serial.println("Showing web-portal on http://" + WiFi.localIP().toString() + " for " + String(portalTimeoutTime) + " seconds");

  configSaved = false;
  wifiManager.startWebPortal();

  unsigned long start = millis();
  while (!configSaved && millis() - start < portalTimeoutTime * 1000UL) {
    wifiManager.process();
    delay(10);
  }

  wifiManager.stopWebPortal();
}

// Get time, calendar and weather, and draw them. Returns false if the calendar could not be fetched - the screen is then left untouched
bool refreshCalendar() {

  // Get time from timeserver - used for the greeting, and when going into deep sleep again to ensure that we wake at the right hour
  configTzTime(timeZone, ntpServer);
  struct tm timeinfo;
  bool timeKnown = getLocalTime(&timeinfo, 10000);
  if(!timeKnown) {
    Serial.println("Failed to obtain time");
  }

  if (!getCalendar()) {
    return false;
  }

  bool weatherSuccess = getWeather();

  drawCalendar(timeKnown, timeinfo, weatherSuccess);

  return timeKnown;
}

// Getting calendar from your published google script
bool getCalendar() {
  String url = config.script;
  if (!url.startsWith("https://")) {
    url = "https://script.google.com/macros/s/" + url + "/exec";
  }
  url += (url.indexOf('?') < 0 ? "?" : "&");
  url += "max=" + String(calEntryCount);
  if (config.scriptKey.length() > 0) {
    url += "&key=" + urlEncode(config.scriptKey);
  }

  Serial.println("Getting calendar");

  WiFiClientSecure client;
  client.useBuiltinCACertBundle();

  HTTPClient http;
  http.setTimeout(20000); // The google script can be slow to respond
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS); // Google script answers with a redirect to googleusercontent.com
  if (!http.begin(client, url)) {
    Serial.println("Cannot connect to google script");
    return false;
  }

  int returnCode = http.GET();
  Serial.print("Returncode: "); Serial.println(returnCode);
  if (returnCode != HTTP_CODE_OK) {
    http.end();
    return false;
  }

  String response = http.getString();
  http.end();

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, response);
  if (error) {
    // Typically a google login or error page - check that the script is deployed with access for "Anyone"
    Serial.print(F("Calendar is not valid JSON: "));
    Serial.println(error.c_str());
    Serial.println(response.substring(0, 200));
    return false;
  }

  if (doc["error"].is<const char*>()) {
    Serial.print("Google script error: ");
    Serial.println(doc["error"].as<const char*>());
    return false;
  }

  // Fill calendarEntries with the entries from the script - they are already sorted and formatted
  calEntryFetched = 0;
  for (JsonObject event : doc["events"].as<JsonArray>()) {
    if (calEntryFetched >= calEntryCount) break;

    calEnt[calEntryFetched].calTime = event["time"] | "";
    calEnt[calEntryFetched].calTitle = event["title"] | "";
    Serial.println(calEnt[calEntryFetched].calTime + " " + calEnt[calEntryFetched].calTitle);
    calEntryFetched++;
  }

  return true;
}

// Get today's weather from Open-Meteo
bool getWeather() {
  String url = "https://api.open-meteo.com/v1/forecast?latitude=" + config.latitude + "&longitude=" + config.longitude +
               "&daily=weather_code,temperature_2m_min,temperature_2m_max,wind_speed_10m_max,wind_direction_10m_dominant"
               "&wind_speed_unit=ms&timezone=auto&forecast_days=1";

  Serial.println(url);

  WiFiClientSecure client;
  client.useBuiltinCACertBundle();

  HTTPClient http;
  if (!http.begin(client, url)) {
    return false;
  }

  int returnCode = http.GET();
  if (returnCode != HTTP_CODE_OK) {
    Serial.print("Weather returncode: "); Serial.println(returnCode);
    http.end();
    return false;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, http.getString());
  http.end();
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return false;
  }

  JsonObject daily = doc["daily"];
  weather.code      = daily["weather_code"][0] | -1;
  weather.tempMin   = daily["temperature_2m_min"][0] | 0.0f;
  weather.tempMax   = daily["temperature_2m_max"][0] | 0.0f;
  weather.windSpeed = daily["wind_speed_10m_max"][0] | 0.0f;
  weather.windDir   = daily["wind_direction_10m_dominant"][0] | 0.0f;

  Serial.println("Weather code: " + String(weather.code));

  return weather.code >= 0;
}

// Main display code - assumes that the display has been initialized
void drawCalendar(bool timeKnown, const struct tm& timeinfo, bool weatherSuccess)
{

  // All data is now gathered and processed.
  // Prepare to refresh the display with the calendar entries and weather info
  const char* greeting = timeKnown ? weekday[timeinfo.tm_wday] : "Hello";

  // Turn off text-wrapping
  display.setTextWrap(false);

  display.setRotation(calendarOrientation);

  // Clear the screen with white using full window mode. Not strictly necessary, but as I selected to use partial window for the content, I decided to do a full refresh first.
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
  }   while(display.nextPage());

  // Print the content on the screen - I use a partial window refresh for the entire width and height, as I find this makes a clearer picture
  display.setPartialWindow(0, 0, display.width(), display.height());
  display.firstPage();
  do {
    int x = calendarPosX;
    int y = calendarPosY;

    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);

    // Print mini-test in top in white (e.g. not visible) - avoids a graphical glitch I observed in all first-lines printed
    display.setCursor(x, 0);
    display.setTextColor(GxEPD_WHITE);
    display.setFont(fontEntryTime);
    display.print(greeting);

    // Print morning greeting (Happy X-day)
    display.setCursor(x, y);
    display.setTextColor(GxEPD_BLACK);
    display.setFont(fontMainTitle);
    display.print(greeting);

    // If fetching the weather was a success, then print the weather
    if(weatherSuccess) {
      drawWeather();
    }

    // Draw battery level
    if(batterylevel >= 0) {
      int batX = weatherPosX+50;
      int batY = weatherPosY + 130;

      display.drawRect(batX + 15, batY - 12, 19, 10, GxEPD_BLACK);
      display.fillRect(batX + 34, batY - 10, 2, 5, GxEPD_BLACK);
      display.fillRect(batX + 17, batY - 10, 15 * batterylevel / 100.0, 6, GxEPD_BLACK);
    }


    // Set position for the first calendar entry
    y = y + 45;

    // Print calendar entries from first [0] to the last fetched [calEntryFetched-1] - in case there is fewer events than the maximum allowed
    for(int i=0;  i < calEntryFetched; i++) {

      // Print event time
      display.setCursor(x, y);
      display.setFont(fontEntryTime);
      display.print(calEnt[i].calTime);

      // Print event title
      display.setCursor(x, y + 30);
      display.setFont(fontEntryTitle);
      display.print(calEnt[i].calTitle);

      // Prepare y-position for next event entry
      y = y + calendarSpacing;

    }

  } while(display.nextPage());
}

// Show a title and a few lines of text - used for the setup instructions
void drawMessage(const String& title, const String& message)
{
  display.setTextWrap(false);
  display.setRotation(calendarOrientation);
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);

    display.setFont(fontMainTitle);
    display.setCursor(calendarPosX, calendarPosY);
    display.print(title);

    display.setFont(fontEntryTime);
    int y = calendarPosY + 60;
    int from = 0;
    while (from <= (int)message.length()) {
      int to = message.indexOf('\n', from);
      if (to < 0) to = message.length();
      display.setCursor(calendarPosX, y);
      display.print(message.substring(from, to));
      y += 30;
      from = to + 1;
    }
  } while(display.nextPage());
}

// Sleep for a number of seconds
void deepSleepFor(uint64_t seconds)
{
  // If battery is too low (see readBattery code), enter deepSleep and do not wake up
  if(batterylevel == 0) {
    Serial.println("Battery empty - sleeping until the battery is charged and the board is reset");
    Serial.flush();
    esp_deep_sleep_start();
  }

  Serial.print("Wake in sec: ");
  Serial.println((unsigned long)seconds);

  esp_sleep_enable_timer_wakeup(seconds * uS_TO_S_FACTOR);
  Serial.flush();
  esp_deep_sleep_start();
}

// Sleep until set wake-hour
void deepSleepTill(int wakeHour)
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo, 1000)){
    Serial.println("Failed to obtain time");
    deepSleepFor(failedRetryMinutes * 60);
  }

  // Find the next wake-hour in local time - mktime takes care of daylight saving time
  time_t now = mktime(&timeinfo);
  struct tm wake = timeinfo;
  wake.tm_hour = wakeHour;
  wake.tm_min = 0;
  wake.tm_sec = 0;
  wake.tm_isdst = -1;
  time_t wakeTime = mktime(&wake);

  // The internal clock drifts during sleep, so if we woke up a bit early, the next refresh is tomorrow
  if (wakeTime - now < 60 * 60) {
    wake = timeinfo;
    wake.tm_mday += 1;
    wake.tm_hour = wakeHour;
    wake.tm_min = 0;
    wake.tm_sec = 0;
    wake.tm_isdst = -1;
    wakeTime = mktime(&wake);
  }

  deepSleepFor(wakeTime - now);
}

// Pick a weathericon from iconsOWM.c based on the WMO weather code from Open-Meteo - see https://open-meteo.com/en/docs
const unsigned char* weatherIcon(int code) {
  if (code == 0) return icon01Map;                  // Clear sky
  if (code == 1) return icon02Map;                  // Mainly clear
  if (code == 2) return icon03Map;                  // Partly cloudy
  if (code == 3) return icon04Map;                  // Overcast
  if (code == 45 || code == 48) return icon50Map;   // Fog
  if (code >= 51 && code <= 57) return icon09Map;   // Drizzle
  if (code >= 61 && code <= 67) return icon10Map;   // Rain
  if (code >= 71 && code <= 77) return icon13Map;   // Snow
  if (code >= 80 && code <= 82) return icon09Map;   // Rain showers
  if (code == 85 || code == 86) return icon13Map;   // Snow showers
  if (code >= 95) return icon11Map;                 // Thunderstorm
  return nullptr;
}

// Draw the weather icon with the temperature below it
void drawWeather() {
  int x = weatherPosX+5;
  int y = weatherPosY;
  int width = 100;
  int height = 100;

  const unsigned char* icon = weatherIcon(weather.code);
  if (icon) {
    display.drawInvertedBitmap(x, y, icon, width, height, GxEPD_BLACK);
  } else {
    Serial.println("No icon for weather code " + String(weather.code));
  }

  String weatherInfo = String(lroundf(weather.tempMin)) + "C / " + String(lroundf(weather.tempMax)) + "C";

  display.setCursor(x+5,y+height-10);
  display.setTextColor(GxEPD_BLACK);
  display.setFont(fontWeather);

  display.print(weatherInfo);

  drawWind();

}

// Combining the wind-info-line
void drawWind() {
  int x = weatherPosX;
  int y = weatherPosY;
  int height = 100;

  float dir = weather.windDir;
  String txtDirection;

  float div = 360/8;
  float start = div / 2;

  if(dir < start  ) {
    txtDirection = "N";
  } else if(dir < (div*1 + start)) {
    txtDirection = "NE";
  } else if(dir < (div*2 + start)) {
    txtDirection = "E";
  } else if(dir < (div*3 + start)) {
    txtDirection = "SE";
  } else if(dir < (div*4 + start)) {
    txtDirection = "S";
  } else if(dir < (div*5 + start)) {
    txtDirection = "SW";
  } else if(dir < (div*6 + start)) {
    txtDirection = "W";
  } else if(dir < (div*7 + start)) {
    txtDirection = "NW";
  } else {
    txtDirection = "N";
  }

  float speed = weather.windSpeed;
  String txtSpeed;

  if(speed < 1.5) {
    txtSpeed = "Calm";
  } else if(speed < 3.3) {
    txtSpeed = "Gentle";
  } else if(speed < 7.9) {
    txtSpeed = "Moderate";
  } else if(speed < 10.7) {
    txtSpeed = "Fresh";
  } else if(speed < 13.8) {
    txtSpeed = "Hard";
  } else if(speed < 17.1) {
    txtSpeed = "Near gale";
  } else if(speed < 20.7) {
    txtSpeed = "Gale";
  } else if(speed < 24.4) {
    txtSpeed = "Hard gale";
  } else if(speed < 28.4) {
    txtSpeed = "Storm";
  } else if(speed < 32.6) {
    txtSpeed = "Hard storm";
  } else {
    txtSpeed = "Hurricane";
  }

  String windInfo = "" + String(speed,0) +"ms " + txtDirection + " " + txtSpeed;

  display.setCursor(x-30,y+height+5); // Below temperature settings
  display.setTextColor(GxEPD_BLACK);
  display.setFont(fontWeather);

  display.print(windInfo);

}


void readBattery() {
  // analogReadMilliVolts uses the factory calibration of the ESP32 ADC
  float voltage = analogReadMilliVolts(batteryPin) / 1000.0 * (batteryR1 + batteryR2) / batteryR2;

  if (voltage > 1 ) { // Only display if there is a valid reading
    Serial.println("Voltage = " + String(voltage));

    float percentage = (voltage - batteryEmptyVoltage) / (batteryFullVoltage - batteryEmptyVoltage) * 100;
    batterylevel = constrain(percentage, 0, 100);
    Serial.println("Batterylevel = " + String(batterylevel));
  }

}

// Escape text for use in an URL query
String urlEncode(const String& text) {
  const char* hex = "0123456789ABCDEF";
  String encoded;
  for (unsigned int i = 0; i < text.length(); i++) {
    char c = text[i];
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      encoded += c;
    } else {
      encoded += '%';
      encoded += hex[(c >> 4) & 0x0F];
      encoded += hex[c & 0x0F];
    }
  }
  return encoded;
}
