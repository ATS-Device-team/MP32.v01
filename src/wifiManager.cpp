#include "wifiManager.h"
#include "WiFi.h"
#include "ETH.h"
#include "ESP32Ping.h"
#include "ArduinoJson.h"
#include "SPIFFS.h"
#include "ArduinoOTA.h"

void WiFi_Event(WiFiEvent_t event);
void onStart_OTAHandle(void);
void onEnd_OTAHandle(void);
void onProcess_OTAHandle(unsigned int progress, unsigned int total);
void onError_OTAHandle(ota_error_t error);

const char *DNS_localName PROGMEM = "esp32simulator";
const char *name_wifiInfor PROGMEM = "/WIFI_infor.json";
const char *charAP_WIFI PROGMEM = "AP_WIFI";
const char *charWIFI PROGMEM = "WIFI";
const char *charSSID PROGMEM = "SSID";
const char *charPassword PROGMEM = "Password";
const char *charConnectedCount PROGMEM = "ConnectedCount";
wifiInfor_t wifiInfor_list[WIFI_BUF_NUMBS];
int8_t staWifiIndex = -1;
String APssid;
String APpassword;
bool wifiConnecting = 0;
bool eth_connected = 0;
String OTA_type;

void wifi_init(void)
{
    WiFi.mode(WIFI_MODE_APSTA);
    WiFi.setAutoConnect(false);
    WiFi.onEvent(WiFi_Event);
    WiFi.softAP(APssid.c_str(), APpassword.c_str());
    ETH.begin(0, 4);
    ArduinoOTA.onStart(onStart_OTAHandle);
    ArduinoOTA.onEnd(onEnd_OTAHandle);
    ArduinoOTA.onProgress(onProcess_OTAHandle);
    ArduinoOTA.onError(onError_OTAHandle);
    ArduinoOTA.setRebootOnSuccess(true);
    ArduinoOTA.setHostname(DNS_localName);
    ArduinoOTA.setPassword("tIQMv0VEk3");
    ArduinoOTA.begin();
#if (DEBUG_SERIAL == 1)
    debugSerial.println();
    debugSerial.printf("AP SSID: %s\r\n", APssid.c_str());
    debugSerial.printf("AP Password: %s\r\n", APpassword.c_str());
#endif
}

void WiFi_Event(WiFiEvent_t event)
{
    switch (event)
    {
    case ARDUINO_EVENT_ETH_START:
        Serial.println("ETH Started");
        // set eth hostname here
        ETH.setHostname("esp32-ethernet");
        break;
    case ARDUINO_EVENT_ETH_CONNECTED:
        Serial.println("ETH Connected");
        break;
    case ARDUINO_EVENT_ETH_GOT_IP:
        Serial.print("ETH MAC: ");
        Serial.print(ETH.macAddress());
        Serial.print(", IPv4: ");
        Serial.print(ETH.localIP());
        if (ETH.fullDuplex())
        {
            Serial.print(", FULL_DUPLEX");
        }
        Serial.print(", ");
        Serial.print(ETH.linkSpeed());
        Serial.println("Mbps");
        eth_connected = true;
        break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
        Serial.println("ETH Disconnected");
        eth_connected = false;
        break;
    case ARDUINO_EVENT_ETH_STOP:
        Serial.println("ETH Stopped");
        eth_connected = false;
        break;
    default:
        break;
    }
}

void wifi_process(void)
{
    uint8_t pingTick = 0;

    ArduinoOTA.handle();
    if (pingTick <)
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
        APssid = jsdoc[charAP_WIFI][charSSID].as<String>();
        APpassword = jsdoc[charAP_WIFI][charPassword].as<String>();
        for (uint8_t i = 0; i < WIFI_BUF_NUMBS; i++)
        {
            if (jsdoc[charWIFI][i].isNull())
                break;
            else
            {
                wifiInfor_list[i].ssid = jsdoc[charWIFI][i][charSSID].as<String>();
                wifiInfor_list[i].password = jsdoc[charWIFI][i][charPassword].as<String>();
                wifiInfor_list[i].connectedCount = jsdoc[charWIFI][i][charConnectedCount].as<uint32_t>();
            }
        }
    }
    file.close();
#if (DEBUG_SERIAL == 1)
    debugSerial.println();
    debugSerial.println(F("Load wifi infor:"));
    serializeJsonPretty(jsdoc, debugSerial);
    debugSerial.println();
#endif
}

void wifi_saveInfor(void)
{
    uint8_t offset = 0;

    SPIFFS.remove(name_wifiInfor);
    File file = SPIFFS.open(name_wifiInfor, FILE_WRITE, true);
    StaticJsonDocument<512> jsdoc;
    jsdoc[charAP_WIFI][charSSID] = APssid.c_str();
    jsdoc[charAP_WIFI][charPassword] = APpassword.c_str();
    for (uint8_t i = 0; i < WIFI_BUF_NUMBS; i++)
    {
        if (wifiInfor_list[i].ssid.length() == 0)
        {
            offset += 1;
            continue;
        }
        jsdoc[charWIFI][i - offset][charSSID] = wifiInfor_list[i].ssid.c_str();
        jsdoc[charWIFI][i - offset][charPassword] = wifiInfor_list[i].password.c_str();
        jsdoc[charWIFI][i - offset][charConnectedCount] = wifiInfor_list[i].connectedCount;
    }
    serializeJson(jsdoc, file);
    file.close();
#if (DEBUG_SERIAL == 1)
    debugSerial.println();
    debugSerial.println(F("Save wifi infor:"));
    serializeJsonPretty(jsdoc, debugSerial);
    debugSerial.println();
#endif
}

int wifi_scan(void)
{
    uint8_t networksNumb = 0;
    int8_t wifiSignalCompare = -100, bestWifiIndex = -1;

    networksNumb = WiFi.scanNetworks();
#if (DEBUG_SERIAL == 1)
    debugSerial.println(F("Scan done"));
    if (networksNumb == 0)
    {
        debugSerial.println(F("no networks found"));
    }
    else
    {
        debugSerial.print(networksNumb);
        debugSerial.println(F(" networks found"));
        debugSerial.println(F("Nr | SSID                             | RSSI | CH | Encryption"));
        for (int i = 0; i < networksNumb; ++i)
        {
            // Print SSID and RSSI for each network found
            debugSerial.printf("%2d", i + 1);
            debugSerial.print(F(" | "));
            debugSerial.printf("%-32.32s", WiFi.SSID(i).c_str());
            debugSerial.print(F(" | "));
            debugSerial.printf("%4d", WiFi.RSSI(i));
            debugSerial.print(F(" | "));
            debugSerial.printf("%2d", WiFi.channel(i));
            debugSerial.print(F(" | "));
            switch (WiFi.encryptionType(i))
            {
            case WIFI_AUTH_OPEN:
                debugSerial.print(F("open"));
                break;
            case WIFI_AUTH_WEP:
                debugSerial.print(F("WEP"));
                break;
            case WIFI_AUTH_WPA_PSK:
                debugSerial.print(F("WPA"));
                break;
            case WIFI_AUTH_WPA2_PSK:
                debugSerial.print(F("WPA2"));
                break;
            case WIFI_AUTH_WPA_WPA2_PSK:
                debugSerial.print(F("WPA+WPA2"));
                break;
            case WIFI_AUTH_WPA2_ENTERPRISE:
                debugSerial.print(F("WPA2-EAP"));
                break;
            case WIFI_AUTH_WPA3_PSK:
                debugSerial.print(F("WPA3"));
                break;
            case WIFI_AUTH_WPA2_WPA3_PSK:
                debugSerial.print(F("WPA2+WPA3"));
                break;
            case WIFI_AUTH_WAPI_PSK:
                debugSerial.print(F("WAPI"));
                break;
            default:
                debugSerial.print(F("unknown"));
            }
            debugSerial.println();
            delay(10);
        }
    }
#endif
    if (networksNumb == 0)
        return -1;
#if (DEBUG_SERIAL == 1)
    debugSerial.println(F("WiFi available:"));
#endif
    for (uint8_t savedWifiIndex = 0; savedWifiIndex < WIFI_BUF_NUMBS; savedWifiIndex++)
    {
        if (wifiInfor_list[savedWifiIndex].ssid.length() == 0)
            continue;
        for (uint8_t scanWifiIndex = 0; scanWifiIndex < networksNumb; scanWifiIndex++)
        {
            if (WiFi.SSID(scanWifiIndex) == wifiInfor_list[savedWifiIndex].ssid)
            {
                wifiInfor_list[savedWifiIndex].available = true;
                if (WiFi.RSSI(scanWifiIndex) > wifiSignalCompare)
                {
                    bestWifiIndex = savedWifiIndex;
                    wifiSignalCompare = WiFi.RSSI(scanWifiIndex);
                }
#if (DEBUG_SERIAL == 1)
                debugSerial.printf("WiFi %u: %s %d\r\n", savedWifiIndex, wifiInfor_list[savedWifiIndex].ssid, WiFi.RSSI(scanWifiIndex));
#endif
            }
            else
            {
                wifiInfor_list[savedWifiIndex].available = false;
            }
        }
    }
#if (DEBUG_SERIAL == 1)
    debugSerial.printf("Best wifi: %d", bestWifiIndex);
#endif
    WiFi.scanDelete();

    return bestWifiIndex;
}

bool wifi_setAP(String ssid, String password)
{
    APssid = ssid;
    APpassword = password;
    if (WiFi.softAP(APssid.c_str(), APpassword.c_str()))
    {
#if (DEBUG_SERIAL == 1)
        debugSerial.println();
        debugSerial.printf("AP SSID: %s\r\n", APssid.c_str());
        debugSerial.printf("AP Password: %s\r\n", APpassword.c_str());
#endif
        return true;
    }
    else
    {
#if (DEBUG_SERIAL == 1)
        debugSerial.println();
        debugSerial.println(F("AP config failed"));
#endif
        return false;
    }
}

bool wifi_connect(String ssid, String password)
{
    int savedSlot = -1, emptySlot = -1, leastConnectedSlot = 0, notSavedSlot;

    wifiConnecting = 1;
    for (uint8_t scanIndex = 0; scanIndex < WIFI_BUF_NUMBS; scanIndex++)
    {
        if (wifiInfor_list[scanIndex].ssid.length() == 0)
        {
            if (emptySlot < 0)
                emptySlot = scanIndex;
            continue;
        }
        if (wifiInfor_list[scanIndex].connectedCount < wifiInfor_list[leastConnectedSlot].connectedCount)
            leastConnectedSlot = scanIndex;
        if (ssid == wifiInfor_list[scanIndex].ssid)
        {
            savedSlot = scanIndex;
            if (password == "")
                password = wifiInfor_list[savedSlot].password;
        }
    }
    WiFi.begin(ssid.c_str(), password.c_str());
    if (WiFi.waitForConnectResult(10000) != WL_CONNECTED)
    {
        wifiConnecting = 0;
        return false;
    }
    WiFi.setAutoReconnect(true);
    if (savedSlot >= 0)
    {
        if (wifiInfor_list[savedSlot].password != password)
            wifiInfor_list[savedSlot].password = password;
        wifiInfor_list[savedSlot].connectedCount += 1;
        staWifiIndex = savedSlot;
        wifiInfor_list[savedSlot].available = 1;
    }
    else
    {
        notSavedSlot = (emptySlot >= 0) ? emptySlot : leastConnectedSlot;
        wifiInfor_list[notSavedSlot].ssid = ssid;
        wifiInfor_list[notSavedSlot].password = password;
        wifiInfor_list[notSavedSlot].connectedCount = 1;
        wifiInfor_list[notSavedSlot].available = 1;
        staWifiIndex = notSavedSlot;
    }
#if (DEBUG_SERIAL == 1)
    debugSerial.println();
    debugSerial.printf("Connected to SSID: %s\r\n", ssid.c_str());
    debugSerial.printf("IP: %s\r\n", WiFi.localIP().toString().c_str());
    debugSerial.printf("Current slot: %d\r\n", staWifiIndex);
    debugSerial.printf("Empty slot: %d\r\n", emptySlot);
    debugSerial.printf("Least connected slot: %d\r\n", leastConnectedSlot);
#endif
    return true;
}

bool wifi_delete(String ssid)
{
    for (uint8_t wifiScan = 0; wifiScan < WIFI_BUF_NUMBS; wifiScan++)
    {
    }
    return false;
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
        wifiInfor_list[i].available = 0;
    }
    WiFi.disconnect(false, true);
    WiFi.softAP(APssid.c_str(), APpassword.c_str());
#if (DEBUG_SERIAL == 1)
    debugSerial.println();
    debugSerial.println(F("Set wifi to default"));
#endif
}

bool wifi_isConnecting(void)
{
    return wifiConnecting;
}

void onStart_OTAHandle(void)
{
    if (ArduinoOTA.getCommand() == U_FLASH)
    {
        OTA_type = F("sketch");
    }
    else
    {
        OTA_type = F("filesystem");
        SPIFFS.end();
    }
#if (DEBUG_SERIAL == 1)
    debugSerial.println("Start updating " + OTA_type);
#endif
}

void onEnd_OTAHandle(void)
{
    if (OTA_type == F("filesystem"))
        SPIFFS.begin();
#if (DEBUG_SERIAL == 1)
    debugSerial.println(F("End update"));
#endif
}

void onProcess_OTAHandle(unsigned int progress, unsigned int total)
{
    static unsigned int lastPercent = 0;
    unsigned int percent = (progress / (total / 100));
    if (percent != lastPercent)
    {
#if (DEBUG_SERIAL == 1)
        debugSerial.printf("Progress: %u%%\r\n", percent);
#endif
        lastPercent = percent;
    }
}

void onError_OTAHandle(ota_error_t error)
{
#if (DEBUG_SERIAL == 1)
    debugSerial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR)
        debugSerial.println(F("Auth Failed"));
    else if (error == OTA_BEGIN_ERROR)
        debugSerial.println(F("Begin Failed"));
    else if (error == OTA_CONNECT_ERROR)
        debugSerial.println(F("Connect Failed"));
    else if (error == OTA_RECEIVE_ERROR)
        debugSerial.println(F("Receive Failed"));
    else if (error == OTA_END_ERROR)
        debugSerial.println(F("End Failed"));
#endif
}
