#include <Arduino.h>
#include "tds.h"

#define TdsSensorPin 5  // GPIO pin for the TDS sensor, using ADC1_CH4 on ESP32-S3
#define VREF 3.3           // Analog reference voltage (Volt) of the ADC for ESP32
#define SCOUNT 30          // Sum of sample points
int analogBuffer[SCOUNT];  // Store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0, copyIndex = 0;
float averageVoltage = 0, tdsValue = 0, temperature = 25;
float calibrationFactor = 1.23; // Replace this with your actual calibration factor

void initTDS() {
    Serial.begin(115200);
    Serial.println("TDS monitoring setup complete.");
    pinMode(TdsSensorPin, INPUT);
}

int getMedianNum(int bArray[], int iFilterLen) {
    int bTab[iFilterLen];
    for (byte i = 0; i < iFilterLen; i++)
        bTab[i] = bArray[i];
    int i, j, bTemp;
    for (j = 0; j < iFilterLen - 1; j++) {
        for (i = 0; i < iFilterLen - j - 1; i++) {
            if (bTab[i] > bTab[i + 1]) {
                bTemp = bTab[i];
                bTab[i] = bTab[i + 1];
                bTab[i + 1] = bTemp;
            }
        }
    }
    if ((iFilterLen & 1) > 0)
        bTemp = bTab[(iFilterLen - 1) / 2];
    else
        bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
    return bTemp;
}

float getTDSValue() {
    static unsigned long analogSampleTimepoint = millis();
    if (millis() - analogSampleTimepoint > 40U) { // Every 40 milliseconds, read the analog value from the ADC
        analogSampleTimepoint = millis();
        analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin); // Read the analog value and store into the buffer
        analogBufferIndex++;
        if (analogBufferIndex == SCOUNT)
            analogBufferIndex = 0;
    }

    static unsigned long printTimepoint = millis();
    if (millis() - printTimepoint > 800U) {
        printTimepoint = millis();
        for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++)
            analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
        averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 4096.0; // Read the analog value more stable by the median filtering algorithm, and convert to voltage value
        float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0); // Temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
        float compensationVoltage = averageVoltage / compensationCoefficient; // Temperature compensation
        tdsValue = (133.42 * compensationVoltage * compensationVoltage * compensationVoltage - 255.86 * compensationVoltage * compensationVoltage + 857.39 * compensationVoltage) * 0.5; // Convert voltage value to TDS value

        // Apply calibration factor
        tdsValue *= calibrationFactor;
    }
    return tdsValue;
}

