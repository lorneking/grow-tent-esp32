#include <Arduino.h>
#include "sysvoltagemon.h"
#include "tds.h"
#include "ph.h"

void setup() {
    // Initialize all systems
    initSysVoltageMon();
    initTDS();
    initph();
}

void loop() {
    // Monitor all systems and get values
    float voltage3V3 = getVoltage3V3();
    float voltage5V = getVoltage5V();
    float tdsValue = getTDSValue(voltage5V);
    float phValue = getPHValue(voltage5V);

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

    delay(2000); // Update every 2 seconds
}
