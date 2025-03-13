#ifndef FUZZY_H
#define FUZZT_H

#include "config.h"

#define FUZZY_SETS 3
#define FUZZY_RULES 6

#define LIGHT_DARK 0
#define LIGHT_MEDIUM 1
#define LIGHT_BRIGHT 2

#define NO_ALARM 0
#define LOW_ALARM 1
#define HIGH_ALARM 2

float mapToLux(int analogValue);
void calculateLightMembership(float luxValue);
int applyFuzzyRules(int motionValue);

#endif