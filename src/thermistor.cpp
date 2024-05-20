#include <Arduino.h>
#include "thermistor.h"

#define THERMISTOR_PIN 17   // GPIO pin connected to the thermistor
#define SERIES_RESISTOR 10000.0 // Value of the series resistor (10k ohms)
#define B_COEFFICIENT 3950 // Beta coefficient of the thermistor
#define NOMINAL_RESISTANCE 10000 // Resistance at 25 degrees Celsius (10k ohms)
#define NOMINAL_TEMPERATURE 25.0 // Nominal temperature (25 degrees Celsius)

// Function to initialize the thermistor pin
void initThermistor() {
    pinMode(THERMISTOR_PIN, INPUT);
}

// Function to get the temperature reading from the thermistor
float getTemperature(float vref) {
    int adcValue = analogRead(THERMISTOR_PIN);
    float voltage = (vref / 4095.0) * adcValue;

    // Print ADC value and voltage for debugging
    Serial.print("ADC Value: ");
    Serial.println(adcValue);
    Serial.print("Voltage: ");
    Serial.println(voltage);

    // Prevent division by zero in case voltage is very low
    if (voltage == 0) {
        Serial.println("Voltage is zero, returning error value.");
        return -273.15;
    }

    // Calculate the resistance of the thermistor
    float thermistorResistance = SERIES_RESISTOR * (vref / voltage - 1.0);
    
    // Print thermistor resistance for debugging
    Serial.print("Thermistor Resistance: ");
    Serial.println(thermistorResistance);

    // Ensure resistance is positive and non-zero before taking the log
    if (thermistorResistance <= 0) {
        Serial.println("Invalid thermistor resistance, returning error value.");
        return -273.15;
    }

    // Calculate the temperature using the Steinhart-Hart equation
    float steinhart;
    steinhart = thermistorResistance / NOMINAL_RESISTANCE;      // (R/Ro)
    steinhart = log(steinhart);                                 // ln(R/Ro)
    steinhart /= B_COEFFICIENT;                                 // 1/B * ln(R/Ro)
    steinhart += 1.0 / (NOMINAL_TEMPERATURE + 273.15);          // + (1/To)
    steinhart = 1.0 / steinhart;                                // Invert
    steinhart -= 273.15;                                        // Convert to Celsius

    return steinhart;
}
