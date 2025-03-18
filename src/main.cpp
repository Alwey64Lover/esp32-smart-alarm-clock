#include "config.h"
#include "MyAlarm.h"
#include "KeyPadControl.h"
#include "blynkCreds.h"
#include <BlynkSimpleEsp32.h>

#define VIRTUAL_PIN_0 V0
#define VIRTUAL_PIN_1 V1
#define VIRTUAL_PIN_2 V2

const char* ssid = "Wokwi-GUEST";
const char* password = "";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7 * 3600;
const int   daylightOffset_sec = 0;

const int pirPin = 19;  
const int ldrPin = 34;  
const int trigPin = 5;
const int echoPin = 4;

const float GAMMA = 1.2;
const float RL10 = 20;
  
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
//   //Serial.println("server connection");
//   server.send(200, "text/html", "Alarm Triggered");
// }

void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return;
  }

  int hrs = timeinfo.tm_hour;
  int mins = timeinfo.tm_min;
  int time = hrs * 100 + mins;

  if (myAlarm.alarm_set.find(time) == myAlarm.alarm_set.end()) {
    // Ultrasonic
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // Read the echo pin
    long duration = pulseIn(echoPin, HIGH);

    // Convert to distance (speed of sound = 343 m/s = 0.0343 cm/µs)
    float distance_cm = (duration * 0.0343) / 2;

    Blynk.virtualWrite(VIRTUAL_PIN_2,  distance_cm);

    // Print the result
    //Serial.print("Distance: ");
    //Serial.print(distance_cm);
    //Serial.println(" cm");

    int motionValue = digitalRead(pirPin); 
    int lightValue = analogRead(ldrPin);  
    
    float voltage = float(lightValue) / 1024.0 * 5.0;
    float resistance = 2000.0 * voltage / (1.0 - voltage / 5.0);
    float lux = pow(RL10 * 1e3 * pow(10.0, GAMMA) / resistance, (1.0 / GAMMA));

    if(!isfinite(lux)) lux = -1;

    Serial.print("analog: ");
    Serial.println(lightValue);
    Serial.print("lux: ");
    Serial.println(lux);

    Blynk.virtualWrite(VIRTUAL_PIN_0,  motionValue);
    Blynk.virtualWrite(VIRTUAL_PIN_1,  lux);

    //Serial.print("Motion: ");
    //Serial.print(motionValue);
    //Serial.print(", Light (lux): ");
    //Serial.println(luxValue);

    calculateLightMembership(lux);
    calculateDistanceMembership(distance_cm);

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
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.begin(115200);

  //Serial.print("Connecting to ");
  //Serial.println(ssid);
  WiFi.begin(ssid, password, 6);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //Serial.print(".");
  }
  //Serial.println("");
  //Serial.println("WiFi connected.");
  //Serial.println(WiFi.localIP());

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
}

void loop() {
  unsigned long currentMillis = millis();
  
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

  Blynk.run();
  delay(200);
}