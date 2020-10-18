#include "AppUI.h"
#include "AppStatus.h"
#include "Weather.h"
#include "WeatherIcon.h"
#include "MutualM5.h"

AppUI::AppUI(AppStatus *t_appStatus, AppSettings *t_appSettings) : appStatus(t_appStatus),
                                                                   appSettings(t_appSettings),
                                                                   disbuff(TFT_eSprite(&M5.Lcd)),
                                                                   started(false),                                                                   
                                                                   page(0),
                                                                   mode(0)
{
}

AppUI::~AppUI()
{
}

void AppUI::setMode(int m)
{
    mode = m;
}

void AppUI::begin()
{
    started = true;
    M5.Lcd.setRotation(3);
    disbuff.createSprite(240, 135);

    if (appSettings->needInit)
    {
        setMode(1);
    }
    else
    {
        M5.update();
        if (M5.BtnA.isPressed())
        {
            setMode(2);
        }
    }

    page = 0;
    updatePage(page, true, false);
}

void AppUI::update()
{
    appStatus->lock();

    if (!started)
    {
        begin();
    }

    handlePageUpdate();
    updatePage(page, false, false);

    appStatus->unlock();
}

int AppUI::getModePageCount()
{
    switch (mode)
    {
    // Normal
    case 0:
        return 3;
    // Init
    case 1:
        return 1;
    // Normal/Config
    case 2:
        return 3;
    default:
        return 0;
    }
}

void AppUI::updatePage(int page, bool pageEnter, bool pageExit)
{
    switch (mode)
    {
    case 0:
        switch (page)
        {
        case 0:
            printPageStatus(pageEnter, pageExit);
            break;

        case 1:
            printPageTrend(pageEnter, pageExit);
            break;

        case 2:
            printPageQR(pageEnter, pageExit);
            break;
        }
        break;

    case 1:
        printPageInit(pageEnter, pageExit);
        break;
    
    case 2:
        switch (page) 
        {
        case 0:
            printPageCalibrate(pageEnter, pageExit);
            break;

        case 1:
            printPageEditConfig(pageEnter, pageExit);
            break;

        case 2:
            printPageFactoryReset(pageEnter, pageExit);
            break;
        }
        break;
    }
}

void AppUI::handlePageUpdate()
{
    M5.update();
    if (M5.BtnA.wasPressed())
    {
        int lastPage = page;
        page++;
        if (page >= getModePageCount())
        {
            page = 0;
        }

        if (lastPage != page)
        {
            updatePage(page, false, true);
        }

        updatePage(page, true, false);
    }
}

void AppUI::printPageQR(bool pageEnter, bool pageExit)
{
    if (pageEnter)
    {
        clear();
        String url = String("http://") + appStatus->localIPStr;
        M5.Lcd.qrcode(url, 50, 5, 125, 5);

        M5_Safe.ScreenBreath(9);
    }

    if (pageExit) 
    {
        M5_Safe.ScreenBreath(appStatus->desiredBrightness);
    }
}

void AppUI::printPageCalibrate(bool pageEnter, bool pageExit)
{
    disbuff.fillRect(0, 0, 240, 135, disbuff.color565(0, 0, 0));
    disbuff.setTextSize(3);
    disbuff.setTextColor(TFT_WHITE);
    disbuff.setCursor(30, 50);
    disbuff.printf("Calibrate?");

    disbuff.setCursor(110, 110);
    disbuff.printf("OK");

    if (M5.BtnB.wasReleased()) 
    {
        appStatus->requestCalibrate = true;
        mode = 0;
        page = 0;
    }

    displaybuff();
}

void AppUI::printPageEditConfig(bool pageEnter, bool pageExit)
{
    disbuff.fillRect(0, 0, 240, 135, disbuff.color565(0, 0, 0));
    disbuff.setTextSize(3);
    disbuff.setTextColor(TFT_WHITE);
    disbuff.setCursor(30, 50);
    disbuff.printf("Config?");

    disbuff.setCursor(110, 110);
    disbuff.printf("OK");

    if (M5.BtnB.wasReleased()) 
    {
        M5.Beep.tone(1000);
        delay(300);
        M5.Beep.mute();
        AppSettings appSettings;
        appSettings.load();
        appSettings.editCurrent();
    }

    displaybuff();
}

void AppUI::printPageFactoryReset(bool pageEnter, bool pageExit)
{
    disbuff.fillRect(0, 0, 240, 135, disbuff.color565(0, 0, 0));
    disbuff.setTextSize(3);
    disbuff.setTextColor(TFT_WHITE);
    disbuff.setCursor(30, 50);
    disbuff.printf("Factory?");

    disbuff.setCursor(110, 110);
    disbuff.printf("OK");

    if (M5.BtnB.wasReleased()) 
    {
        M5.Beep.tone(1000);
        delay(300);
        M5.Beep.mute();
        AppSettings appSettings;
        appSettings.editCurrent();
    }

    displaybuff();
}

void AppUI::printPageStatus(bool pageEnter, bool pageExit)
{
    if (appStatus->isMHZ19Available)
    {
        printStatus();
    }
    else
    {
        printStatusWarmup();
    }
}

void AppUI::printPageTrend(bool pageEnter, bool pageExit)
{
    if (pageEnter)
    {
        disbuff.fillRect(0, 0, 240, 135, disbuff.color565(0, 0, 0));
        
        short points[240];
        int arrSize = 0;

        appStatus->co2Values.read([&points, &arrSize](short a) {
            points[arrSize++] = a;
            return arrSize < 240;
        });

        if (arrSize <= 1)
        {
            disbuff.setTextSize(1);
            disbuff.setTextColor(TFT_WHITE);
            disbuff.setCursor(10, 10);
            disbuff.printf("Not enough data");
            displaybuff();
            return;
        }

        short minValue = 20000;
        short maxValue = 0;

        for (int i = 0; i < arrSize; i++)
        {
            if (points[i] < minValue) 
            {
                minValue = points[i];
            }

            if (points[i] > maxValue)
            {
                maxValue = points[i];
            }
        }

        minValue -= 50;
        maxValue += 50;

        float coeff = 135.0 / abs(maxValue - minValue);
        int lastX = 0;
        int lastY = 0;

        bool isFull = arrSize >= 240;

        uint16_t color = disbuff.color565(15, 160, 0);

        for (int point = 0; point < arrSize; point++)
        {
            // Zero index is the most recent value.
            short value = points[arrSize - 1 - point];
            // Scale X Axis if need.
            int coordX = isFull ? point : (240.0 / (arrSize - 1) * point);
            // Scale and align Y Axis.
            int coordY = 135 - round((value - minValue) * coeff);

            if (isFull)
            {
                disbuff.drawPixel(coordX, coordY, color);
            }
            else
            {
                if (point > 0)
                {
                    disbuff.drawLine(lastX, lastY, coordX, coordY, color);
                }
                lastX = coordX;
                lastY = coordY;
            }
        }

        disbuff.setTextSize(1);
        disbuff.setTextColor(TFT_WHITE);
        disbuff.setCursor(10, 10);
        disbuff.printf("Range: %d-%d", minValue + 50, maxValue - 50);

        displaybuff();
    }
}

void AppUI::printStatus()
{
    disbuff.fillRect(0, 0, 240, 135, disbuff.color565(0, 0, 0));
    disbuff.setTextSize(5);
    disbuff.setTextColor(TFT_WHITE);
    disbuff.setCursor(10, 10);
    disbuff.printf("%04d", appStatus->co2Value);

    disbuff.setCursor(160, 10);
    disbuff.printf("%02d", appStatus->temperature);

    if (appStatus->hasWeather)
    {
        const unsigned char *img = getWeatherIcon(&appStatus->weather);
        if (img == NULL) 
        {
            disbuff.setTextSize(3);
            disbuff.setTextColor(TFT_WHITE);
            disbuff.setCursor(10, 70);
            String str = appStatus->weather.weather;
            char strBuf[str.length() + 1];
            str.toCharArray(strBuf, str.length() + 1);
            disbuff.printf("%-8s %.0f", strBuf, round(appStatus->weather.current_Temp));
        } 
        else
        {
            disbuff.pushImage(5, 50, 70, 70, (uint16_t *)img);

            disbuff.setTextSize(2);
            disbuff.setTextColor(TFT_WHITE);
            disbuff.setCursor(80, 70);
            disbuff.printf("%.1f", appStatus->weather.current_Temp);
        }
    }

    if (appStatus->hasForecast)
    {
        const unsigned char *img = getWeatherIcon(&appStatus->forecast);
        if (img != NULL) 
        {
            disbuff.pushImage(125, 50, 70, 70, (uint16_t *)img);

            disbuff.setTextSize(2);
            disbuff.setTextColor(TFT_WHITE);
            disbuff.setCursor(190, 70);
            disbuff.printf("%.1f", appStatus->forecast.current_Temp);
        }
    }

    disbuff.setTextColor(TFT_WHITE);
    disbuff.setTextSize(1);
    disbuff.fillRect(0, 120, 240, 135, disbuff.color565(20, 20, 20));

    if (appStatus->isWifiConnected)
    {
        disbuff.setTextSize(1);
        disbuff.setCursor(5, 125);
        disbuff.printf("IP:%s", appStatus->localIPStr);
        disbuff.setCursor(130, 125);
        disbuff.printf("%02d:%02d:%02d", appStatus->time.Hours, appStatus->time.Minutes, appStatus->time.Seconds);
    }

    disbuff.setCursor(195, 125);
    disbuff.printf("B:%.2f", appStatus->batVoltage);

    displaybuff();
}

void AppUI::printStatusWarmup()
{
    disbuff.fillRect(0, 0, 240, 135, disbuff.color565(0, 0, 0));
    disbuff.fillRect(0, 0, 240, 25, disbuff.color565(20, 20, 20));
    disbuff.setTextSize(2);
    disbuff.drawString("Warming up...", 26, 5, 1);
    displaybuff();
}

void AppUI::printPageInit(bool pageEnter, bool pageExit)
{
    if (pageEnter)
    {
        clear();
        disbuff.setTextSize(2);
        disbuff.setTextColor(TFT_WHITE);
        disbuff.setCursor(5, 50);
        disbuff.printf("WiFi to CO2_Sensor");
        displaybuff();
    }
}

void AppUI::clear()
{
    disbuff.fillRect(0, 0, 240, 135, disbuff.color565(0, 0, 0));
    displaybuff();
}

void AppUI::displaybuff()
{
    disbuff.pushSprite(0, 0);
}