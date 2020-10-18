# CO2 Sensor
## Hardware

* Development board M5StickC Plus
* Sensor MH-Z19B

## Features

- CO2 (Carbon dioxide) level continuous output (400-5000 ppm)
- Temperature  output (internal sensor)
- 24h history chart available from built-in Web server
- CO2 alarming using built-in LED
- Day/Night screen mode
- Current and 3h forecast weather 

## Dev Environment

Project uses PlatformIO IDE targeting Espressiff's ESP32 and Arduino framework.

## Configuration

- Wifi SSID name and password to access weather and NTP
- Timezone (in seconds, e.g., 3600 for UTC+1)
- NTP Server name (e.g., europe.pool.ntp.org)
- CO2 alarm threshold (e.g., 1000)
- City and API Key to read data from openweathermap.org

## User Manual

On first power on devices need to be configured. Connect to "CO2_Sensor" Wi-Fi AP and then go to http://192.168.1.1. 
Edit configuration and confirm with Update button. After devices rebooted you should see current CO2 measure, temperature, current weather and 3h forecast.

To see sensor values history, go to device IP or scan QR code that is available after pressing BtnA.

To reboot, press power button shortly.

To factory reset, use BtnA to switch display until "Factory?" shown. Confirm with button BtnB.

To edit settings, use BtnA to switch display until "Edit?" shown. Confirm with button BtnB.

To calibrate MH-Z19B zero point use "Calibrate" option. Refer to MH-Z19B manual for details.