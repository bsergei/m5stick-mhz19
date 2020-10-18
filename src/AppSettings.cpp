#include "AppSettings.h"
#include "EEPROM.h"

AppSettings::AppSettings()
{
    needInit = false;
    timeZone = 0;
    ssid = "*";
    password = "*";
    ntpServer = "europe.pool.ntp.org";

    co2AlarmThreshold = 1000;

    weatherCity = "";
    openweatherApiKey = "";
}

void AppSettings::load()
{
    AppSettingsData data;
    EEPROM.get<AppSettingsData>(0, data);

    if (data.magicId == 0xFEEDC0DE)
    {
        needInit = data.needInit;
        timeZone = data.timeZone;

        char *t_ssid = new char[strlen(data.ssid) + 1];
        strcpy(t_ssid, data.ssid);
        ssid = t_ssid;

        char *t_password = new char[strlen(data.password) + 1];
        strcpy(t_password, data.password);
        password = t_password;
        
        char *t_ntpServer = new char[strlen(data.ntpServer) + 1];
        strcpy(t_ntpServer, data.ntpServer);
        ntpServer = t_ntpServer;
        
        co2AlarmThreshold = data.co2AlarmThreshold;

        char *t_weatherCity = new char[strlen(data.weatherCity) + 1];
        strcpy(t_weatherCity, data.weatherCity);
        weatherCity = t_weatherCity;

        char *t_openweatherApiKey = new char[strlen(data.openweatherApiKey) + 1];
        strcpy(t_openweatherApiKey, data.openweatherApiKey);
        openweatherApiKey = t_openweatherApiKey;
    }
    else
    {
        needInit = true;
    }
}

void AppSettings::editCurrent()
{
    needInit = true;
    updateInFlash();
    ESP.restart();
}

void AppSettings::commit() 
{
    needInit = false;
    updateInFlash();
}

void AppSettings::updateInFlash()
{
    AppSettingsData data;
    data.magicId = 0xFEEDC0DE;
    data.needInit = needInit;

    data.timeZone = timeZone;
    data.co2AlarmThreshold = co2AlarmThreshold;

    memset(data.ssid, 0, sizeof(data.ssid));
    strncpy(data.ssid, ssid, sizeof(data.ssid) - 1);

    memset(data.password, 0, sizeof(data.password));
    strncpy(data.password, password, sizeof(data.password) - 1);

    memset(data.weatherCity, 0, sizeof(data.weatherCity));
    strncpy(data.weatherCity, weatherCity, sizeof(data.weatherCity) - 1);

    memset(data.openweatherApiKey, 0, sizeof(data.openweatherApiKey));
    strncpy(data.openweatherApiKey, openweatherApiKey, sizeof(data.openweatherApiKey) - 1);

    memset(data.ntpServer, 0, sizeof(data.ntpServer));
    strncpy(data.ntpServer, ntpServer, sizeof(data.ntpServer) - 1);

    EEPROM.put(0, data);
    EEPROM.commit();
}
