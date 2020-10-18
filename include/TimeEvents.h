#ifndef __TIMEEVENTS_H__
#define __TIMEEVENTS_H__

#include <ctime>
#include "AppStatus.h"
#include "RingBuffer.h"
#include "AppSettings.h"

class TimeEvents
{
public:
    TimeEvents(AppSettings *v_appSettings, AppStatus *s);

    void init();
    void update();

private:
    AppSettings *appSettings;
    AppStatus *appStatus;
    bool isNight;
    bool lastIsNight;

    time_t currentEpochTime;
    time_t scheduledWriteCO2Value;

    time_t scheduledWeatherUpdate;
    time_t scheduledForecastUpdate;

    bool getIsNight();
    void onIsNightChanged();
    void updateIsNight();

    void updateSensorValues();

    void updateCurrentEpochTime();

    void updateWeather();
    void updateForecast();
};

#endif
