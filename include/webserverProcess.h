#ifndef __WEBSERVERPROCESS_H__
#define __WEBSERVERPROCESS_H__

#include "main.h"

void webserver_init(void);
void webserver_process(uint32_t tSec);
bool webserver_isWifiScanning(void);

#endif
