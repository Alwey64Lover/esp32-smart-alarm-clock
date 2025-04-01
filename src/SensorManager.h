    #ifndef SENSORMANAGER_H
#define SENSORMANAGER_H

#include <Arduino.h>

class SensorManager{
    public:    
        float GAMMA, RL10;
        int pirPin, ldrPin, trigPin, echoPin;

        SensorManager(int pirPin, int ldrPin, int trigPin, int echoPin);
        float mapToLux(int analogValue);
        float getDistance();
        float getLight();
        int getMotion();
};

#endif