#include "config.h"
#include "MyAlarm.h"
#include "KeyPadControl.h"

const char* ssid = "Wokwi-GUEST";
const char* password = "";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7 * 3600;
const int   daylightOffset_sec = 0;

const int pirPin = 19;  
const int ldrPin = 34;  
  
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

MyAlarm myAlarm;
KeyPadControl keyPadControl(keypad, display, myAlarm);

// WebServer server(80);

// void handleAlarmTrigger() {
//   digitalWrite(18, HIGH);  
//   delay(500);
//   digitalWrite(18, LOW);
//   Serial.println("server connection");
//   server.send(200, "text/html", "Alarm Triggered");
// }

void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }

  int hrs = timeinfo.tm_hour;
  int mins = timeinfo.tm_min;
  int time = hrs * 100 + mins;

  if (myAlarm.alarm_set.find(time) != myAlarm.alarm_set.end()) {
    int motionValue = digitalRead(pirPin); 
    int lightValue = analogRead(ldrPin);  
    Serial.println(lightValue);

    float luxValue = mapToLux(lightValue);

    Serial.print("Motion: ");
    Serial.print(motionValue);
    Serial.print(", Light (lux): ");
    Serial.println(luxValue);

    calculateLightMembership(luxValue);

    int alarmAction = applyFuzzyRules(motionValue);

    myAlarm.controlBuzzer(alarmAction);
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

  Serial.begin(115200);

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
  printLocalTime();

  // server.on("/trigger_alarm", handleAlarmTrigger);
  // server.begin();
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

void loop() {
  unsigned long currentMillis = millis();
  // Serial.println(*myAlarm.alarm_itr);

  if (currentMillis - lastPrintTime >= 1000){
    if (keyPadControl.state == 0){
        // Run every 1 second
        lastPrintTime = currentMillis;
        printLocalTime();
      
    }
    else if (keyPadControl.state == 1){
      display.showNumberDec(keyPadControl.entered_value / pow(10, 4 - keyPadControl.valIndex));
    }
    else{
      display.showNumberDec(*myAlarm.alarm_itr);
    }
  }
  keyPadControl.getInput();

  delay(200);
}