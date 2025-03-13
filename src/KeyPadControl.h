#ifndef KEYPADCONTROL_H
#define KEYPADCONTROL_H

#include "config.h"
#include "MyAlarm.h"

class KeyPadControl{
    public:
        int state, valIndex, entered_value;
        Keypad& keypad;
        TM1637Display& display;
        MyAlarm& alarm;
    
        KeyPadControl(Keypad& keypad, TM1637Display& display, MyAlarm& alarm);
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