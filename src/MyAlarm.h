#ifndef MYALARM_H
#define MYALARM_H

#include <set>
#include "config.h"
#include "fuzzy.h"

class MyAlarm{
    public:
        std::set<int> alarm_set;
        std::set<int>::iterator alarm_itr;
        bool buzzerToggle;
    
        MyAlarm();
        void controlBuzzer(int alarmAction);
        void setAlarm(int val);
        void deleteAlarm(std::set<int>::iterator itr);
};

#endif