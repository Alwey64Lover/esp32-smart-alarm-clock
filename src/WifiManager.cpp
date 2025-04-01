#include "WifiManager.h"

WifiManager::WifiManager(){

}

void WifiManager::initWifiConnection(char ssid[], char password[], char ntpServer[], long gmtOffset_sec, int daylightOffset_sec){
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password, 6);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println(WiFi.localIP());

    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

int WifiManager::getTime() {
    struct tm timeinfo;
  
    if (!getLocalTime(&timeinfo)) {
    
        return 0;
    }

    return timeinfo.tm_hour * 100 + timeinfo.tm_min;
  }