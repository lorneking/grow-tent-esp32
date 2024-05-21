#include "i2c_driver.h"

void I2CDriver::begin() {
    Wire.begin(); // Initialize the I2C bus
    Serial.begin(115200); // Initialize serial communication
    while (!Serial); // Wait for the serial connection to be established
    Serial.println("I2C Driver Initialized");
}

void I2CDriver::scan() {
    byte error, address;
    int nDevices;

    Serial.println("Scanning for I2C devices...");

    nDevices = 0;
    for (address = 1; address < 127; address++ ) {
        // The i2c_scanner uses the return value of
        // the Write.endTransmission to see if
        // a device did acknowledge to the address.
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0) {
            Serial.print("I2C device found at address 0x");
            if (address < 16) {
                Serial.print("0");
            }
            Serial.print(address, HEX);
            Serial.println("  !");

            nDevices++;
        } else if (error == 4) {
            Serial.print("Unknown error at address 0x");
            if (address < 16) {
                Serial.print("0");
            }
            Serial.println(address, HEX);
        }
    }
    if (nDevices == 0) {
        Serial.println("No I2C devices found\n");
    } else {
        Serial.println("Scan complete\n");
    }
}
