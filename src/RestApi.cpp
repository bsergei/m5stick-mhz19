#include <ArduinoJson.h>

#include "RestApi.h"

RestApi::RestApi(AppStatus *s)
{
  appStatus = s;
  server = new WebServer(80);
}

RestApi::~RestApi()
{
  delete server;
}

void RestApi::begin()
{
  server->on("/", std::bind(&RestApi::handle_OnConnect, this));
  server->on("/api/points", std::bind(&RestApi::handle_OnApiPoints, this));
  server->onNotFound(std::bind(&RestApi::handle_NotFound, this));
  server->enableCORS(true);

  Serial.println("HTTP server begin");
  server->begin();
  Serial.println("HTTP server started");
}

void RestApi::handle()
{
  server->handleClient();
}

void RestApi::handle_OnConnect()
{
  server->send(200, "text/html", sendHTML());
}

void RestApi::handle_OnApiPoints()
{
  server->send(200, "application/json", sendPoints());
}

void RestApi::handle_NotFound()
{
  server->send(404, "text/plain", "Not found");
}

String RestApi::sendPoints()
{
  size_t capacity = JSON_ARRAY_SIZE(appStatus->co2Values.maxSize) + JSON_ARRAY_SIZE(appStatus->temperatureValues.maxSize) + JSON_OBJECT_SIZE(2);
  DynamicJsonDocument doc(capacity + 1024);

  JsonArray co2 = doc.createNestedArray("co2");
  JsonArray temperature = doc.createNestedArray("temperature");

  appStatus->mutualRead<bool>([&co2, &temperature](AppStatus *s)
  {
    s->co2Values.read([&co2](short a) {
      co2.add(a);
      return true;
    });

    s->temperatureValues.read([&temperature](short a) {
      temperature.add(a);
      return true;
    });

    return true;
  });

  String ptr;
  serializeJson(doc, ptr);
  return ptr;
}

String RestApi::sendHTML()
{
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>CO2 Meter</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0 auto; text-align: center;} body {margin: 0;}\n";
  ptr += "</style>\n";
  ptr += "<script src=\"https://cdn.jsdelivr.net/npm/chart.js@2.8.0\"></script>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<canvas id=\"myChart\"></canvas>\n";
  ptr += "<script>\n";
  ptr += "fetch('/api/points')\n";
  ptr += "  .then(\n";
  ptr += "    function(response) {\n";
  ptr += "      if (response.status !== 200) {\n";
  ptr += "        console.log('Looks like there was a problem. Status Code: ' +\n";
  ptr += "          response.status);\n";
  ptr += "        return;\n";
  ptr += "      }\n";
  ptr += "      // Examine the text in the response\n";
  ptr += "      response.json().then(function(data) {\n";
  ptr += "        var ctx = document.getElementById('myChart').getContext('2d');\n";
  ptr += "        var dt = new Date();\n";
  ptr += "        var labels = [];\n";
  ptr += "        var dataRev = [];\n";
  ptr += "        for (var idx = data.co2.length - 1; idx >= 0; idx--) {\n";
  ptr += "          dataRev.push(data.co2[idx]);\n";
  ptr += "          var pd = new Date(dt.valueOf() - idx * 1000 * 60);\n";
  ptr += "          labels.push(pd.getHours() + ':' + pd.getMinutes())\n";
  ptr += "        }\n";
  ptr += "        var dataRev2 = [];\n";
  ptr += "        for (var idx = data.temperature.length - 1; idx >= 0; idx--) {\n";
  ptr += "          dataRev2.push(data.temperature[idx]);\n";
  ptr += "        }\n";
  ptr += "        var chart = new Chart(ctx, {\n";
  ptr += "            type: 'line',\n";
  ptr += "            data: {\n";
  ptr += "                labels: labels,\n";
  ptr += "                datasets: [\n";
  ptr += "                {\n";
  ptr += "                    label: 'CO2 History',\n";
  ptr += "                    //backgroundColor: 'rgb(255, 99, 132)',\n";
  ptr += "                    borderColor: 'rgb(255, 99, 132)',\n";
  ptr += "                    data: dataRev,\n";
  ptr += "                    yAxisID: 'CO2 History',\n";
  ptr += "                },\n";
  ptr += "                {\n";
  ptr += "                    label: 'T History',\n";
  ptr += "                    //backgroundColor: 'rgb(160, 116, 196)',\n";
  ptr += "                    borderColor: 'rgb(160, 116, 196)',\n";
  ptr += "                    data: dataRev2,\n";
  ptr += "                    yAxisID: 'T History',\n";
  ptr += "                }\n";
  ptr += "                ]\n";
  ptr += "            },\n";
  ptr += "            // Configuration options go here\n";
  ptr += "            options: {\n";
  ptr += "                scales: {\n";
  ptr += "                yAxes: [{\n";
  ptr += "                  id: 'CO2 History',\n";
  ptr += "                  type: 'linear',\n";
  ptr += "                  position: 'left',\n";
  ptr += "                }, {\n";
  ptr += "                  id: 'T History',\n";
  ptr += "                  type: 'linear',\n";
  ptr += "                  position: 'right',\n";
  ptr += "                  ticks: {\n";
  ptr += "                          max: 50,\n";
  ptr += "                          min: 10\n";
  ptr += "                        }\n";
  ptr += "                }]\n";
  ptr += "              }\n";
  ptr += "            }\n";
  ptr += "        });\n";
  ptr += "      });\n";
  ptr += "    }\n";
  ptr += "  )\n";
  ptr += "  .catch(function(err) {\n";
  ptr += "    console.log('Fetch Error :-S', err);\n";
  ptr += "  });\n";
  ptr += "</script>\n";
  ptr += "</body></html>\n";
  return ptr;
}