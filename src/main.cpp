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
  if (alarm_set.find(time) != alarm_set.end() && !buzzerActive) {
    buzzerActive = true;
    buzzerStartTime = millis();
    activeAlarm = time; // Set the active alarm
    digitalWrite(18, HIGH); // Turn on the buzzer
    Serial.println("Buzzer ON - Alarm triggered!");
  }

  // Turn off the buzzer after 500ms (simulate beeping)
  if (buzzerActive && (millis() - buzzerStartTime >= 500)) {
    digitalWrite(18, LOW); // Turn off the buzzer
    Serial.println("Buzzer OFF");
    if (activeAlarm != -1) {
      // Re-trigger the buzzer after 1 second (simulate beeping)
      buzzerStartTime = millis();
      digitalWrite(18, HIGH);
      Serial.println("Buzzer ON - Beeping...");
    }
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
}