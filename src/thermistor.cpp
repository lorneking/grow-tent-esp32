#include <Arduino.h>
#include "thermistor.h"

#define THERMISTOR_PIN 17   // GPIO pin connected to the thermistor
#define SERIES_RESISTOR 10000.0 // Value of the series resistor (10k ohms)
#define B_COEFFICIENT 7101 // Updated Beta coefficient based on calibration points
#define NOMINAL_RESISTANCE 10000 // Resistance at 25 degrees Celsius (10k ohms)
#define NOMINAL_TEMPERATURE 25.0 // Nominal temperature (25 degrees Celsius)

// Function to initialize the thermistor pin
void initThermistor() {
    pinMode(THERMISTOR_PIN, INPUT);
}

// Function to get the raw ADC value from the thermistor
int getRawADCValue() {
    return analogRead(THERMISTOR_PIN);
}

// Function to calculate the voltage across the thermistor
float getThermistorVoltage(float vref) {
    int adcValue = getRawADCValue();
    return (vref / 4095.0) * adcValue;
}

// Function to calculate the resistance of the thermistor
float getThermistorResistance(float vref) {
    float voltage = getThermistorVoltage(vref);
    return SERIES_RESISTOR * (vref / voltage - 1.0);
}

// Function to get the temperature reading from the thermistor
float getTemperature(float vref) {
    float thermistorResistance = getThermistorResistance(vref);

    // Prevent division by zero or negative resistance values
    if (thermistorResistance <= 0) {
        return -273.15; // Return an error value
    }

    // Debugging prints
    Serial.print("Thermistor Resistance: ");
    Serial.println(thermistorResistance);

    // Calculate the temperature using the Steinhart-Hart equation
    float steinhart;
    steinhart = thermistorResistance / NOMINAL_RESISTANCE;      // (R/Ro)
    steinhart = log(steinhart);                                 // ln(R/Ro)
    steinhart /= B_COEFFICIENT;                                 // 1/B * ln(R/Ro)
    steinhart += 1.0 / (NOMINAL_TEMPERATURE + 273.15);          // + (1/To)
    steinhart = 1.0 / steinhart;                                // Invert
    steinhart -= 273.15;                                        // Convert to Celsius

    // Debugging prints
    Serial.print("Steinhart: ");
    Serial.println(steinhart);

    return steinhart;
}
