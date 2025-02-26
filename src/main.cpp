#include <WiFi.h>
#include "time.h"
#include <TM1637Display.h>
#include <Keypad.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7 * 3600;
const int   daylightOffset_sec = 0;
int state=0;

const uint8_t rows=4, cols=4;
char keys[rows][cols] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

uint8_t rowPins[rows] = {23, 32, 33, 25};
uint8_t colPins[cols] = {26, 27, 14, 13};

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, rows, cols);
TM1637Display display(16, 17);

void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }

  int hrs = timeinfo.tm_hour;
  int mins = timeinfo.tm_min;
  int time = hrs*100 + mins;

  // Serial.print(hrs);
  // Serial.print(':');
  // Serial.println(mins);

  if (time <= 730) {
    digitalWrite(18, HIGH); // Turn on buzzer
    digitalWrite(18, LOW);
    Serial.println("Buzzer ON");
  }

  display.clear();
  display.setBrightness(3);
  display.showNumberDec(time);
}

void enterAlarm(){
  
}

void enterInput(){
  char key = keypad.getKey();
  
  switch(key)
  {
    case 'A':
      enterAlarm();
      break;
  }
}

void setup(){
  pinMode(18, OUTPUT);
  digitalWrite(18, LOW);

  Serial.begin(115200);

  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  
  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

void loop(){
  delay(100);
  if (state == 0) printLocalTime();
  enterInput();
}