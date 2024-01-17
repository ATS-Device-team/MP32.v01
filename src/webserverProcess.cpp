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
const char *textHTML PROGMEM = "text/html";
const char *textCSS PROGMEM = "text/css";

void webserver_init(void)
{
    webserver.onNotFound([](AsyncWebServerRequest *request)
                         { request->send(SPIFFS, F("/404error.html"), textHTML); });
    webserver.on("/", [](AsyncWebServerRequest *request)
                 { if (!request->authenticate("admin", "150896"))
                 return request->requestAuthentication();
                    request->send(SPIFFS, F("/home.html"), textHTML); });
    webserver.on("/SimulatorStyle.css", [](AsyncWebServerRequest *request)
                 { request->send(SPIFFS, F("/SimulatorStyle.css"), textCSS); });
    webserver.on("/update", HTTP_POST, [](AsyncWebServerRequest *request)
                 { request->send(200, textHTML, updateResponse); });
    webserver.on("/wifimanager", handle_wifimanager);
    webserver.on("/wifiscan", HTTP_POST, handle_wifiscan);
    webserver.on("/wificonnect", HTTP_POST, handle_wificonnect);
    webserver.on("/wifiapconfig", HTTP_POST, handle_wifiAPconfig);
    webserver.on("/setdefault", HTTP_POST, handle_setDefault);
    webserver.begin();
}

void webserver_process(uint32_t tSec)
{
    updateResponse = F("<strong>Time server on: </strong><span>");
    updateResponse += String(tSec);
    updateResponse += F(" Sec</span>");
    if (WiFi.status() == WL_CONNECTED)
    {
        updateResponse += F("<br><strong>Connected to: </strong><span>");
        updateResponse += WiFi.SSID();
        updateResponse += F("</span>");
        updateResponse += F("<br><strong>Local IP: </strong><span>");
        updateResponse += WiFi.localIP().toString();
        updateResponse += F("</span>");
    }
}

bool webserver_isWifiScanning(void)
{
    return wifiScanning;
}

void handle_wifimanager(AsyncWebServerRequest *request)
{
    String htmlWEB, tempString;
    File file = SPIFFS.open(F("/wifiManager.html"));
    char *tempChar;
    uint8_t wifiPercent;

    if (!request->authenticate("admin", "150896"))
        return request->requestAuthentication();
    while (file.available())
        htmlWEB += (char)file.read();
    tempChar = strstr(htmlWEB.begin(), "<div id=\"inputStationMsg\"></div>") + 26;
    tempString = tempChar;
    htmlWEB.remove(htmlWEB.length() - tempString.length());
    if (WiFi.status() == WL_CONNECTED)
    {
        htmlWEB += F("<strong>Connected </strong>to ");
        htmlWEB += WiFi.SSID();
        htmlWEB += F("<br><strong>Signal streng: </strong>");
        wifiPercent = map(WiFi.RSSI(), -100, -50, 0, 100);
        if (wifiPercent > 100)
            wifiPercent = 100;
        htmlWEB += String(wifiPercent);
        htmlWEB += F("\%<br><strong>Local IP:</strong> ");
        htmlWEB += WiFi.localIP().toString();
    }
    else
        htmlWEB += F("<strong>Not connected</strong>");
    htmlWEB += tempString;
    request->send(200, textHTML, htmlWEB);
    file.close();
    htmlWEB.end();
}

void handle_wifiscan(AsyncWebServerRequest *request)
{
    uint8_t numbsAP = 0;
    String webserverResponse;

    wifiScanning = 1;
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
            webserverResponse += F("<div><a onclick='c(this)'>");
            webserverResponse += WiFi.SSID(countAP);
            for (uint8_t scanWifi = 0; scanWifi < WIFI_BUF_NUMBS; scanWifi++)
                if (WiFi.SSID(countAP) == wifiInfor_list[scanWifi].ssid)
                {
                    webserverResponse += F(" (Saved)");
                    break;
                }
            webserverResponse += F("</a><div role='img' aria-label ='");
            webserverResponse += String(wifiPercent);
            webserverResponse += F("\%' title='");
            webserverResponse += String(wifiPercent);
            webserverResponse += F("\%' class='q q-");
            webserverResponse += String(wifiLevel);
            if (WiFi.encryptionType(countAP) != WIFI_AUTH_OPEN)
                webserverResponse += F(" l '></div><div class='q h'>");
            else
                webserverResponse += F("  '></div><div class='q h'>");
            webserverResponse += String(wifiPercent);
            webserverResponse += F("\%</div></div>");
        }
    }
    else
    {
        webserverResponse = F("No networks found");
    }
    request->send(200, textHTML, webserverResponse);
    wifiScanning = 0;
}

void handle_wificonnect(AsyncWebServerRequest *request)
{
    String webserverResponse;
    uint8_t wifiPercent;

    if (wifi_connect(request->arg("ssid"), request->arg("password")))
    {
        wifi_saveInfor();
        webserverResponse = F("<strong>Connected </strong>to ");
        webserverResponse += WiFi.SSID();
        webserverResponse += F("<br><strong>Signal streng: </strong>");
        wifiPercent = map(WiFi.RSSI(), -100, -50, 0, 100);
        if (wifiPercent > 100)
            wifiPercent = 100;
        webserverResponse += String(wifiPercent);
        webserverResponse += F("\%<br><strong>Local IP:</strong> ");
        webserverResponse += WiFi.localIP().toString();
    }
    else
        webserverResponse = F("<strong>Connect failed</strong>");

    request->send(200, textHTML, webserverResponse);
}

void handle_wifiAPconfig(AsyncWebServerRequest *request)
{
    String reponseHTML;

    if (wifi_setAP(request->arg("APssid"), request->arg("APpassword")))
    {
        wifi_saveInfor();
        reponseHTML = F("Acesspoint config successful");
    }
    else
        reponseHTML = F("Acesspoint config failed");
    request->send(200, textHTML, reponseHTML);
}

void handle_setDefault(AsyncWebServerRequest *request)
{
    wifi_default();
    wifi_saveInfor();
    request->send(200, textHTML, F("Returned to default"));
}
