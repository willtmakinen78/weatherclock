/**************************************************************************/
/*
  return the json string recieved when making a GET request to url
*/
String getJsonString(String url)
{
  HTTPClient http;
  String jsonstring = "";

  if (http.begin(url)) {
    // start connection and send HTTP header
    Serial.println("[HTTP] getting url: " + url);
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been sent and Server response header has been handled
      Serial.println("[HTTP] GET... code: " + String(httpCode));

      // file found at server
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        jsonstring = http.getString();
      }
    } else {
      Serial.println("[HTTP] GET... failed, error: " + http.errorToString(httpCode));
    }
    http.end();
  }
  else {
    Serial.println("[HTTP] Unable to connect");
  }
  return jsonstring;
}
/**************************************************************************/
/*
  helper method to return the url string for the api address
*/
String makeApiString()
{
  String apiAddress = API_ADDRESS;
  String lat = LAT;
  String lon = LON;
  String units = UNITS;
  String exclusions = EXCLUSIONS;
  String apiKey = API_KEY;
  return apiAddress + LAT + "&lon=" + LON + units +
         exclusions + apiKey;
}
/**************************************************************************/
/*
  parse jsonString to extract current weather info. load this into
  currentWeather[] with the order [temp, dewpoint, wind, iconid] and
  hourlyWeather[] which is a NUM_HOURS x HOURLY_PARMETER_NUM array with
  [temp, dewpoint icon] for each hour.
*/
void parseJson(String jsonString, int currentWeather[CURRENT_PARAMETER_NUM], int hourlyWeather[NUM_HOURS][HOURLY_PARAMETER_NUM]) {
  // allocate memory for the json document
  // calculated with https://arduinojson.org/v6/assistant/
  const size_t capacity = 48 * JSON_ARRAY_SIZE(1) + JSON_ARRAY_SIZE(48) +
                          20 * JSON_OBJECT_SIZE(1) + 48 * JSON_OBJECT_SIZE(4) +
                          JSON_OBJECT_SIZE(5) + 28 * JSON_OBJECT_SIZE(10) +
                          20 * JSON_OBJECT_SIZE(11) + 7220;
  DynamicJsonDocument doc(capacity);
  deserializeJson(doc, jsonString);

  JsonArray hourly = doc["hourly"];

  JsonObject current = hourly[0];
  int currentTemp = current["temp"]; 
  int currentDewpoint = current["dew_point"];
  int currentWind = current["wind_speed"];

  JsonObject weather = current["weather"][0];
  const char* currentIconStr = weather["icon"];
  int currentIcon = iconToInt(currentIconStr);
  
  currentWeather[TEMP_INDEX] = currentTemp;
  currentWeather[DEWPOINT_INDEX] = currentDewpoint;
  currentWeather[WIND_INDEX] = currentWind;
  currentWeather[CURRENT_ID] = currentIcon;

  // load hourly forecast data
  for (int i = 0; i < NUM_HOURS; i++) {
    JsonObject h = hourly[i+1];
    int hTemp = h["temp"];
    int hDewPoint = h["dew_point"];

    JsonObject hWeather = h["weather"][0];
    const char* hIconStr = hWeather["icon"];
    int hIcon = iconToInt(hIconStr);

    hourlyWeather[i][TEMP_INDEX] = hTemp;
    hourlyWeather[i][DEWPOINT_INDEX] = hDewPoint;
    hourlyWeather[i][HOURLY_ID] = hIcon;
  }
}
/**************************************************************************/
/*
  helper method to convert open weather maps' icon strings to ints
*/
int iconToInt(const char* iconString) {
  char buf[3];
  buf[0] = iconString[0];
  buf[1] = iconString[1];
  buf[2] = '\0';
  return (int) atoi(buf);
}
