#include <ctime>
#include "TimeEvents.h"
#include "MutualM5.h"
#include "RingBuffer.h"
#include "AppSettings.h"
#include "Weather.h"

TimeEvents::TimeEvents(AppSettings *v_appSettings, AppStatus *s)
{
    appSettings = v_appSettings;
    appStatus = s;
    isNight = false;
}

void TimeEvents::init()
{
    isNight = getIsNight();
    lastIsNight = isNight;
    onIsNightChanged();

    updateCurrentEpochTime();
    scheduledWriteCO2Value = currentEpochTime;
    scheduledWeatherUpdate = currentEpochTime;
    scheduledForecastUpdate = currentEpochTime;
}

void TimeEvents::update()
{
    updateCurrentEpochTime();
    updateIsNight();
    updateSensorValues();
    updateWeather();
    updateForecast();
}

void TimeEvents::updateCurrentEpochTime()
{
    tm tt = appStatus->mutualRead<tm>([](AppStatus *s) {
        tm t;
        t.tm_year = s->date.Year - 1900;
        t.tm_mon = s->date.Month - 1;
        t.tm_mday = s->date.Date;
        t.tm_hour = s->time.Hours;
        t.tm_min = s->time.Minutes;
        t.tm_sec = s->time.Seconds;
        t.tm_isdst = 0;
        return t;
    });

    currentEpochTime = mktime(&tt) - appSettings->timeZone;
}

void TimeEvents::updateIsNight()
{
    isNight = getIsNight();
    if (lastIsNight != isNight)
    {
        lastIsNight = isNight;
        onIsNightChanged();
    }
}

void TimeEvents::updateSensorValues()
{
    if (currentEpochTime >= scheduledWriteCO2Value)
    {
        scheduledWriteCO2Value += 60;
        appStatus->mutualUpdate([](AppStatus *s) {
            s->co2Values.put(s->co2Value);
            s->temperatureValues.put(s->temperature);
        });
        
        Serial.println("Write sensor value");
    }
}

void TimeEvents::onIsNightChanged()
{
    if (isNight)
    {
        Serial.println("Night mode ON");
        M5_Safe.ScreenBreath(8);
        appStatus->mutualUpdate([] (AppStatus *s) {
            s->desiredBrightness = 8;
        });
    }
    else
    {
        Serial.println("Night mode OFF");
        M5_Safe.ScreenBreath(12);
        appStatus->mutualUpdate([] (AppStatus *s) {
            s->desiredBrightness = 11;
        });
    }
}

bool TimeEvents::getIsNight()
{
    return appStatus->mutualRead<bool>([](AppStatus *s) {
        return (s->time.Hours >= 21 && s->time.Hours <= 23) || (s->time.Hours >= 0 && s->time.Hours <= 8);
    });
}

void TimeEvents::updateWeather()
{
    if (currentEpochTime >= scheduledWeatherUpdate)
    {
        bool isWifiConnected = appStatus->mutualRead<bool>([](AppStatus *s) { return s->isWifiConnected; });
        if (!isWifiConnected)
        {
            scheduledWeatherUpdate = currentEpochTime + 1;
            return;
        }

        appStatus->mutualUpdate([this](AppStatus *s) {
            s->hasWeather = Weather(this->appSettings).update(&s->weather, false);
            if (s->hasWeather)
            {
                // Update after 15 min.
                this->scheduledWeatherUpdate = currentEpochTime + 900;
            }
            else
            {
                // Retry in minute.
                this->scheduledWeatherUpdate = currentEpochTime + 60;
            }
        });
    }
}

void TimeEvents::updateForecast()
{
    if (currentEpochTime >= scheduledForecastUpdate)
    {
        bool isWifiConnected = appStatus->mutualRead<bool>([](AppStatus *s) { return s->isWifiConnected; });
        if (!isWifiConnected)
        {
            scheduledForecastUpdate = currentEpochTime + 1;
            return;
        }

        appStatus->mutualUpdate([this](AppStatus *s) {
            s->hasForecast = Weather(this->appSettings).update(&s->forecast, true);
            if (s->hasForecast)
            {
                // Update after 60 min.
                this->scheduledForecastUpdate = currentEpochTime + 3600;
            }
            else
            {
                // Retry in minute.
                this->scheduledForecastUpdate = currentEpochTime + 60;
            }
        });
    }
}