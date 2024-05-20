#include <Arduino.h>
#include "sysvoltagemon.h"

#define VREF 3.3           // Analog reference voltage (Volt) of the ADC for ESP32
#define ADC_MAX 4095       // 12-bit ADC
#define VOLTAGE_DIVIDER_RATIO (10.0 + 4.7) / 4.7 // Ratio for the 5V rail voltage divider
#define CORRECTION_FACTOR_5V 0.789 // Correction factor based on observed discrepancy for 5V rail
#define CORRECTION_FACTOR_3V3 1.0067 // Correction factor based on observed discrepancy for 3.3V rail

#define Rail3V3Pin 6       // ADC pin for 3.3V rail
#define Rail5VPin 5       // ADC pin for 5V rail (through voltage divider)

void initSysVoltageMon() {
    Serial.begin(115200);
    Serial.println("Voltage monitoring setup complete.");
    pinMode(Rail3V3Pin, INPUT);
    pinMode(Rail5VPin, INPUT);
}

float getVoltage3V3() {
    int adcValue3V3 = analogRead(Rail3V3Pin);
    float measuredVoltage3V3 = (adcValue3V3 * VREF) / ADC_MAX;
    return measuredVoltage3V3 * CORRECTION_FACTOR_3V3;
}

float getVoltage5V() {
    int adcValue5V = analogRead(Rail5VPin);
    float measuredVoltage5V = (adcValue5V * VREF) / ADC_MAX;
    return measuredVoltage5V * VOLTAGE_DIVIDER_RATIO * CORRECTION_FACTOR_5V;
}
