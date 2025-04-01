#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <WiFi.h>
#include "time.h"

class WifiManager{
    public:    
        WifiManager();
        int getTime();
        void initWifiConnection(char ssid[], char password[], char ntpServer[], long gmtOffset_sec, int daylightOffset_sec);
};

#endif