/**************************************************************************/
/*
  library inclusions
*/
/**************************************************************************/
/*
  magic numbers
*/

// icon ids
#define DEWPOINT_ID 20
#define WIND_ID     21
#define BATTERY_ID  30
#define WIFI_ID     31
#define SD_ID       32
#define HTTP_ID     33
#define CLOCK_ID    40
#define TEMP_ID     41

#define HOURLY_ICON_SIZE  55
#define ERROR_ICON_SIZE   100

const GFXfont *tempFont = &FreeSans24pt7b;

/**************************************************************************/
/*
  get displays ready to and set general formatting parameters
*/
void prepDisplays() {
  currentDisplay.clearBuffer();
  hourlyDisplay.clearBuffer();

  currentDisplay.setRotation(3);
  currentDisplay.fillScreen(EPD_WHITE);
  currentDisplay.setTextWrap(false);

  hourlyDisplay.fillScreen(EPD_WHITE);
  hourlyDisplay.setTextWrap(false);
}
/**************************************************************************/
/*
  display current weather info on currentDisplay. timeinfo prints update time
  order: [temp, dewpoint, wind, iconid]
*/
void updateCurrent(int currentWeather[CURRENT_PARAMETER_NUM], struct tm timeinfo)
{
  int temp = currentWeather[TEMP_INDEX];
  int dewpoint = currentWeather[DEWPOINT_INDEX];
  int wind = currentWeather[WIND_INDEX];
  int iconId = currentWeather[CURRENT_ID];

  // big temp text
  currentDisplay.setTextColor(EPD_BLACK);
  currentDisplay.setCursor(10, currentDisplay.height() - 35);
  currentDisplay.setFont(tempFont);
  currentDisplay.setTextSize(3);
  currentDisplay.println(temp);

  // icon
  displayIcon(iconId, 5, 15, 'l');
  //  currentDisplay.drawRect(5, 15, 100, 100, EPD_RED);

  // dewpoint
  // dewpoint icon
  displayIcon(DEWPOINT_ID, currentDisplay.width() - 48, 15, 'a');
  // text
  currentDisplay.setTextColor(EPD_BLACK);
  currentDisplay.setCursor(currentDisplay.width() - 52, 50);
  currentDisplay.setFont(NULL);
  currentDisplay.setTextSize(3);
  currentDisplay.println(dewpoint);
  // degree symbol
  currentDisplay.drawCircle(currentDisplay.width() - 12, 45, 3, EPD_BLACK);

  // windspeed
  // wind icon
  displayIcon(WIND_ID, currentDisplay.width() - 48, 70, 'a');
  // number text
  currentDisplay.setTextColor(EPD_BLACK);
  currentDisplay.setCursor(currentDisplay.width() - 52, 98);
  currentDisplay.setFont(NULL);
  currentDisplay.setTextSize(3);
  currentDisplay.print(wind);
  // mph text
  currentDisplay.setTextSize(0);
  currentDisplay.println("mph");

  // update time
  //  printUpdateTime();
  currentDisplay.setTextColor(EPD_RED);
  currentDisplay.setCursor(3, currentDisplay.height() - 10);
  currentDisplay.setFont(NULL);
  currentDisplay.setTextSize(0);
  currentDisplay.print("Updated ");
  currentDisplay.print(&timeinfo, "%B %d %H:%M");
}
/**************************************************************************/
/*
  display hourly forecast on hourlyDisplay using input array hourlyWeather
*/
void updateHourly(int hourlyWeather[NUM_HOURS][HOURLY_PARAMETER_NUM], struct tm timeinfo)
{
  int h = timeinfo.tm_hour;

  // draw key icons
  displayIcon(CLOCK_ID, 1, 1, 's');
  displayIcon(TEMP_ID, 0, 80, 's');
  displayIcon(DEWPOINT_ID, 0, 107, 's');

  hourlyDisplay.setTextColor(EPD_BLACK);
  hourlyDisplay.setFont(NULL);
  hourlyDisplay.setTextSize(2);

  for (int i = 0; i < NUM_HOURS; i++) {
    int hPrint = h + i + 1;
    if (hPrint > 23) hPrint = i;

    // times
    hourlyDisplay.setCursor(i * HOURLY_ICON_SIZE + 32, 3);
    hourlyDisplay.printf("%02dh", hPrint);

    // weather icons
    displayIcon(hourlyWeather[i][HOURLY_ID], i * HOURLY_ICON_SIZE + 21, 23, 's');
   
    // temps
    hourlyDisplay.setCursor(i * HOURLY_ICON_SIZE + 35, 83);
    hourlyDisplay.print(hourlyWeather[i][TEMP_INDEX]);

    // dewpoints
    hourlyDisplay.setCursor(i * HOURLY_ICON_SIZE + 35, 111);
    hourlyDisplay.print(hourlyWeather[i][DEWPOINT_INDEX]);
  }
}
/**************************************************************************/
/*
  display iconid at x, y, loading differently depending on type
*/
void displayIcon(int iconId, int x, int y, char type)
{
  String id = String(iconId);
  // the image to load into RAM
  Adafruit_Image_EPD img;

  switch (type)
  {
    // auxillary icon for current display
    case 'a': {
        Serial.print("updating auxillary icon. SD read status: ");

        String path = "/auxillary/" + id + ".bmp";
        int len = path.length();
        char buf[len + 1];
        path.toCharArray(buf, len + 1);

        ImageReturnCode stat = reader.loadBMP(buf, img);
        reader.printStatus(stat);

        img.draw(currentDisplay, x, y);
      }
      break;
    // large icon for current display
    case 'l':
      {
        Serial.print("updating large icon. SD read status: ");

        String path = "/large/" + id + ".bmp";
        int len = path.length();
        char buf[len + 1];
        path.toCharArray(buf, len + 1);

        ImageReturnCode stat = reader.loadBMP(buf, img);
        reader.printStatus(stat);

        img.draw(currentDisplay, x, y);
      }
      break;
    // small icon for hourly display
    case 's':
      {
        Serial.print("updating small icon. SD read status: ");

        String path = "/small/" + id + ".bmp";
        int len = path.length();
        char buf[len + 1];
        path.toCharArray(buf, len + 1);

        ImageReturnCode stat = reader.loadBMP(buf, img);
        reader.printStatus(stat);

        img.draw(hourlyDisplay, x, y);
      }
      break;
    // error icon for hourly display
    case 'e':
      {
        Serial.print("updating error icon. SD read status: ");

        String path = "/error/" + id + ".bmp";
        int len = path.length();
        char buf[len + 1];
        path.toCharArray(buf, len + 1);

        ImageReturnCode stat = reader.loadBMP(buf, img);
        reader.printStatus(stat);

        img.draw(hourlyDisplay, x, y);
      }
      break;
  }
}
/**************************************************************************/
/*
  display low battery icon to indicate need to charge
*/
void displayError(char type)
{
  Serial.print("Initialization ERROR: ");

  // prep
  hourlyDisplay.clearBuffer();
  hourlyDisplay.fillScreen(EPD_WHITE);
  hourlyDisplay.setTextWrap(false);

  int x = (hourlyDisplay.width() / 2) - (ERROR_ICON_SIZE / 2);
  int y = (hourlyDisplay.height() / 2) - (ERROR_ICON_SIZE / 2);

  // display on hourlyDisplay
  switch (type) {
    // sd card not initialized
    case 's':
      Serial.println("SD Card unable to initialize");

      // sd card not working, so can't use icons
      hourlyDisplay.setTextColor(EPD_RED);
      hourlyDisplay.setCursor(10, 35);
      hourlyDisplay.setFont(NULL);
      hourlyDisplay.setTextSize(5);
      hourlyDisplay.print("!");
      hourlyDisplay.setTextColor(EPD_BLACK);
      hourlyDisplay.print("SD");
      break;
    // low batery
    case 'b':
      Serial.println("Low Battery");
      displayIcon(BATTERY_ID, x, y, 'e');
      break;
    // wifi not connected
    case 'w':
      Serial.println("WiFi unable to connect");
      displayIcon(WIFI_ID, x, y, 'e');
      break;
    // http request failed
    case 'h':
      Serial.println("HTTP request failed");
      displayIcon(HTTP_ID, x, y, 'e');
      break;
  }
  hourlyDisplay.display();

  poweroff();
}
/**************************************************************************/
