#ifndef _INFO_H THEN
#define _INFO_H

// WiFi info
#define WIFI_SSID "Langaton"
#define WIFI_PASSWORD "yhteys_2015"

// change this depending on the desired location
#define LAT "38.7852"
#define LON "-77.1027"

// set your timezone
#define GMTOFFSET_SEC -5 * 3600
// set to 3600 if the area follows daylight savings, 0 otherwise
#define DAYLIGHTOFFSET_SEC 3600

// open weather map API url
#define API_ADDRESS "https://api.openweathermap.org/data/2.5/onecall?lat="

// open weather map api key
#define API_KEY "######"

// only want hourly forecast, so exclude everything else
#define EXCLUSIONS "&exclude=current,minutely,daily&appid="

// units to use (can be "&units=imperial" or "&units=metric"
#define UNITS "&units=imperial"

#endif
