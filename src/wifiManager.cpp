#include "wifiManager.h"
#include "WiFi.h"
#include "ArduinoJson.h"
#include "SPIFFS.h"

#define WIFI_BUF_NUMBS 10

const char *name_wifiInfor = "/WIFI_infor.js";
wifiInfor_t wifiInfor_list[WIFI_BUF_NUMBS];
int8_t staWifiIndex = -1;
String APssid;
String APpassword;

void wifi_init(void)
{
    WiFi.mode(WIFI_MODE_APSTA);
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
    WiFi.softAP(APssid.c_str(), APpassword.c_str());
}

void wifi_loadInfor(void)
{
    File file = SPIFFS.open(name_wifiInfor);
    StaticJsonDocument<512> jsdoc;
    DeserializationError errorJson = deserializeJson(jsdoc, file);
    if (errorJson)
    {
        APssid = AP_SSID_DEFAULT;
        APssid += " ";
        APssid += WiFi.macAddress();
        APpassword = AP_PASSWORD_DEFAULT;
    }
    else
    {
        APssid = jsdoc["AP_WIFI"]["SSID"].as<String>();
        APpassword = jsdoc["AP_WIFI"]["Password"].as<String>();
        for (uint8_t i = 0; i < WIFI_BUF_NUMBS; i++)
        {
            if (jsdoc["WIFI"][i].isNull())
                break;
            else
            {
                wifiInfor_list[i].ssid = jsdoc["WIFI"][i]["SSID"].as<String>();
                wifiInfor_list[i].password = jsdoc["WIFI"][i]["Password"].as<String>();
                wifiInfor_list[i].connectedCount = jsdoc["WIFI"][i]["Connected count"].as<uint32_t>();
            }
        }
    }
    file.close();
    debugSerial.println("Load wifi infor:");
    serializeJsonPretty(jsdoc, debugSerial);
    debugSerial.println();
}

void wifi_saveInfor(void)
{
    uint8_t offset = 0;

    SPIFFS.remove(name_wifiInfor);
    File file = SPIFFS.open(name_wifiInfor, FILE_WRITE, true);
    StaticJsonDocument<512> jsdoc;
    jsdoc["AP_WIFI"]["SSID"] = APssid.c_str();
    jsdoc["AP_WIFI"]["Password"] = APpassword.c_str();
    for (uint8_t i = 0; i < WIFI_BUF_NUMBS; i++)
    {
        if (wifiInfor_list[i].ssid.length() == 0)
        {
            offset += 1;
            continue;
        }
        jsdoc["WIFI"][i - offset]["SSID"] = wifiInfor_list[i].ssid.c_str();
        jsdoc["WIFI"][i - offset]["Password"] = wifiInfor_list[i].password.c_str();
        jsdoc["WIFI"][i - offset]["Connected count"] = wifiInfor_list[i].connectedCount;
    }
    serializeJson(jsdoc, file);
    file.close();
    debugSerial.println("Save wifi infor:");
    serializeJsonPretty(jsdoc, debugSerial);
    debugSerial.println();
}

int wifi_scan(void)
{
    uint8_t networksNumb = 0;
    int8_t wifiSignalCompare = -100, bestWifiIndex = -1;

    networksNumb = WiFi.scanNetworks();
    if (networksNumb == 0)
        return -1;
    for (uint8_t savedWifiIndex = 0; savedWifiIndex < WIFI_BUF_NUMBS; savedWifiIndex++)
    {
        if (wifiInfor_list[savedWifiIndex].ssid.length() == 0)
            continue;
        for (uint8_t scanWifiIndex = 0; scanWifiIndex < networksNumb; scanWifiIndex++)
        {
            if (WiFi.SSID(scanWifiIndex) == wifiInfor_list[savedWifiIndex].ssid)
            {
                wifiInfor_list[savedWifiIndex].available = true;
                wifiInfor_list[savedWifiIndex].rssi = WiFi.RSSI(savedWifiIndex);
                if (wifiInfor_list[savedWifiIndex].rssi > wifiSignalCompare)
                {
                    bestWifiIndex = savedWifiIndex;
                    wifiSignalCompare = wifiInfor_list[bestWifiIndex].rssi;
                }
            }
            else
            {
                wifiInfor_list[savedWifiIndex].available = false;
                wifiInfor_list[savedWifiIndex].rssi = -100;
            }
        }
    }
    WiFi.scanDelete();

    return bestWifiIndex;
}

bool wifi_connect(String ssid, String password)
{
    int8_t slotToWriteIndex = -1, emptySlotIndex = -1;
    uint32_t leastConnetcedCount = 0;

    WiFi.begin(ssid.c_str(), password.c_str());
    if (WiFi.waitForConnectResult(10000) != WL_CONNECTED)
        return false;
    for (uint8_t i = 0; i < WIFI_BUF_NUMBS; i++)
    {
        if (wifiInfor_list[i].ssid.length() == 0)
        {
            if (emptySlotIndex < 0)
                emptySlotIndex = i;
            continue;
        }
        if (wifiInfor_list[i].ssid == ssid)
        {
            staWifiIndex = i;
            wifiInfor_list[i].connectedCount += 1;
            wifiInfor_list[i].rssi = WiFi.RSSI();
            wifiInfor_list[i].available = true;
            return true;
        }
        else
        {
            if (wifiInfor_list[i].connectedCount < leastConnetcedCount)
            {
                leastConnetcedCount = wifiInfor_list[i].connectedCount;
                slotToWriteIndex = i;
            }
        }
    }
    if (emptySlotIndex >= 0)
        slotToWriteIndex = emptySlotIndex;
    wifiInfor_list[slotToWriteIndex].ssid = ssid;
    wifiInfor_list[slotToWriteIndex].password = password;
    wifiInfor_list[slotToWriteIndex].connectedCount = 1;
    wifiInfor_list[slotToWriteIndex].rssi = WiFi.RSSI();
    wifiInfor_list[slotToWriteIndex].available = true;
    staWifiIndex = slotToWriteIndex;

    return true;
}

void wifi_default(void)
{
    APssid = AP_SSID_DEFAULT;
    APssid += " ";
    APssid += WiFi.macAddress();
    APpassword = AP_PASSWORD_DEFAULT;
    staWifiIndex = -1;
    for (uint8_t i = 0; i < WIFI_BUF_NUMBS; i++)
    {
        wifiInfor_list[i].ssid = "";
        wifiInfor_list[i].password = "";
        wifiInfor_list[i].connectedCount = 0;
        wifiInfor_list[i].rssi = -100;
        wifiInfor_list[i].available = 0;
    }
    WiFi.disconnect(false, true);
    WiFi.softAP(APssid.c_str(), APpassword.c_str());
}
