#ifndef __WEATHER_H__
#define __WEATHER_H__

#include "Arduino.h"
#include "AppSettings.h"

struct WeatherData
{
  String weather;
  String description;
  String icon;
  int id;
  float current_Temp;
  float min_temp;
  float max_temp;
  float humidity;
};

class Weather
{
public:
  Weather(AppSettings *s);
  bool update(WeatherData *w, bool forecast);

private:
  AppSettings *appSettings;
};

#endif
