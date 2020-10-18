#ifndef __APPUI_H__
#define __APPUI_H__

#define UI_STATUS 1
#define UI_STATUS_WARMUP 2

#include <M5StickCPlus.h>
#include "AppStatus.h"

class AppUI
{
public:
    AppUI(AppStatus *t_appStatus, AppSettings *t_appSettings);
    ~AppUI();
    void begin();
    void update();

    void setMode(int m);

private:
    AppStatus *appStatus;
    AppSettings *appSettings;
    TFT_eSprite disbuff;
    bool started;
    int page;
    int mode;

    void handlePageUpdate();
    void updatePage(int page, bool pageEnter, bool pageExit);

    int getModePageCount();

    void printStatus();
    void printStatusWarmup();
    
    void printPageStatus(bool pageEnter, bool pageExit);
    void printPageTrend(bool pageEnter, bool pageExit);
    void printPageQR(bool pageEnter, bool pageExit);
    void printPageCalibrate(bool pageEnter, bool pageExit);
    void printPageEditConfig(bool pageEnter, bool pageExit);
    void printPageFactoryReset(bool pageEnter, bool pageExit);
    void printPageInit(bool pageEnter, bool pageExit);

    void displaybuff();
    void clear();
};

#endif