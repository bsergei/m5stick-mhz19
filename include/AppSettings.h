#ifndef __APPSETTINGS_H__
#define __APPSETTINGS_H__

struct AppSettingsData
{
    unsigned long magicId;

    int co2AlarmThreshold;

    int timeZone;
    char ssid[100];
    char password[100];
    char ntpServer[100];

    char weatherCity[100];
    char openweatherApiKey[100];

    bool needInit;
};

class AppSettings
{
public:
    AppSettings();

    void load();
    void editCurrent();
    void commit();

    bool needInit;

    int co2AlarmThreshold;

    int timeZone;

    const char *ssid;
    const char *password;
    const char *ntpServer;

    const char *weatherCity;
    const char *openweatherApiKey;

private:
    void updateInFlash();
};

#endif