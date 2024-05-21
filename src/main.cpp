#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
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

// NTP server to request time from
const char* ntpServer = "pool.ntp.org";
// Time zone offset in seconds (for example, -5*3600 for EST)
const long gmtOffset_sec = 0; // Adjust this to your time zone
const int daylightOffset_sec = 3600; // Daylight offset, adjust if needed

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

    // Synchronize time with NTP
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    Serial.println("Waiting for NTP time sync...");
    struct tm timeinfo;
    while (!getLocalTime(&timeinfo)) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println("\nTime synchronized!");

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

    // Create the CSV header if the file doesn't exist
    File file = SD_MMC.open("/datalog.csv", FILE_READ);
    if (!file) {
        file = SD_MMC.open("/datalog.csv", FILE_WRITE);
        if (file) {
            file.println("Timestamp,3.3V Rail (V),5V Rail (V),TDS Value (ppm),pH Value,Temperature (°C),DS18B20 Temperature (°C)");
            file.close();
        }
    }
    else {
        file.close();
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

    // Get the current time
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return;
    }
    char timeString[30];
    strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &timeinfo);

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

    // Log data to the SD card in CSV format
    String data = String(timeString) + "," + String(voltage3V3, 3) + "," +
                  String(voltage5V, 3) + "," + String(tdsValue, 0) + "," +
                  String(phValue, 3) + "," + String(temperature, 2) + "," +
                  String(ds18b20Temperature, 2) + "\n";
    if (sdCard.writeFile("/datalog.csv", data.c_str())) {
        Serial.println("Data logged to SD card");
    }

    // Handle web server client requests
    wifi.handleClient();

    delay(2000); // Update every 2 seconds
}
