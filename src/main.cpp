#include "config.h"
#include "AlarmManager.h"
#include "KeypadManager.h"
#include "WifiManager.h"
#include "SensorManager.h"

#include "blynkCreds.h"
#include <BlynkSimpleEsp32.h>

#define VIRTUAL_PIN_0 V0
#define VIRTUAL_PIN_1 V1
#define VIRTUAL_PIN_2 V2
#define VIRTUAL_PIN_3 V3
#define VIRTUAL_PIN_4 V4
#define VIRTUAL_PIN_5 V5
#define VIRTUAL_PIN_6 V6
#define VIRTUAL_PIN_7 V7
#define VIRTUAL_PIN_8 V8
#define VIRTUAL_PIN_9 V9

char ssid[] = "Wokwi-GUEST";
char password[] = "";

char ntpServer[] = "pool.ntp.org";
long  gmtOffset_sec = 7 * 3600;
int   daylightOffset_sec = 0;

int pirPin = 19;  
int ldrPin = 34;  
int trigPin = 5;
int echoPin = 4;
  
unsigned long lastPrintTime = 0;

const uint8_t rows = 4, cols = 4;
char keys[rows][cols] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

uint8_t rowPins[rows] = {23, 32, 33, 25};
uint8_t colPins[cols] = {26, 27, 14, 13};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, rows, cols);
TM1637Display display(16, 17);

WifiManager wifiManager;
SensorManager sensorManager(pirPin, ldrPin, trigPin, echoPin);
AlarmManager alarmManager;
KeypadManager keypadManager(keypad, display, alarmManager);

void runAlarm() {
  int time = wifiManager.getTime();
  float distance_cm = sensorManager.getDistance();
  int motionValue = sensorManager.getMotion(); 
  int lux = sensorManager.getLight();
  
  Blynk.virtualWrite(VIRTUAL_PIN_0,  motionValue);
  Blynk.virtualWrite(VIRTUAL_PIN_1,  lux);
  Blynk.virtualWrite(VIRTUAL_PIN_2,  distance_cm);
  
  calculateLightMembership(lux);

  Blynk.virtualWrite(VIRTUAL_PIN_3, lightMembership[LIGHT_DARK]);
  Blynk.virtualWrite(VIRTUAL_PIN_4, lightMembership[LIGHT_MEDIUM]);
  Blynk.virtualWrite(VIRTUAL_PIN_5, lightMembership[LIGHT_BRIGHT]);

  calculateDistanceMembership(distance_cm);

  Blynk.virtualWrite(VIRTUAL_PIN_6, distanceMembership[DISTANCE_NEAR]);
  Blynk.virtualWrite(VIRTUAL_PIN_7, distanceMembership[DISTANCE_MEDIUM]);
  Blynk.virtualWrite(VIRTUAL_PIN_8, distanceMembership[DISTANCE_FAR]);

  int alarmAction = applyFuzzyRules(motionValue);

  Blynk.virtualWrite(V9, alarmAction == 0 ? "NO ALARM" : (alarmAction == 1 ? "LOW ALARM" : "HIGH ALARM"));

  if (alarmManager.alarm_set.find(time) != alarmManager.alarm_set.end()) {  
    alarmManager.controlBuzzer(alarmAction);
  }

  display.clear();
  display.setBrightness(3);
  display.showNumberDec(time);
}

void setup() {
  pinMode(18, OUTPUT);
  digitalWrite(18, LOW);
  pinMode(pirPin, INPUT);  
  pinMode(ldrPin, INPUT);  
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.begin(115200);

  wifiManager.initWifiConnection(ssid, password, ntpServer, gmtOffset_sec, daylightOffset_sec);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
}

void loop() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - lastPrintTime >= 1000){
    if (keypadManager.state == 0){
        // Run every 1 second
        lastPrintTime = currentMillis;
        runAlarm();
      
    }
    else if (keypadManager.state == 1){
      display.showNumberDec(keypadManager.entered_value / pow(10, 4 - keypadManager.valIndex));
    }
    else{
      display.showNumberDec(*alarmManager.alarm_itr);
    }
  }
  keypadManager.getInput();

  Blynk.run();
  delay(200);
}