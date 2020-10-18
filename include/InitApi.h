#ifndef __INITAPI_H__
#define __INITAPI_H__

#include <Arduino.h>
#include <WebServer.h>

#include "AppSettings.h"

class InitApi
{
public:
    InitApi(AppSettings *s);
    virtual ~InitApi();

    void begin();
    void handle();

private:
    WebServer *server;
    AppSettings *appSettings;

    void handle_OnConnect();
    void handle_NotFound();
    void handle_OnUpdate();
};

#endif