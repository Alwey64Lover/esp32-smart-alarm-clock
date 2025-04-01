#ifndef KEYPADMANAGER_H
#define KEYPADMANAGER_H

#include "config.h"
#include "AlarmManager.h"

class KeypadManager{
    public:
        int state, valIndex, entered_value;
        Keypad& keypad;
        TM1637Display& display;
        AlarmManager& alarm;
    
        KeypadManager(Keypad& keypad, TM1637Display& display, AlarmManager& alarm);
        void nextChar(char key);
        void enterAlarm();
        void turnOffAlarm();
        void removeAlarm();
        void keyPadState2();
        void keyPadState1();
        void keyPadState0();
        void getInput();
};

#endif