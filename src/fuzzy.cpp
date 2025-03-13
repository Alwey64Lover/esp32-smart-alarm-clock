#include "fuzzy.h"

float lightMembership[FUZZY_SETS] = {0.0f, 0.0f, 0.0f};

int fuzzyRules[6][3] = {
  {LOW, LIGHT_DARK, HIGH_ALARM},
  {LOW, LIGHT_MEDIUM, HIGH_ALARM},
  {LOW, LIGHT_BRIGHT, LOW_ALARM},   
  {HIGH, LIGHT_DARK, HIGH_ALARM},
  {HIGH, LIGHT_MEDIUM, LOW_ALARM},
  {HIGH, LIGHT_BRIGHT, NO_ALARM}    
};

float mapToLux(int analogValue) {
    return map(analogValue, 4063, 32, 0.1, 100000.0);
  }

void calculateLightMembership(float luxValue) {
    lightMembership[LIGHT_DARK] = max(0.0f, min(1.0f, (100.0f - luxValue) / 99.9f));
  
    lightMembership[LIGHT_MEDIUM] = max(0.0f, min(1.0f, (luxValue - 50.0f) / 4950.0f));
    lightMembership[LIGHT_MEDIUM] = min(lightMembership[LIGHT_MEDIUM], (10000.0f - luxValue) / 5000.0f);
  
    lightMembership[LIGHT_BRIGHT] = max(0.0f, min(1.0f, (luxValue - 5000.0f) / 95000.0f));
  
    Serial.print("Light Membership - DARK: ");
    Serial.print(lightMembership[LIGHT_DARK]);
    Serial.print(", MEDIUM: ");
    Serial.print(lightMembership[LIGHT_MEDIUM]);
    Serial.print(", BRIGHT: ");
    Serial.println(lightMembership[LIGHT_BRIGHT]);
}

int applyFuzzyRules(int motionValue) {
    float actionStrength[FUZZY_SETS] = {0.0f, 0.0f, 0.0f};
  
    for (int i = 0; i < FUZZY_RULES; i++) {
      int motionRule = fuzzyRules[i][0];
      int lightSet = fuzzyRules[i][1];
      int actionSet = fuzzyRules[i][2];
  
      if (motionValue == motionRule) {
        float strength = lightMembership[lightSet];
        actionStrength[actionSet] = max(actionStrength[actionSet], strength);
      }
    }
  
    Serial.print("Action Strength - NO_ALARM: ");
    Serial.print(actionStrength[NO_ALARM]);
    Serial.print(", LOW_ALARM: ");
    Serial.print(actionStrength[LOW_ALARM]);
    Serial.print(", HIGH_ALARM: ");
    Serial.println(actionStrength[HIGH_ALARM]);
  
    float numerator = 0.0f;
    float denominator = 0.0f;
  
    for (int i = 0; i < FUZZY_SETS; i++) {
      numerator += actionStrength[i] * i;
      denominator += actionStrength[i];
    }
  
    if (denominator == 0) return NO_ALARM;
  
    int action = round(numerator / denominator);
  
    Serial.print("Final Action: ");
    Serial.println(action);
  
    return action;
  }