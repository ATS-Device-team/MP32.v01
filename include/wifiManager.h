#ifndef __WIFIMANAGER_H__
#define __WIFIMANAGER_H__

#include "main.h"

#define WIFI_BUF_NUMBS 10
#define AP_SSID_DEFAULT "ATS Simulator"
#define AP_PASSWORD_DEFAULT ""

typedef struct wifiInfor_st
{
    String ssid = "";
    String password = "";
    uint32_t connectedCount = 0;
    bool available = 0;
} wifiInfor_t;

void wifi_init(void);
void wifi_process(void);
void wifi_loadInfor(void);
void wifi_saveInfor(void);
int wifi_scan(void);
bool wifi_setAP(String ssid, String password);
bool wifi_connect(String ssid, String password);
bool wifi_delete(String ssid);
void wifi_default(void);
bool wifi_isConnecting(void);

extern wifiInfor_t wifiInfor_list[];
extern int8_t staWifiIndex;
extern String APssid;
extern String APpassword;

#endif
