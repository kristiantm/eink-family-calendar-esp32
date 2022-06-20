#ifndef CONFIG_H
#define CONFIG_H

//Which NTP source?
#define NTP_HOST "pool.ntp.org"
#define NTP_OFFSET -6 //problem with DST?
#define NTP_PORT 123

//Which network?
#define NETWORK_SSID "ssid"
#define NETWORK_PASS "pass"

#define NWS_HOST "api.weather.gov"
#define NWS_USER_AGENT "your email here"
#define NWS_LOCATION "FWD/86,102" //Weather Forecast Office + Gridpoint: can be looked up from lat/long at theclockspot.com/weather

#define ICAL_HOST "host.com"
#define ICAL_PATH "/path/"
#define ICAL_URL "https://host.com/path/"
#define ICAL_POSTDATAJSON "{auth: \"authcode\", src: \"calendar url\"}"

#endif