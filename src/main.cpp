#include <Arduino.h>
#include "sysvoltagemon.h"
#include "tds.h"
#include "ph.h"
#include "thermistor.h"
#include "wifi_driver.h"
#include "ds18b20.h"
#include "sdcard_driver.h"  // Include the SD card driver

// Replace with your network credentials
const char* ssid = "Apocalypse";
const char* password = "JS283ac$";

// Create an instance of the WiFiDriver class
WiFiDriver wifi;

// Pin where the DS18B20 is connected
const uint8_t DS18B20_PIN = 47; // Corrected pin for DS18B20

// Create an instance of the DS18B20 class
DS18B20 temperatureSensor(DS18B20_PIN);

// Create an instance of the SDCardDriver class
SDCardDriver sdCard;

void setup() {
    // Start the Serial Monitor
    Serial.begin(115200);
    delay(10);

    // Initialize the Wi-Fi driver and connect to Wi-Fi
    wifi.begin(ssid, password);

    // Initialize all other systems
    initSysVoltageMon();
    initTDS();
    initph();
    initThermistor();
    
    // Initialize the DS18B20 temperature sensor
    temperatureSensor.begin();
    
    // Initialize the SD card
    if (!sdCard.begin()) {
        Serial.println("SD card initialization failed!");
        return;
    }
}

void loop() {
    // Reconnect if Wi-Fi is disconnected
    if (!wifi.isConnected()) {
        Serial.println("Disconnected from Wi-Fi, reconnecting...");
        wifi.begin(ssid, password);
    }

    // Monitor all systems and get values
    float voltage3V3 = getVoltage3V3();
    float voltage5V = getVoltage5V();
    float tdsValue = getTDSValue(voltage5V);
    float phValue = getPHValue(voltage5V);
    float temperature = getTemperature(voltage5V); // Use the 5V reference for the thermistor
    float ds18b20Temperature = temperatureSensor.getTemperature(); // Get temperature from DS18B20

    // Update the Wi-Fi driver with the latest sensor values
    wifi.updateSensorValues(voltage3V3, voltage5V, tdsValue, phValue, ds18b20Temperature);

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

    // Print the raw ADC value, voltage, and resistance for calibration
    int adcValue = getRawADCValue();
    float thermistorVoltage = getThermistorVoltage(voltage5V);
    float thermistorResistance = getThermistorResistance(voltage5V);

    Serial.print("Raw ADC Value: ");
    Serial.println(adcValue);
    Serial.print("Thermistor Voltage: ");
    Serial.println(thermistorVoltage);
    Serial.print("Thermistor Resistance: ");
    Serial.println(thermistorResistance);

    Serial.print("Temperature: ");
    Serial.print(temperature, 2);
    Serial.println(" °C");

    Serial.print("DS18B20 Temperature: ");
    Serial.print(ds18b20Temperature, 2);
    Serial.println(" °C");

    // Log data to the SD card
    String data = "3.3V Rail: " + String(voltage3V3, 3) + " V, " +
                  "5V Rail: " + String(voltage5V, 3) + " V, " +
                  "TDS Value: " + String(tdsValue, 0) + " ppm, " +
                  "pH Value: " + String(phValue, 3) + ", " +
                  "Temperature: " + String(temperature, 2) + " °C, " +
                  "DS18B20 Temperature: " + String(ds18b20Temperature, 2) + " °C\n";
    if (sdCard.writeFile("/datalog.txt", data.c_str())) {
        Serial.println("Data logged to SD card");
    }

    // Handle web server client requests
    wifi.handleClient();

    delay(2000); // Update every 2 seconds
}
