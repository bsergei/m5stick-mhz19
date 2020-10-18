#include <ArduinoJson.h>

#include "InitApi.h"

InitApi::InitApi(AppSettings *s)
{
  appSettings = s;
  server = new WebServer(80);
}

InitApi::~InitApi()
{
  delete server;
}

void InitApi::begin()
{
  server->on("/", std::bind(&InitApi::handle_OnConnect, this));
  server->on("/update", std::bind(&InitApi::handle_OnUpdate, this));
  server->onNotFound(std::bind(&InitApi::handle_NotFound, this));
  server->enableCORS(true);

  Serial.println("HTTP server begin");
  server->begin();
  Serial.println("HTTP server started");
}

void InitApi::handle()
{
  server->handleClient();
}

void InitApi::handle_OnConnect()
{
  String ptr;
  ptr += "<!DOCTYPE html>\n";
  ptr += "<html>\n";
  ptr += "<head>\n";
  ptr += "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "  <title>Configuration</title>\n";
  ptr += "  <style>\n";
  ptr += "    html {\n";
  ptr += "      font-family: Helvetica;\n";
  ptr += "      display: inline-block;\n";
  ptr += "      margin: 0 auto;\n";
  ptr += "      text-align: center;\n";
  ptr += "    }\n";
  ptr += "    body {\n";
  ptr += "      margin: 0;\n";
  ptr += "    }\n";
  ptr += "    form {\n";
  ptr += "      margin-top: 10px;\n";
  ptr += "    }\n";
  ptr += "    form > div {\n";
  ptr += "      margin-bottom: 5px;\n";
  ptr += "    }\n";
  ptr += "    form > div > label {\n";
  ptr += "      width: 150px;\n";
  ptr += "      display: inline-block;\n";
  ptr += "      text-align: left;\n";
  ptr += "    }\n";
  ptr += "  </style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "  <form action=\"/update\" method=\"POST\">\n";
  ptr += "    <div><label>Wifi SSID</label><input type=\"text\" id=\"ssid\" name=\"ssid\" value=\"" + String(appSettings->ssid) + "\"></div>\n";
  ptr += "    <div><label>Wifi Password</label><input type=\"password\" id=\"password\" name=\"password\" value=\"" + String(appSettings->password) + "\"></div>\n";
  ptr += "    <div><label>NTP Server</label><input type=\"text\" id=\"ntpServer\" name=\"ntpServer\" value=\"" + String(appSettings->ntpServer) + "\"></div>\n";
  ptr += "    <div><label>Timezone (seconds)</label><input type=\"text\" id=\"timeZone\" name=\"timeZone\" value=\"" + String(appSettings->timeZone) + "\"></div>\n";
  ptr += "    <div><label>Weather City</label><input type=\"text\" id=\"weatherCity\" name=\"weatherCity\" value=\"" + String(appSettings->weatherCity) + "\"></div>\n";
  ptr += "    <div><label>Weather API Key</label><input type=\"text\" id=\"openweatherApiKey\" name=\"openweatherApiKey\" value=\"" + String(appSettings->openweatherApiKey) + "\"></div>\n";
  ptr += "    <div><label>CO2 Alarm</label><input type=\"text\" id=\"co2AlarmThreshold\" name=\"co2AlarmThreshold\" value=\"" + String(appSettings->co2AlarmThreshold) + "\"></div>\n";
  ptr += "    <input type=\"submit\" name=\"SUBMIT\" value=\"Update\">\n";
  ptr += "  </form>\n";
  ptr += "</body>\n";
  ptr += "</html>\n";

  server->send(200, "text/html", ptr);
}

void InitApi::handle_NotFound()
{
  server->send(404, "text/plain", "Not found");
}

void InitApi::handle_OnUpdate()
{
  if (server->hasArg("ssid"))
  {
    String str = server->arg("ssid");
    int16_t len = str.length() + 1;
    char *buffer = new char[str.length() + 1];
    str.toCharArray(buffer, len);
    appSettings->ssid = buffer;
  }

  if (server->hasArg("password"))
  {
    String str = server->arg("password");
    int16_t len = str.length() + 1;
    char *buffer = new char[len];
    str.toCharArray(buffer, len);
    appSettings->password = buffer;
  }

  if (server->hasArg("ntpServer"))
  {
    String str = server->arg("ntpServer");
    int16_t len = str.length() + 1;
    char *buffer = new char[len];
    str.toCharArray(buffer, len);
    appSettings->ntpServer = buffer;
  }

  if (server->hasArg("weatherCity"))
  {
    String str = server->arg("weatherCity");
    int16_t len = str.length() + 1;
    char *buffer = new char[len];
    str.toCharArray(buffer, len);
    appSettings->weatherCity = buffer;
  }

  if (server->hasArg("openweatherApiKey"))
  {
    String str = server->arg("openweatherApiKey");
    int16_t len = str.length() + 1;
    char *buffer = new char[len];
    str.toCharArray(buffer, len);
    appSettings->openweatherApiKey = buffer;
  }

  if (server->hasArg("timeZone"))
  {
    String str = server->arg("timeZone");
    appSettings->timeZone = str.toInt();
  }

    if (server->hasArg("co2AlarmThreshold"))
  {
    String str = server->arg("co2AlarmThreshold");
    appSettings->co2AlarmThreshold = str.toInt();
  }

  appSettings->commit();

  String ptr;
  ptr += "<!DOCTYPE html>\n";
  ptr += "<html>\n";
  ptr += "<head>\n";
  ptr += "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "  <title>Configuration</title>\n";
  ptr += "  <style>\n";
  ptr += "    html {\n";
  ptr += "      font-family: Helvetica;\n";
  ptr += "      display: inline-block;\n";
  ptr += "      margin: 0 auto;\n";
  ptr += "      text-align: center;\n";
  ptr += "    }\n";
  ptr += "    body {\n";
  ptr += "      margin: 0;\n";
  ptr += "    }\n";
  ptr += "  </style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "  <p>Settings updated</p>\n";
  ptr += "</body>\n";
  ptr += "</html>\n";

  server->send(200, "text/html", ptr);

  delay(1000);
  
  Serial.println("Restarting...");
  ESP.restart();
}
