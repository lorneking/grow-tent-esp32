#include <Arduino.h>
#include "sysvoltagemon.h"
#include "tds.h"
#include "ph.h"
#include "thermistor.h"

void setup() {
    // Initialize all systems
    Serial.begin(115200);
    initSysVoltageMon();
    initTDS();
    initph();
    initThermistor();
}

void loop() {
    // Monitor all systems and get values
    float voltage3V3 = getVoltage3V3();
    float voltage5V = getVoltage5V();
    float tdsValue = getTDSValue(voltage5V);
    float phValue = getPHValue(voltage5V);
    float temperature = getTemperature(voltage5V); // Use the 5V reference for the thermistor

    // Print the readings in a tidy format
    Serial.print("3.3V Rail: ");
    Serial.print(voltage3V3, 3);
    Serial.println(" V");

    Serial.print("5V Rail: ");
    Serial.print(voltage5V, 3);
    Serial.println(" V");

    Serial.print("TDS Value: ");
    Serial.print(tdsValue, 0);
    Serial.println(" ppm");

    Serial.print("pH Value: ");
    Serial.print(phValue, 3);
    Serial.println("");

    Serial.print("Temperature: ");
    Serial.print(temperature, 2);
    Serial.println(" °C");

    delay(2000); // Update every 2 seconds
}
