#include "SensorManager.h"

SensorManager::SensorManager(int pirPin, int ldrPin, int trigPin, int echoPin){
    GAMMA = 1.2;
    RL10 = 20;

    this->pirPin = pirPin;
    this->ldrPin = ldrPin;
    this->trigPin = trigPin;
    this->echoPin = echoPin;
}

float SensorManager::mapToLux(int analogValue) {
    int mappedValue = map(analogValue, 0, 4095, 0, 1023); // convert esp32 12-bit to Arduino 10-bit (for more accurate lux calculation)
    float voltage = float(mappedValue) / 1024.0 * 5.0;                          //
    float resistance = 2000.0 * voltage / (1.0 - voltage / 5.0);                // convert read value to lux unit
    float lux = pow(RL10 * 1e3 * pow(10.0, GAMMA) / resistance, (1.0 / GAMMA)); //
  
    if (!isfinite(lux)) lux=6000; // lux reaches infinite state at around value of 6000

    return lux;
}

float SensorManager::getDistance(){
    // Ultrasonic
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // Read the echo pin
    long duration = pulseIn(echoPin, HIGH);

    // Convert to distance (speed of sound = 343 m/s = 0.0343 cm/µs)
    return (duration * 0.0343) / 2;;
}

float SensorManager::getLight(){
    int lightValue = analogRead(ldrPin);
    float lux = mapToLux(lightValue);

    return lux;
}

int SensorManager::getMotion(){
    int motionValue = digitalRead(pirPin); 
    return motionValue;
}