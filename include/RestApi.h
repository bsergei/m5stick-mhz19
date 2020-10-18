#ifndef __RESTAPI_H__
#define __RESTAPI_H__

#include <Arduino.h>
#include <WebServer.h>

#include "AppStatus.h"

class RestApi
{
public:
    RestApi(AppStatus *s);
    virtual ~RestApi();

    void begin();
    void handle();

private:
    WebServer *server;
    AppStatus *appStatus;

    void handle_OnConnect();
    void handle_OnApiPoints();
    void handle_NotFound();

    String sendPoints();
    String sendHTML();
};

#endif