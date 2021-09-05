/**************************************************************************/
/*
  library inclusions
*/

#include <Adafruit_GFX.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_EPD.h>
#include <Adafruit_ImageReader_EPD.h>
#include <SdFat.h>
#include "info.h"
#include "time.h"
#include <Fonts/FreeSans24pt7b.h>

/**************************************************************************/
/*
  magic numbers
*/

// # of seconds to sleep between waking up and weather updates
#define SLEEP 3600

// WiFi timeout (seconds)
#define WIFI_TIMEOUT 45

// [temp, dewpoint, wind, iconId]
#define CURRENT_PARAMETER_NUM 4
#define NUM_HOURS 5
// [temp, dewpoint, iconId]
#define HOURLY_PARAMETER_NUM 3
#define TEMP_INDEX 0
#define DEWPOINT_INDEX 1
#define WIND_INDEX 2
#define CURRENT_ID 3
#define HOURLY_ID 2

// display dimensions
#define CURRENT_WIDTH 264
#define CURRENT_HEIGHT 176
#define HOURLY_WIDTH 296
#define HOURLY_HEIGHT 128

// ntp address
const char* ntpServer = "pool.ntp.org";

// how long to give the EPDs to flash the new image
#define DISPLAY_UPDATE_DELAY 7500

/**************************************************************************/
/*
  pin assignments
*/

// common to both displays
#define EPD_EN       22
#define EPD_RST      14

// current display
#define CURRENT_CS   32
#define CURRENT_DC   15
#define CURRENT_SRCS -1

// hourly display
#define HOURLY_CS    33
#define HOURLY_DC    27
#define HOURLY_SRCS  12
#define HOURLY_BUSY  -1

// use the SD card slot on the current display
#define SD_CS        23

// for battery monitoring
#define BAT_PIN A13
#define ADC_MAX 4095
#define VOLTAGE_SCALE 2
#define ESP32_VOLTAGE 3.3
#define REFERENCE_VOLTAGE 1.1
#define LOW_BATTERY 3.25

/**************************************************************************/
/*
  library initializations
*/

// 2.7" tricolor epd for current weather
Adafruit_IL91874 currentDisplay(CURRENT_WIDTH, CURRENT_HEIGHT, CURRENT_DC, EPD_RST, CURRENT_CS, CURRENT_SRCS);

// 2.9" tricolor epd for hourly forecast
Adafruit_IL0373 hourlyDisplay(HOURLY_WIDTH, HOURLY_HEIGHT, HOURLY_DC, EPD_RST, HOURLY_CS, HOURLY_SRCS, HOURLY_BUSY);

// SD card filesystem
SdFat SD;

// Image-reader object, pass in SD filesystem
Adafruit_ImageReader reader(SD);

/**************************************************************************/
/*
  due to sleeping, all the actual work will just be done in setup()
*/
void setup() 
{
  // start everything up
  initialize();

  // grab json string
  String jsonString = getJsonString(makeApiString());

  // error check for internet connectivity/http errors
  if (jsonString.length() < 1) {
    displayError('h');
  }

  // parse json for current weather and hourly forecast.
  // current weather order: [temp, dewpoint, wind, iconid]
  // hourly forecast: NUM_HOURS x HOURLY_PARAMETER_NUM array one entry
  // for each hour. order: [temp, dewpoint, icon]
  int currentWeather[CURRENT_PARAMETER_NUM];
  int hourlyWeather[NUM_HOURS][HOURLY_PARAMETER_NUM];
  parseJson(jsonString, currentWeather, hourlyWeather);

  Serial.print("Current Weather: ");
  for (int i = 0; i < CURRENT_PARAMETER_NUM; i++) {
    Serial.print(currentWeather[i]);
    Serial.print(", ");
  }
  Serial.println("\nHourly Forecast: ");
  for (int i = 0; i < NUM_HOURS; i++) {
    Serial.print("\thour "); Serial.print(i); Serial.print(": ");
    for (int j = 0; j < HOURLY_PARAMETER_NUM; j++) {
      Serial.print(hourlyWeather[i][j]);
      Serial.print(", ");
    }
    Serial.println();
  }

  // get current time before passing along`
  struct tm timeinfo = getTime();

  // set up before updating displays
  prepDisplays();

  // update current weather display
  updateCurrent(currentWeather, timeinfo);

  // update hourly forecast display
  updateHourly(hourlyWeather, timeinfo);

  Serial.println("drawing to displays");
  currentDisplay.display();
  hourlyDisplay.display();

  poweroff();
}
/**************************************************************************/
/*
  initialize all libraries and components
*/
void initialize()
{
  Serial.begin(9600);

  // enable the display's regulators
  pinMode(EPD_EN, OUTPUT);
  digitalWrite(EPD_EN, HIGH);
  Serial.println("displays enabled");

  // initialize both displays and clear their buffers
  currentDisplay.begin();
  hourlyDisplay.begin();
  Serial.println("displays initialized");

  // initialize sd card
  if (!SD.begin(SD_CS, SD_SCK_MHZ(10))) {
    displayError('s');
  }
  Serial.println("SD card initialized");

  // check for low battery
  float voltage = float(analogRead(BAT_PIN)) / float(ADC_MAX) *
                  VOLTAGE_SCALE * ESP32_VOLTAGE * REFERENCE_VOLTAGE;
  Serial.print("lipo voltage: ");
  Serial.println(voltage);
  if (voltage <= LOW_BATTERY) {
    displayError('b');
  }
  else {
    Serial.println("battery ok");
  }

  // initialize wifi
  Serial.print("Connecting to WiFi ");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int counter = 0;
  while (WiFi.status() != WL_CONNECTED && counter < WIFI_TIMEOUT) {
    delay(1000);
    Serial.print(".");
    counter++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("connected");
  }
  else {
    displayError('w');
  }

}
/**************************************************************************/
/*
  put everything into low-power mode then sleep
*/
void poweroff()
{
  Serial.println("powering down");
  currentDisplay.powerDown();
  hourlyDisplay.powerDown();
  Serial.println("sleeping");
  ESP.deepSleep(SLEEP * 1e6);
}
/**************************************************************************/
/*
  get current time
*/
struct tm getTime() {
  Serial.println("getting time from NTP");
  configTime(GMTOFFSET_SEC, DAYLIGHTOFFSET_SEC, ntpServer);
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain update time");
  }
  else {
    Serial.println("NTP update successful");
  }

  return timeinfo;
}

/**************************************************************************/

void loop() {
  // will never reach this point; included for completion
}

/**************************************************************************/
