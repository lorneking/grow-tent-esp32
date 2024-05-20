#include <Arduino.h>

// "To" appears to be a temperature compensation pin but its function is currently unclear
#define PH_TO 16 // This is the pin number connected to To
// "Do" appears to be an interrupt pin that goes LOW (3.65V -> 0.77V) when the limit set by the potentiometer near the pins is reached.
// A red LED on the board (near the green one) activates when the pH limit is reached and can be used to calibrate the potentiometer.
#define PH_DO 15 // This is the pin number connected to Do
// "Po" is the raw output value for the pH
#define PH_PO 7 // This is the pin number connected to Po
#define VREF 4.95

#include <Arduino.h>
#include "ph.h"

void initph() {
    Serial.begin(115200);
    Serial.println("pH monitoring setup complete.");
    pinMode(PH_PO, INPUT);
}

float getPHValue() {
    int measure = analogRead(PH_PO);
    double voltage = (3.3 / 4095.0) * measure; // ESP32 uses a 12-bit ADC and a 3.3V reference voltage
    float Po = 7 + ((2.5 - voltage) / 0.18);
    return Po;
}
