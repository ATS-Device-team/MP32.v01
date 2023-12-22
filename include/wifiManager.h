#ifndef __WIFIMANAGER_H__
#define __WIFIMANAGER_H__

#include "main.h"

#define AP_SSID_DEFAULT "ATS Simulator"
#define AP_PASSWORD_DEFAULT "12345678"

typedef struct wifiInfor_st
{
    String ssid = "";
    String password = "";
    uint32_t connectedCount = 0;
    int8_t rssi = -100;
    bool available = 0;
} wifiInfor_t;

void wifi_init(void);
void wifi_loadInfor(void);
void wifi_saveInfor(void);
int wifi_scan(void);
bool wifi_connect(String ssid, String password);
void wifi_default(void);

extern wifiInfor_t wifiInfor_list[];
extern int8_t staWifiIndex;
extern String APssid;
extern String APpassword;

#endif
