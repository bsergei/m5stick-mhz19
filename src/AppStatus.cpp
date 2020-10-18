#include "AppStatus.h"

AppStatus::AppStatus() : co2Values(1440), temperatureValues(1440)
{
    isWifiConnected = false;
    isMHZ19Available = false;
    hasWeather = false;
    hasForecast = false;
    co2Value = 0;
    temperature = 0;
    batVoltage = 0;
    desiredBrightness = 10;
    requestCalibrate = false;

    xMutex_appStatus = xSemaphoreCreateMutex();
}

void AppStatus::lock()
{
    while (xSemaphoreTake(xMutex_appStatus, 10 / portTICK_PERIOD_MS) != pdTRUE)
    {
        delay(1);
    }
}

void AppStatus::unlock()
{
    xSemaphoreGive(xMutex_appStatus);
}

void AppStatus::mutualUpdate(const std::function<void(AppStatus *)> action)
{
    lock();
    action(this);
    unlock();
}
