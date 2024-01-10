#include "wifiManager.h"
#include "WiFi.h"
#include "ArduinoJson.h"
#include "SPIFFS.h"

#define WIFI_BUF_NUMBS 10

const char *name_wifiInfor PROGMEM = "/WIFI_infor.js";
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

void wifi_init(void)
{
    WiFi.mode(WIFI_MODE_APSTA);
    WiFi.softAP(APssid.c_str(), APpassword.c_str());
#if (DEBUG_SERIAL == 1)
    debugSerial.println();
    debugSerial.printf("AP SSID: %s\r\n", APssid.c_str());
    debugSerial.printf("AP Password: %s\r\n", APpassword.c_str());
#endif
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
    debugSerial.println("Load wifi infor:");
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
    debugSerial.println("Save wifi infor:");
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
    debugSerial.println("Scan done");
    if (networksNumb == 0)
    {
        debugSerial.println("no networks found");
    }
    else
    {
        debugSerial.print(networksNumb);
        debugSerial.println(" networks found");
        debugSerial.println("Nr | SSID                             | RSSI | CH | Encryption");
        for (int i = 0; i < networksNumb; ++i)
        {
            // Print SSID and RSSI for each network found
            debugSerial.printf("%2d", i + 1);
            debugSerial.print(" | ");
            debugSerial.printf("%-32.32s", WiFi.SSID(i).c_str());
            debugSerial.print(" | ");
            debugSerial.printf("%4d", WiFi.RSSI(i));
            debugSerial.print(" | ");
            debugSerial.printf("%2d", WiFi.channel(i));
            debugSerial.print(" | ");
            switch (WiFi.encryptionType(i))
            {
            case WIFI_AUTH_OPEN:
                debugSerial.print("open");
                break;
            case WIFI_AUTH_WEP:
                debugSerial.print("WEP");
                break;
            case WIFI_AUTH_WPA_PSK:
                debugSerial.print("WPA");
                break;
            case WIFI_AUTH_WPA2_PSK:
                debugSerial.print("WPA2");
                break;
            case WIFI_AUTH_WPA_WPA2_PSK:
                debugSerial.print("WPA+WPA2");
                break;
            case WIFI_AUTH_WPA2_ENTERPRISE:
                debugSerial.print("WPA2-EAP");
                break;
            case WIFI_AUTH_WPA3_PSK:
                debugSerial.print("WPA3");
                break;
            case WIFI_AUTH_WPA2_WPA3_PSK:
                debugSerial.print("WPA2+WPA3");
                break;
            case WIFI_AUTH_WAPI_PSK:
                debugSerial.print("WAPI");
                break;
            default:
                debugSerial.print("unknown");
            }
            debugSerial.println();
            delay(10);
        }
    }
#endif
    if (networksNumb == 0)
        return -1;
#if (DEBUG_SERIAL == 1)
    debugSerial.println("WiFi available:");
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
        debugSerial.println("AP config failed");
#endif
        return false;
    }
}

bool wifi_connect(String ssid, String password)
{
    uint8_t wifiScanIndex;
    int8_t slotToWriteIndex = -1, emptySlotIndex = -1;
    uint32_t leastConnetcedCount = 0;
    bool wifiSaved = 0;

    wifiConnecting = 1;
    WiFi.begin(ssid.c_str(), password.c_str());
    if (WiFi.waitForConnectResult(10000) != WL_CONNECTED)
    {
        wifiConnecting = 0;
        return false;
    }
    WiFi.setAutoReconnect(true);
    for (wifiScanIndex = 0; wifiScanIndex < WIFI_BUF_NUMBS; wifiScanIndex++)
    {
        if (wifiInfor_list[wifiScanIndex].ssid.length() == 0)
        {
            if (emptySlotIndex < 0)
                emptySlotIndex = wifiScanIndex;
            continue;
        }
        if (wifiInfor_list[wifiScanIndex].ssid == ssid)
        {
            staWifiIndex = wifiScanIndex;
            wifiInfor_list[wifiScanIndex].connectedCount += 1;
            wifiInfor_list[wifiScanIndex].available = true;
            wifiSaved = 1;
            break;
        }
        else
        {
            if (wifiInfor_list[wifiScanIndex].connectedCount < leastConnetcedCount)
            {
                leastConnetcedCount = wifiInfor_list[wifiScanIndex].connectedCount;
                slotToWriteIndex = wifiScanIndex;
            }
        }
    }
    if (!wifiSaved)
    {
        if (emptySlotIndex >= 0)
            slotToWriteIndex = emptySlotIndex;
        wifiInfor_list[slotToWriteIndex].ssid = ssid;
        wifiInfor_list[slotToWriteIndex].password = password;
        wifiInfor_list[slotToWriteIndex].connectedCount = 1;
        wifiInfor_list[slotToWriteIndex].available = true;
        staWifiIndex = slotToWriteIndex;
    }
    wifiConnecting = 0;
#if (DEBUG_SERIAL == 1)
    debugSerial.println();
    debugSerial.printf("Connected to SSID: %s\r\n", ssid.c_str());
    debugSerial.printf("IP: %s\r\n", WiFi.localIP().toString().c_str());
    debugSerial.printf("Slot: %u\r\n", staWifiIndex);
#endif

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
