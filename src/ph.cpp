#include <Arduino.h>
#include "ph.h"

// "To" appears to be a temperature compensation pin but its function is currently unclear
#define PH_TO 16 // This is the pin number connected to To
// "Do" appears to be an interrupt pin that goes LOW (3.65V -> 0.77V) when the limit set by the potentiometer near the pins is reached.
// A red LED on the board (near the green one) activates when the pH limit is reached and can be used to calibrate the potentiometer.
#define PH_DO 15 // This is the pin number connected to Do
// "Po" is the raw output value for the pH
#define PH_PO 7 // This is the pin number connected to Po

void initph() {
    Serial.begin(115200);
    Serial.println("pH monitoring setup complete.");
    pinMode(PH_PO, INPUT);
    pinMode(PH_TO, INPUT);
}

float getPHValue(float voltage5V) {
    int measure = analogRead(PH_PO);
    double voltage = (voltage5V / 4095.0) * measure;

    // Read temperature from TO pin
    int tempValue = analogRead(PH_TO);
    float tempVoltage = (voltage5V / 4095.0) * tempValue; // Use passed voltage5V for temperature reading
    float tempSensorValue = tempVoltage * 100.0; // Example conversion, adjust based on sensor specifics

    // Example known ambient temperature (adjust as needed)
    float ambientTemperature = 25.0;

    // Calculate offset
    float offset = ambientTemperature - tempSensorValue;

    // Apply temperature compensation (example formula, adjust based on specifics)
    float compensatedVoltage = voltage + (offset * 0.01); // Example adjustment

    // Use voltage5V / 2 as the midpoint instead of assuming 2.5
    float midpoint = voltage5V / 2.0;
    float Po = 7 + ((midpoint - compensatedVoltage) / 0.18);
    return Po;
}
