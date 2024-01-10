#include "webserverProcess.h"
#include "AsyncWebSocket.h"
#include "SPIFFS.h"
#include "wifiManager.h"

AsyncWebServer webserver(80);
String updateResponse;
bool wifiScanning = 0;

void handle_wifimanager(AsyncWebServerRequest *request);
void handle_wifiscan(AsyncWebServerRequest *request);
void handle_wificonnect(AsyncWebServerRequest *request);
void handle_wifiAPconfig(AsyncWebServerRequest *request);
void handle_setDefault(AsyncWebServerRequest *request);

void webserver_init(void)
{
    webserver.onNotFound([](AsyncWebServerRequest *request)
                         { request->send(SPIFFS, "/404error.html", "text/html"); });
    webserver.on("/", [](AsyncWebServerRequest *request)
                 { request->send(SPIFFS, "/home.html", "text/html"); });
    webserver.on("/SimulatorStyle.css", [](AsyncWebServerRequest *request)
                 { request->send(SPIFFS, "/SimulatorStyle.css", "text/css"); });
    webserver.on("/update", [](AsyncWebServerRequest *request)
                 { request->send(200, "text/html", updateResponse); });
    webserver.on("/wifimanager", handle_wifimanager);
    webserver.on("/wifiscan", handle_wifiscan);
    webserver.on("/wificonnect", handle_wificonnect);
    webserver.on("/wifiapconfig", handle_wifiAPconfig);
    webserver.on("/setdefault", handle_setDefault);
    webserver.begin();
}

void webserver_process(void)
{
    updateResponse = "<strong>Time server on: </strong><span>";
    updateResponse += String((uint32_t)(millis() / 1000));
    updateResponse += " Sec</span>";
    if (WiFi.status() == WL_CONNECTED)
    {
        updateResponse += "<br><strong>Connected to: </strong><span>";
        updateResponse += WiFi.SSID();
        updateResponse += "</span>";
        updateResponse += "<br><strong>Local IP: </strong><span>";
        updateResponse += WiFi.localIP().toString();
        updateResponse += "</span>";
    }
}

bool webserver_isWifiScanning(void)
{
    return wifiScanning;
}

void handle_wifimanager(AsyncWebServerRequest *request)
{
    String htmlWEB, tempString;
    File file = SPIFFS.open("/wifiManager.html");
    char *tempChar;
    uint8_t wifiPercent;

    while (file.available())
        htmlWEB += (char)file.read();
    tempChar = strstr(htmlWEB.begin(), "<div id=\"inputStationMsg\"></div>") + 26;
    tempString = tempChar;
    htmlWEB.remove(htmlWEB.length() - tempString.length());
    if (WiFi.status() == WL_CONNECTED)
    {
        htmlWEB += "<strong>Connected </strong>to ";
        htmlWEB += WiFi.SSID();
        htmlWEB += "<br><strong>Signal streng: </strong>";
        wifiPercent = map(WiFi.RSSI(), -100, -50, 0, 100);
        if (wifiPercent > 100)
            wifiPercent = 100;
        htmlWEB += String(wifiPercent);
        htmlWEB += "\%<br><strong>Local IP:</strong> ";
        htmlWEB += WiFi.localIP().toString();
    }
    else
        htmlWEB += "<strong>Not connected</strong>";
    htmlWEB += tempString;
    request->send(200, "text/html", htmlWEB);
    file.close();
    htmlWEB.end();
}

void handle_wifiscan(AsyncWebServerRequest *request)
{
    uint8_t numbsAP = 0;
    String webserverResponse;

    numbsAP = WiFi.scanNetworks();
    if (numbsAP)
    {
        for (uint8_t countAP = 0; countAP < numbsAP; countAP++)
        {
            uint8_t wifiLevel, wifiPercent;

            if (WiFi.SSID(countAP).length() == 0)
                continue;
            wifiPercent = map(WiFi.RSSI(countAP), -100, -50, 0, 100);
            if (wifiPercent > 100)
                wifiPercent = 100;
            wifiLevel = map(wifiPercent, 0, 100, 1, 4);
            webserverResponse += "<div><a onclick='c(this)'>";
            webserverResponse += WiFi.SSID(countAP);
            webserverResponse += "</a><div role='img' aria-label ='";
            webserverResponse += String(wifiPercent);
            webserverResponse += "\%' title='";
            webserverResponse += String(wifiPercent);
            webserverResponse += "\%' class='q q-";
            webserverResponse += String(wifiLevel);
            if (WiFi.encryptionType(countAP) != WIFI_AUTH_OPEN)
                webserverResponse += " l '></div><div class='q h'>";
            else
                webserverResponse += "  '></div><div class='q h'>";
            webserverResponse += String(wifiPercent);
            webserverResponse += "\%</div></div>";
        }
    }
    else
    {
        webserverResponse = "No networks found";
    }
    request->send(200, "text/html", webserverResponse);
}

void handle_wificonnect(AsyncWebServerRequest *request)
{
    String webserverResponse;
    uint8_t wifiPercent;

    if (wifi_connect(request->arg("ssid"), request->arg("password")))
    {
        wifi_saveInfor();
        webserverResponse = "<strong>Connected </strong>to ";
        webserverResponse += WiFi.SSID();
        webserverResponse += "<br><strong>Signal streng: </strong>";
        wifiPercent = map(WiFi.RSSI(), -100, -50, 0, 100);
        if (wifiPercent > 100)
            wifiPercent = 100;
        webserverResponse += String(wifiPercent);
        webserverResponse += "\%<br><strong>Local IP:</strong> ";
        webserverResponse += WiFi.localIP().toString();
    }
    else
        webserverResponse = "<strong>Connect failed</strong>";

    request->send(200, "text/html", webserverResponse);
}

void handle_wifiAPconfig(AsyncWebServerRequest *request)
{
    String reponseHTML;

    if (wifi_setAP(request->arg("APssid"), request->arg("APpassword")))
    {
        wifi_saveInfor();
        reponseHTML = "Acesspoint config successful";
    }
    else
        reponseHTML = "Acesspoint config failed";
    request->send(200, "text/html", reponseHTML);
}

void handle_setDefault(AsyncWebServerRequest *request)
{
    wifi_default();
    wifi_saveInfor();
    request->send(200, "text/html", "Returned to default");
}
