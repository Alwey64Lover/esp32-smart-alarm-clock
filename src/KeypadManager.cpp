#include "KeypadManager.h"

KeypadManager::KeypadManager(Keypad& keypad, TM1637Display& display, AlarmManager& alarm)
: keypad(keypad), display(display), alarm(alarm) {} 

void KeypadManager::nextChar(char key) {
    if (valIndex < 4 && 
      !(valIndex == 2 && key - '0' > 5) && 
      !(valIndex == 0 && key - '0' > 2) && 
      !(valIndex == 1 && entered_value == 2000 && key - '0' > 3)) {
      entered_value += (key - '0') * pow(10, 3 - valIndex);
      valIndex++;
    }
}

void KeypadManager::enterAlarm() {
    state = 1;
}
  
void KeypadManager::turnOffAlarm() {
    alarm.buzzerToggle = false;
}

void KeypadManager::removeAlarm(){
    state = 2;
    alarm.alarm_itr = alarm.alarm_set.begin();
}
  
  void KeypadManager::keyPadState2(){
    char key = keypad.getKey();
  
    switch (key){
      case 'A':
        alarm.deleteAlarm(alarm.alarm_itr);
        state = 0;
        display.setSegments(SEG_DONE);
        delay(1000);
        break;
      case 'B':
        state = 0;
        break;
      case 'C':
        if (alarm.alarm_itr != std::prev(alarm.alarm_set.end()))
        alarm.alarm_itr++;
        Serial.println("moving up");
        break;
      case 'D':
        if (alarm.alarm_itr != alarm.alarm_set.begin())
        alarm.alarm_itr--;
        Serial.println("moving down");
        break;
    } 
  }
  
  void KeypadManager::keyPadState1() {
    char key = keypad.getKey();
  
    switch (key) {
        case 'A':
            alarm.setAlarm(entered_value);
            state = 0;
            valIndex = 0;
            entered_value = 0;
            display.setSegments(SEG_DONE);
            delay(1000);
            break;
        case 'B':
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
  
  void KeypadManager::keyPadState0() {
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
  
  void KeypadManager::getInput() {
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