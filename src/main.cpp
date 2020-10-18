#include <ctime>
#include <Arduino.h>
#include <MHZ19.h>
#include <MHZ19PWM.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <M5StickCPlus.h>
#include <NTPClient.h>
#include <EEPROM.h>

#include "MutualM5.h"
#include "RestApi.h"
#include "StringPrinter.h"
#include "RingBuffer.h"
#include "AppUI.h"
#include "AppStatus.h"
#include "TimeEvents.h"
#include "AppSettings.h"
#include "InitApi.h"

AppSettings appSettings;
AppStatus appStatus;

MHZ19 mhz(&Serial2);
TimeEvents timeEvents = TimeEvents(&appSettings, &appStatus);
AppUI appUI = AppUI(&appStatus, &appSettings);

RestApi *restApi = NULL;

void loop()
{
  appUI.update();
  delay(100);
}

void webServerTask(void *arg)
{
  while (1)
  {
    if (restApi)
    {
      restApi->handle();
    }

    delay(10);
  }
}

void mhz19SensorTask(void *arg)
{
  while (1)
  {
    bool isRequestCalibrate = appStatus.mutualRead<bool>([](AppStatus *s) { return s-> requestCalibrate; });
    if (isRequestCalibrate) 
    {
      appStatus.mutualUpdate([](AppStatus *s) { s-> requestCalibrate = false; });

      M5.Beep.tone(5000);
      delay(200);
      M5.Beep.mute();

      mhz.calibrateZero();

      M5.Beep.tone(4000);
      delay(200);
      M5.Beep.mute();

      delay(5000);
    }

    MHZ19_RESULT response = mhz.retrieveData();
    if (response == MHZ19_RESULT_OK)
    {
      short co2Value = mhz.getCO2();
      short temperature = mhz.getTemperature();

      appStatus.mutualUpdate([co2Value, temperature](AppStatus *s) {
        s->isMHZ19Available = true;
        s->co2Value = co2Value;
        s->temperature = temperature;
      });

      Serial.print("CO2 value:");
      Serial.println(co2Value);

      delay(5000);
    }
    else
    {
      appStatus.mutualUpdate([](AppStatus *s) {
        s->isMHZ19Available = false;
      });

      delay(1000);
    }
  }
}

void powerTask(void *arg)
{
  float vbat = M5_Safe.GetBatVoltage();
  while (vbat < 3.2)
  {
    delay(100);
    vbat = M5_Safe.GetBatVoltage();
  }

  while (1)
  {
    if (M5_Safe.GetBtnPress())
    {
      M5.Beep.tone(1500);
      do
      {
        delay(100);
      } while (M5_Safe.GetBtnPress());
      M5.Beep.mute();
      ESP.restart();
    }

    appStatus.mutualUpdate([](AppStatus *s) {
      s->batVoltage = M5_Safe.GetBatVoltage();
    });
    delay(100);
  }
}

void wifiTask(void *arg)
{
  appStatus.mutualUpdate([](AppStatus *s) {
    s->isWifiConnected = false;
    StringPrinter(s->localIPStr).print("***.***.***.***");
  });

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(false);
  WiFi.persistent(false);
  delay(100);
  WiFi.begin(appSettings.ssid, appSettings.password);
  delay(100);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }

  Serial.println("Connected WiFi");

  appStatus.mutualUpdate([](AppStatus *s) {
    StringPrinter(s->localIPStr).print(WiFi.localIP());
    s->isWifiConnected = true;
  });

  if (!restApi)
  {
    restApi = new RestApi(&appStatus);
    restApi->begin();
  }

  while (1)
  {
    // TODO Wifi resilience.
    delay(1000);
  }
}

void rtcTask(void *arg)
{
  while (!appStatus.mutualRead<bool>([](AppStatus *s) { return s->isWifiConnected; }))
  {
    delay(100);
  }
  WiFiUDP ntpUDP;
  NTPClient timeClient(ntpUDP, appSettings.ntpServer);
  timeClient.begin();
  while (!timeClient.forceUpdate() || timeClient.getEpochTime() == 0)
  {
    Serial.println("NTP failed");
    delay(1000);
  }

  time_t epochTime = timeClient.getEpochTime() + appSettings.timeZone;
  tm *localTime = localtime(&epochTime);
  RTC_DateTypeDef date;
  RTC_TimeTypeDef time;
  date.Year = localTime->tm_year + 1900;
  date.Year = localTime->tm_year + 1900;
  date.Month = localTime->tm_mon + 1;
  date.Date = localTime->tm_mday;
  date.WeekDay = localTime->tm_wday;
  time.Hours = localTime->tm_hour;
  time.Minutes = localTime->tm_min;
  time.Seconds = localTime->tm_sec;

  M5_Safe.SetDateAndTime(&date, &time);

  Serial.printf("NTP: %d/%d/%d %02d:%02d:%02d\n", date.Year, date.Month, date.Date, time.Hours, time.Minutes, time.Seconds);

  appStatus.mutualUpdate([date, time](AppStatus *s) {
    s->date = date;
    s->time = time;
  });

  timeEvents.init();

  while (1)
  {
    delay(1000);

    appStatus.mutualUpdate([](AppStatus *s) {
      M5_Safe.GetDateAndTime(&s->date, &s->time);
    });

    timeEvents.update();
  }
}

void blinkTask(void *arg)
{
  uint8_t s = HIGH;
  while (1)
  {
    if (appStatus.mutualRead<short>([](AppStatus *s) { return s->co2Value; }) > appSettings.co2AlarmThreshold)
    {
      if (s == HIGH)
      {
        s = LOW;
      }
      else
      {
        s = HIGH;
      }
    }
    else
    {
      s = HIGH;
    }
    digitalWrite(10, s);
    delay(300);
  }
}

void initTask(void *arg)
{
  IPAddress apIP(192, 168, 1, 1);
  
  WiFi.mode(WIFI_AP);  
  WiFi.softAP("CO2_Sensor");
  delay(500);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  delay(500);
  
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  appStatus.isWifiConnected = true;
  StringPrinter(appStatus.localIPStr).print(WiFi.softAPIP());

  InitApi api = InitApi(&appSettings);
  api.begin();

  while (1) 
  {
    api.handle();
    delay(10);
  }
}

void onStartSetup()
{
  M5.Beep.tone(4000);
  delay(100);
  M5.Beep.mute();
}

void onFinishSetup()
{
  M5.Beep.tone(5000);
  delay(100);
  M5.Beep.mute();

  digitalWrite(10, HIGH);
}

void setDefaultPCF8563Register() {
  Wire1.beginTransmission(0x51);
  Wire1.write(0x0);
  Wire1.write(0b00001000);
  Wire1.write(0b00000000);
  Wire1.endTransmission();
}

void setup()
{
  pinMode(10, OUTPUT);
  // Turn on LED.
  digitalWrite(10, LOW);

  M5.begin();
  setDefaultPCF8563Register();

  EEPROM.begin(sizeof(AppSettingsData));
  appSettings.load();
  bool isInit = appSettings.needInit;

  Serial2.begin(9600, SERIAL_8N1, 26, 0);

  onStartSetup();  

  if (isInit) 
  {
    xTaskCreatePinnedToCore(initTask, "initTask", 1024 * 20, NULL, 4, NULL, 1);
  } 
  else
  {
    xTaskCreatePinnedToCore(powerTask, "powerTask", 1024 * 4, NULL, 4, NULL, 1);
    xTaskCreatePinnedToCore(mhz19SensorTask, "mhz19SensorTask", 1024 * 4, NULL, 4, NULL, 1);
    xTaskCreatePinnedToCore(wifiTask, "wifiTask", 1024 * 4, NULL, 4, NULL, 1);
    xTaskCreatePinnedToCore(webServerTask, "webServerTask", 1024 * 20, NULL, 4, NULL, 1);
    xTaskCreatePinnedToCore(rtcTask, "rtcTask", 1024 * 4, NULL, 4, NULL, 1);
    xTaskCreatePinnedToCore(blinkTask, "blinkTask", 1024 * 2, NULL, 4, NULL, 1);
  }

  onFinishSetup();
}
