#ifndef ALARMMANAGER_H
#define ALARMMANAGER_H

#include <set>
#include "config.h"
#include "fuzzy.h"

class AlarmManager{
    public:
        std::set<int> alarm_set;
        std::set<int>::iterator alarm_itr;
        bool buzzerToggle;
    
        AlarmManager();
        void controlBuzzer(int alarmAction);
        void setAlarm(int val);
        void deleteAlarm(std::set<int>::iterator itr);
};

#endif