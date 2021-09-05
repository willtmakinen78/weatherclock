# weatherclock
An analog-digital hybrid wall clock that can tell the time and give a forecast for the current day.

This was one of my first forways into low power IOT electronics. At its core is an esp32, which is able to be placed into a deep sleep mode and absolutely sip power. Using two e-ink displays, the esp32 wakes up once an hour, grabs the current and hourly forecast from openweathermap.org, updates the displays, and then goes back to sleep. Due to to the fact that e-ink displays consume no power to hold an image on screen, power is only consumed during this update period once an hour, with the esp32 consuming microamps of current during sleep. This allows the clock to have a battery life of about one year, after which it can be charged through the integrated charing port.

Given the limited amount of display space available, the weather paremeters to display were chosen to reflect those most pertinent to running: temperature, dewpoint, and wind. Additionally, in the event of an SD card failure or low battery, one of the screens will be taken up by displaying an error icon notifying the user, as can be seen in the code.
