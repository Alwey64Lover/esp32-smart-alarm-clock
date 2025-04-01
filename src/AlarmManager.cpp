#include "AlarmManager.h"

AlarmManager::AlarmManager(){
    alarm_itr = alarm_set.begin();
    buzzerToggle = false;
}

void AlarmManager::controlBuzzer(int alarmAction) {
    switch (alarmAction) {
      case NO_ALARM:
        digitalWrite(18, LOW); 
        Serial.println("NO");
        break;
      case LOW_ALARM:
        digitalWrite(18, HIGH);
        delay(500);
        digitalWrite(18, LOW);
        delay(500);
        Serial.println("LOW");
        break;
      case HIGH_ALARM:
        digitalWrite(18, HIGH);
        delay(100);
        digitalWrite(18, LOW);
        delay(100);
        Serial.println("HIGH");
        break;
    }
  }

void AlarmManager::setAlarm(int val) {

    alarm_set.insert(val);
  
    Serial.print("All alarms: ");
    for (int i : alarm_set) {
      Serial.print(i);
      Serial.print(" ");
    }
    Serial.println();

    alarm_itr = alarm_set.begin();
}

void AlarmManager::deleteAlarm(std::set<int>::iterator itr){
    alarm_set.erase(itr);
    alarm_itr = alarm_set.begin();
  }