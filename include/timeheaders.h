// Setup for getting time for time-server

const unsigned long UpdateInterval = (30L * 60L - 03) * 1000000L; // Update delay in microseconds, 13-secs is the time to update so compensate for that
bool LargeIcon =  true;
bool SmallIcon =  false;
#define Large  6
#define Small  4
String time_str, Day_time_str; // strings to hold time and received weather data;



//################ PROGRAM VARIABLES and OBJECTS ################

typedef struct { // For current Day and Day 1, 2, 3, etc
  String   Dt;
  String   Period;
  String   Icon;
  String   Trend;
  String   Main0;
  String   Forecast0;
  String   Forecast1;
  String   Forecast2;
  String   Description;
  String   Time;
  String   Country;
  float    lat;
  float    lon;
  float    Temperature;
  float    Humidity;
  float    High;
  float    Low;
  float    Winddir;
  float    Windspeed;
  float    Rainfall;
  float    Snowfall;
  float    Pressure;
  int      Cloudcover;
  int      Visibility;
  int      Sunrise;
  int      Sunset;
} Forecast_record_type;

#define max_readings 24

Forecast_record_type  WxConditions[1];
Forecast_record_type  WxForecast[max_readings];

#define autoscale_on  true
#define autoscale_off false
#define barchart_on   true
#define barchart_off  false

float pressure_readings[max_readings]    = {0};
float temperature_readings[max_readings] = {0};
float humidity_readings[max_readings]    = {0};
float rain_readings[max_readings]        = {0};
