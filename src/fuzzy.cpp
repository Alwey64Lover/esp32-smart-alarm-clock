#include "fuzzy.h"

float lightMembership[FUZZY_SETS] = {0.0f, 0.0f, 0.0f};
float distanceMembership[FUZZY_SETS] = {0.0f, 0.0f, 0.0f};

int fuzzyRules[FUZZY_RULES][FUZZY_SETS] = {
  {LOW, LIGHT_DARK, DISTANCE_NEAR, HIGH_ALARM},
  {LOW, LIGHT_DARK, DISTANCE_MEDIUM, HIGH_ALARM},
  {LOW, LIGHT_DARK, DISTANCE_FAR, HIGH_ALARM},
  {LOW, LIGHT_MEDIUM, DISTANCE_NEAR, HIGH_ALARM},
  {LOW, LIGHT_MEDIUM, DISTANCE_MEDIUM, HIGH_ALARM},
  {LOW, LIGHT_MEDIUM, DISTANCE_FAR, LOW_ALARM},
  {LOW, LIGHT_BRIGHT, DISTANCE_NEAR, LOW_ALARM},
  {LOW, LIGHT_BRIGHT, DISTANCE_MEDIUM, LOW_ALARM},
  {LOW, LIGHT_BRIGHT, DISTANCE_FAR, NO_ALARM},

  {HIGH, LIGHT_DARK, DISTANCE_NEAR, HIGH_ALARM},
  {HIGH, LIGHT_DARK, DISTANCE_MEDIUM, HIGH_ALARM},
  {HIGH, LIGHT_DARK, DISTANCE_FAR, HIGH_ALARM},
  {HIGH, LIGHT_MEDIUM, DISTANCE_NEAR, HIGH_ALARM},
  {HIGH, LIGHT_MEDIUM, DISTANCE_MEDIUM, LOW_ALARM},
  {HIGH, LIGHT_MEDIUM, DISTANCE_FAR, NO_ALARM},
  {HIGH, LIGHT_BRIGHT, DISTANCE_NEAR, LOW_ALARM},
  {HIGH, LIGHT_BRIGHT, DISTANCE_MEDIUM, NO_ALARM},
  {HIGH, LIGHT_BRIGHT, DISTANCE_FAR, NO_ALARM}
};

float mapToLux(int analogValue) {
  int mappedValue = map(analogValue, 0, 4095, 0, 1023); // convert esp32 12-bit to Arduino 10-bit (for more accurate lux calculation)
  // Serial.print("map: ");
  // Serial.println(mappedValue);
  float voltage = float(mappedValue) / 1024.0 * 5.0;                          //
  float resistance = 2000.0 * voltage / (1.0 - voltage / 5.0);                // convert read value to lux unit
  float lux = pow(RL10 * 1e3 * pow(10.0, GAMMA) / resistance, (1.0 / GAMMA)); //

  if (!isfinite(lux)) lux=6000; // lux reaches infinite state at around value of 6000

  return lux;
}

// Old calculateLightMembership
// void calculateLightMembership(float lux) {
//   lightMembership[LIGHT_DARK] = max(0.0f, min(1.0f, (100.0f - lux) / 99.9f));
//   lightMembership[LIGHT_MEDIUM] = max(0.0f, min(1.0f, (lux - 50.0f) / 4950.0f));
//   lightMembership[LIGHT_MEDIUM] = min(lightMembership[LIGHT_MEDIUM], (10000.0f - lux) / 5000.0f);
//   lightMembership[LIGHT_BRIGHT] = max(0.0f, min(1.0f, (lux - 5000.0f) / 95000.0f));
// }

void calculateLightMembership(float lux) {
  const float MAX_LUX = 311.64f;

  lightMembership[LIGHT_DARK] = max(0.0f, min(1.0f, (MAX_LUX - lux) / MAX_LUX));

  // LIGHT_MEDIUM: memiliki distribusi membership dalam rentang tertentu
  // Misalnya, LIGHT_MEDIUM mulai naik ketika lux > 10 dan turun ketika lux > 200
  float mediumStart = 10.0f;
  float mediumPeak = 100.0f;
  float mediumEnd = 200.0f;

  if (lux <= mediumStart || lux >= mediumEnd) {
      lightMembership[LIGHT_MEDIUM] = 0.0f;
  } else if (lux <= mediumPeak) {
      lightMembership[LIGHT_MEDIUM] = (lux - mediumStart) / (mediumPeak - mediumStart);
  } else {
      lightMembership[LIGHT_MEDIUM] = (mediumEnd - lux) / (mediumEnd - mediumPeak);
  }

  // LIGHT_BRIGHT: jika lux > 50 mulai naik, jika lux >= MAX_LUX seharusnya 0
  float brightStart = 50.0f;
  if (lux <= brightStart) {
      lightMembership[LIGHT_BRIGHT] = 0.0f;
  } else {
      lightMembership[LIGHT_BRIGHT] = max(0.0f, min(1.0f, (lux - brightStart) / (MAX_LUX - brightStart)));
  }
}


void calculateDistanceMembership(float distanceValue) {
  // const float NO_OBJECT_THRESHOLD = 40.0f;  
  
  // Distance Near: (0 - 40 cm)
  if (distanceValue <= 120.0f) {
      distanceMembership[DISTANCE_NEAR] = 1.0f;
  } else if (distanceValue > 120.0f && distanceValue <= 180.0f) {
      distanceMembership[DISTANCE_NEAR] = (180 - distanceValue) / (180 - 120);
  } else {
      distanceMembership[DISTANCE_NEAR] = 0.0f;
  }

  // Distance Medium: 40 cm - 110 cm
  if (distanceValue < 100.0f || distanceValue > 300.0f) {
      distanceMembership[DISTANCE_MEDIUM] = 0.0f;
  } else if (distanceValue >= 100 && distanceValue <= 180.0f) {
      distanceMembership[DISTANCE_MEDIUM] = (distanceValue - 100.0f) / (180.0f - 100.0f);
  } else if (distanceValue >= 180 && distanceValue <= 250) {
    distanceMembership[DISTANCE_MEDIUM] = 1;
  }
  else {
      distanceMembership[DISTANCE_MEDIUM] = (300.0f - distanceValue) / (300.0f - 250.0f);
  }

  // Distance Far: If object is beyond 110 cm
  if (distanceValue < 250.0f) {
    distanceMembership[DISTANCE_FAR] = 0;
  } else if (distanceValue >= 250.0f && distanceValue <= 300.0f) {
      distanceMembership[DISTANCE_FAR] = (distanceValue - 250)/(300-250);
  } else if (distanceValue >= 350.0f) {
      distanceMembership[DISTANCE_FAR] = 1;
  }

  // if (distanceValue > NO_OBJECT_THRESHOLD) {
  //     distanceMembership[DISTANCE_NEAR] = 0.0f;
  // }
}


int applyFuzzyRules(int motionValue) {
  float actionStrength[FUZZY_SETS] = {0.0f, 0.0f, 0.0f};

  for (int i = 0; i < FUZZY_RULES; i++) {
    int motionRule = fuzzyRules[i][0];
    int lightSet = fuzzyRules[i][1];
    int distanceSet = fuzzyRules[i][2];
    int actionSet = fuzzyRules[i][3];

    if (motionValue == motionRule) {
      float strength = min(lightMembership[lightSet], distanceMembership[distanceSet]);
      actionStrength[actionSet] += strength;
    }
    
  }

  float numerator = 0.0f;
  float denominator = 0.0f;

  for (int i = 0; i < FUZZY_SETS; i++) {
    numerator += actionStrength[i] * i;
    denominator += actionStrength[i];
  }

  if (denominator == 0) return NO_ALARM;

  int action = round(numerator / denominator);
  return action;
}