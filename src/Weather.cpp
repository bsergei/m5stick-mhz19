#include "Arduino.h"
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "Weather.h"

Weather::Weather(AppSettings *s) : appSettings(s)
{
}

bool Weather::update(WeatherData *w, bool forecast)
{
  String url = "/data/2.5/";
  if (forecast)
  {
    url += "forecast?q=" + String(appSettings->weatherCity) + "&appid=" + String(appSettings->openweatherApiKey) + "&units=metric&cnt=2";
  }
  else
  {
    url += "weather?q=" + String(appSettings->weatherCity) + "&appid=" + String(appSettings->openweatherApiKey) + "&units=metric&cnt=1";
  }

  const char *host = "api.openweathermap.org";

  WiFiClient httpClient;

  Serial.println("Weather connecting");
  int retryCount = 0;
  while ((!httpClient.connect(host, 80)))
  {
    delay(500);
    retryCount++;
    if (retryCount > 5)
    {
      return false;
    }
  }

  httpClient.print(
      String("GET ") + url + " HTTP/1.1\r\n" +
      "Host: " + host + "\r\n" +
      "Connection: close\r\n\r\n");

  Serial.print("Weather requested");

  retryCount = 0;
  while (!httpClient.available())
  {
    delay(500);
    Serial.print(".");
    if (retryCount > 5)
    {
      return false;
    }
  }

  Serial.println();

  String headers;
  String payload;
  bool isHeaders = true;
  char lastChar = 0;
  while (httpClient.available())
  {
    char c = httpClient.read();
    if (isHeaders && lastChar == '\n' && c == '\r')
    {
      isHeaders = false;
      httpClient.read();
      c = httpClient.read();
    }

    if (isHeaders)
    {
      headers += c;
    }
    else
    {
      payload += c;
    }

    lastChar = c;
  }

  Serial.println("--- Begin Headers");
  Serial.println(headers);
  Serial.println("--- End Headers");
  Serial.println("--- Begin Payload");
  Serial.println(payload);
  Serial.println("--- End Payload");

  DynamicJsonDocument doc(10 * 1024);
  deserializeJson(doc, payload);

  if (forecast)
  {
    w->description = doc["list"][1]["weather"][0]["description"].as<String>();
    w->weather = doc["list"][1]["weather"][0]["main"].as<String>();
    w->icon = doc["list"][1]["weather"][0]["icon"].as<String>();
    w->id = doc["list"][1]["weather"][0]["id"].as<int>();
    w->current_Temp = doc["list"][1]["main"]["temp"].as<float>();
    w->min_temp = doc["list"][1]["main"]["temp_min"].as<float>();
    w->max_temp = doc["list"][1]["main"]["temp_max"].as<float>();
    w->humidity = doc["list"][1]["main"]["humidity"].as<float>();
  }
  else
  {
    w->description = doc["weather"][0]["description"].as<String>();
    w->weather = doc["weather"][0]["main"].as<String>();
    w->icon = doc["weather"][0]["icon"].as<String>();
    w->id = doc["weather"][0]["id"].as<int>();
    w->current_Temp = doc["main"]["temp"].as<float>();
    w->min_temp = doc["main"]["temp_min"].as<float>();
    w->max_temp = doc["main"]["temp_max"].as<float>();
    w->humidity = doc["main"]["humidity"].as<float>();
  }

  if (forecast)
  {
    Serial.print("Forecast: ");
  }
  else
  {
    Serial.print("Weather: ");
  }

  Serial.println(w->weather);
  Serial.println(w->description);
  Serial.println(w->current_Temp);

  return true;
}
