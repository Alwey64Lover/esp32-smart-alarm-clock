#include <WiFi.h>
#include "time.h"
#include <TM1637Display.h>
#include <Keypad.h>
#include <set>

const char* ssid = "Wokwi-GUEST";
const char* password = "";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7 * 3600;
const int   daylightOffset_sec = 0;

const int pirPin = 19;  // Sensor PIR (motion)
const int ldrPin = 34;  // Sensor LDR (cahaya)
const int trigPin = 5;
const int echoPin = 4;

const uint8_t SEG_ADD[] = {
	SEG_A | SEG_B | SEG_C | SEG_E| SEG_F | SEG_G,    // A
	SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           // d
	SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           // d
  0x00
	},
  SEG_DEL[] = {
	  SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           // d
	  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,           // E
    SEG_D | SEG_E | SEG_F,                           // L 
    0x00
  },
  SEG_DONE[] = {
    SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           // d
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
    SEG_C | SEG_E | SEG_G,                           // n
    SEG_A | SEG_D | SEG_E | SEG_F | SEG_G            // E
    };;
  

int state = 0, valIndex = 0;
int entered_value = 0;
unsigned long lastPrintTime = 0;
unsigned long buzzerStartTime = 0;
bool buzzerActive = false;
int activeAlarm = -1; // Track the currently active alarm

std::set<int> alarm_set;
std::set<int>::iterator alarm_itr;

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

// Fuzzy Logic Definitions
#define FUZZY_SETS 3
#define FUZZY_RULES 6

// Fuzzy Sets for Light (LDR)
#define LIGHT_DARK 0
#define LIGHT_MEDIUM 1
#define LIGHT_BRIGHT 2

// Fuzzy Sets for Action (Alarm)
#define NO_ALARM 0
#define LOW_ALARM 1
#define HIGH_ALARM 2

// Membership functions for Light
float lightMembership[FUZZY_SETS] = {0.0f, 0.0f, 0.0f};

// Fuzzy rules
int fuzzyRules[6][3] = {
  {LOW, LIGHT_DARK, HIGH_ALARM},
  {LOW, LIGHT_MEDIUM, HIGH_ALARM},
  {LOW, LIGHT_BRIGHT, LOW_ALARM},   
  {HIGH, LIGHT_DARK, HIGH_ALARM},
  {HIGH, LIGHT_MEDIUM, LOW_ALARM},
  {HIGH, LIGHT_BRIGHT, NO_ALARM}    
};

// Function to map analog read value to lux
float mapToLux(int analogValue) {
  // Analog read value ranges from 32 (100,000 lux) to 4063 (0.1 lux)
  // Invert the mapping: higher analog value = lower lux
  return map(analogValue, 4063, 32, 0.1, 100000.0);
}

// Function to calculate membership for Light
void calculateLightMembership(float luxValue) {
  // Calculate membership for DARK (0.1 lux to 100 lux)
  lightMembership[LIGHT_DARK] = max(0.0f, min(1.0f, (100.0f - luxValue) / 99.9f));

  // Calculate membership for MEDIUM (50 lux to 10,000 lux)
  lightMembership[LIGHT_MEDIUM] = max(0.0f, min(1.0f, (luxValue - 50.0f) / 4950.0f));
  lightMembership[LIGHT_MEDIUM] = min(lightMembership[LIGHT_MEDIUM], (10000.0f - luxValue) / 5000.0f);

  // Calculate membership for BRIGHT (5,000 lux to 100,000 lux)
  lightMembership[LIGHT_BRIGHT] = max(0.0f, min(1.0f, (luxValue - 5000.0f) / 95000.0f));

  // Debug: Print light membership values
  Serial.print("Light Membership - DARK: ");
  Serial.print(lightMembership[LIGHT_DARK]);
  Serial.print(", MEDIUM: ");
  Serial.print(lightMembership[LIGHT_MEDIUM]);
  Serial.print(", BRIGHT: ");
  Serial.println(lightMembership[LIGHT_BRIGHT]);
}

// Function to apply fuzzy rules
int applyFuzzyRules(int motionValue) {
  float actionStrength[FUZZY_SETS] = {0.0f, 0.0f, 0.0f};

  for (int i = 0; i < FUZZY_RULES; i++) {
    int motionRule = fuzzyRules[i][0];
    int lightSet = fuzzyRules[i][1];
    int actionSet = fuzzyRules[i][2];

    // Check if the motion value matches the rule
    if (motionValue == motionRule) {
      float strength = lightMembership[lightSet];
      actionStrength[actionSet] = max(actionStrength[actionSet], strength);
    }
  }

  // Debug: Print action strengths
  Serial.print("Action Strength - NO_ALARM: ");
  Serial.print(actionStrength[NO_ALARM]);
  Serial.print(", LOW_ALARM: ");
  Serial.print(actionStrength[LOW_ALARM]);
  Serial.print(", HIGH_ALARM: ");
  Serial.println(actionStrength[HIGH_ALARM]);

  // Defuzzification (using centroid method)
  float numerator = 0.0f;
  float denominator = 0.0f;

  for (int i = 0; i < FUZZY_SETS; i++) {
    numerator += actionStrength[i] * i;
    denominator += actionStrength[i];
  }

  if (denominator == 0) return NO_ALARM;

  int action = round(numerator / denominator);

  // Debug: Print final action
  Serial.print("Final Action: ");
  Serial.println(action);

  return action;
}

// Function to control the buzzer
void controlBuzzer(int alarmAction) {
  switch (alarmAction) {
    case NO_ALARM:
      digitalWrite(18, LOW); // Turn off the buzzer
      Serial.println("NO");
      break;
    case LOW_ALARM:
      // Simulate short beeps (e.g., 200ms on, 200ms off)
      digitalWrite(18, HIGH);
      delay(500);
      digitalWrite(18, LOW);
      delay(500);
      Serial.println("LOW");
      break;
    case HIGH_ALARM:
      // Continuous beeping
      digitalWrite(18, HIGH);
      delay(100);
      digitalWrite(18, LOW);
      delay(100);
      Serial.println("HIGH");
      break;
  }
}

void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }

  int hrs = timeinfo.tm_hour;
  int mins = timeinfo.tm_min;
  int time = hrs * 100 + mins;

  // Check if the current time matches any alarm
  if (alarm_set.find(time) != alarm_set.end()) {
    int motionValue = digitalRead(pirPin); // Binary input (HIGH or LOW)
    int lightValue = analogRead(ldrPin);  // Analog input (32–4063)
    Serial.println(lightValue);

    // Convert analog read value to lux
    float luxValue = mapToLux(lightValue);

    // Debug: Print sensor values
    Serial.print("Motion: ");
    Serial.print(motionValue);
    Serial.print(", Light (lux): ");
    Serial.println(luxValue);

    // Calculate fuzzy membership for light
    calculateLightMembership(luxValue);

    // Apply fuzzy rules
    int alarmAction = applyFuzzyRules(motionValue);

    // Control the buzzer based on fuzzy logic output
    controlBuzzer(alarmAction);
  }

  // Display the current time
  display.clear();
  display.setBrightness(3);
  display.showNumberDec(time);
}

void nextChar(char key) {
  if (valIndex < 4 && 
    !(valIndex == 2 && key - '0' > 5) && 
    !(valIndex == 0 && key - '0' > 2) && 
    !(valIndex == 1 && entered_value == 2000 && key - '0' > 3)) {
    Serial.print("Entered value: ");
    Serial.println(key);
    entered_value += (key - '0') * pow(10, 3 - valIndex);
    valIndex++;
  }
}

void enterAlarm() {
  state = 1;
  Serial.println("Entering alarm mode. Please enter a 4-digit time (HHMM).");
}

void setAlarm() {

  alarm_set.insert(entered_value);
  state = 0;
  valIndex = 0;
  entered_value = 0;

  Serial.print("Alarm set: ");
  Serial.println(entered_value);

  Serial.print("All alarms: ");
  for (int i : alarm_set) {
    Serial.print(i);
    Serial.print(" ");
  }
  Serial.println();

  display.setSegments(SEG_DONE);
  delay(1000);
}

void turnOffAlarm() {
  if (buzzerActive) {
    buzzerActive = false;
    activeAlarm = -1; // Clear the active alarm
    digitalWrite(18, LOW); // Turn off the buzzer
    Serial.println("Alarm turned off.");
  }
}

void removeAlarm(){
  state = 2;
  alarm_itr = alarm_set.begin();
}

void deleteAlarm(){
  alarm_set.erase(alarm_itr);
  state = 0;
  display.setSegments(SEG_DONE);
  delay(1000);
}

void keyPadState2(){
  char key = keypad.getKey();

  switch (key){
    case 'A':
      deleteAlarm();
      break;
    case 'B':
      state = 0;
      break;
    case 'C':
      if (alarm_itr != std::prev(alarm_set.end()))
      alarm_itr++;
      Serial.println("moving up");
      break;
    case 'D':
      if (alarm_itr != alarm_set.begin())
      alarm_itr--;
      Serial.println("moving down");
      break;
  } 
}

void keyPadState1() {
  char key = keypad.getKey();

  switch (key) {
    case 'A':
      setAlarm();
      break;
    case 'B':
      Serial.println("Exiting alarm mode without setting.");
      state = 0;
      valIndex = 0;
      entered_value = 0;
      break;
    default:
      if (isDigit(key)) {
        nextChar(key);
      }
  }
}

void keyPadState0() {
  char key = keypad.getKey();

  switch (key) {
    case 'A':
      display.setSegments(SEG_ADD);
      delay(1000);
      enterAlarm();
      break;
    case 'B':
      display.setSegments(SEG_DEL);
      delay(1000);
      removeAlarm();
      break;
    case 'C':
      turnOffAlarm();
      break;
  }
}

void getInput() {
  switch (state) {
    case 0:
      keyPadState0();
      break;
    case 1:
      keyPadState1();
      break;
    case 2:
      keyPadState2();
      break;
  }
}

void setup() {
  pinMode(18, OUTPUT);
  digitalWrite(18, LOW);
  pinMode(pirPin, INPUT);  
  pinMode(ldrPin, INPUT);  
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
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

  // Disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

void loop() {
  unsigned long currentMillis = millis();

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

  // Print the result
  Serial.print("Distance: ");
  Serial.print(distance_cm);
  Serial.println(" cm");

  
  if (currentMillis - lastPrintTime >= 1000){
    if (state == 0){
        // Run every 1 second
        lastPrintTime = currentMillis;
        printLocalTime();
      
    }
    else if (state == 1){
      display.showNumberDec(entered_value / pow(10, 4 - valIndex));
    }
    else{
      display.showNumberDec(*alarm_itr);
    }
  }
  getInput();

  delay(200);
}