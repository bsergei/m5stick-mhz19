#ifndef __APPSTATUS_H__
#define __APPSTATUS_H__

#include <M5StickCPlus.h>
#include "RingBuffer.h"
#include "Weather.h"

class AppStatus
{
public:
    AppStatus();

    bool isWifiConnected;
    char localIPStr[16];

    bool isMHZ19Available;
    short co2Value;
    short temperature;

    float batVoltage;

    RingBuffer co2Values;
    RingBuffer temperatureValues;

    RTC_DateTypeDef date;
    RTC_TimeTypeDef time;

    WeatherData weather;
    bool hasWeather;

    WeatherData forecast;
    bool hasForecast;

    int desiredBrightness;

    bool requestCalibrate;

    void mutualUpdate(const std::function<void(AppStatus *)> action);

    template <typename T>
    T mutualRead(const std::function<T(AppStatus *)> func)
    {
        lock();
        T value = func(this);
        unlock();
        return value;
    }

    void lock();
    void unlock();

private:
    SemaphoreHandle_t xMutex_appStatus;
};

#endif