#include <Arduino.h>
#include <WiFi.h>
#include <FS.h>
#include <SD_MMC.h>
#include "time.h"
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
// const char* ntpServer = "pool.ntp.org";
const char* ntpServer = "time.google.com";
// Time zone offset in seconds (for example, -5*3600 for EST)
const long gmtOffset_sec = -5 * 3600; // Adjust this to your time zone
const int daylightOffset_sec = 3600; // Daylight offset, adjust if needed

// Pin where the DS18B20 is connected
const uint8_t DS18B20_PIN = 47; // Corrected pin for DS18B20

// Create an instance of the DS18B20 class
DS18B20 temperatureSensor(DS18B20_PIN);

// Create an instance of the SDCardDriver class
SDCardDriver sdCard;

// Create a mutex for SD card access
SemaphoreHandle_t sdCardMutex;

// Flag to control data logging
bool isLoggingEnabled = true;

// Create an instance of the WiFiDriver class
WiFiDriver wifi;

// Function declarations
void handleSerialCommand();
void printSensorValues();
void printLocalTime();
void downloadFile(String fileName);  // Forward declaration
void deleteFile(String fileName);  // Forward declaration
void createDatalogWithHeaders();  // Function to create datalog with headers

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

    // Print the current local time for debugging
    printLocalTime();

    // Initialize all other systems
    initSysVoltageMon();
    initTDS();
    initph();
    initThermistor();
    
    // Initialize the DS18B20 temperature sensor
    temperatureSensor.begin();
    
    // Initialize the SD card mutex
    sdCardMutex = xSemaphoreCreateMutex();

    // Initialize the SD card
    xSemaphoreTake(sdCardMutex, portMAX_DELAY);
    bool sdCardInitialized = sdCard.begin();
    xSemaphoreGive(sdCardMutex);
    if (!sdCardInitialized) {
        Serial.println("SD card initialization failed!");
        return;
    }

    // Create the CSV header if the file doesn't exist
    createDatalogWithHeaders();
}

void loop() {
    // Handle serial commands
    if (Serial.available() > 0) {
        handleSerialCommand();
    }

    // Perform regular data logging if enabled
    if (isLoggingEnabled) {
        printSensorValues();
    }

    delay(2000); // Update every 2 seconds
}

void handleSerialCommand() {
    String command = Serial.readStringUntil('\n');
    command.trim(); // Remove any trailing whitespace

    if (command.startsWith("DOWNLOAD ")) {
        String fileName = command.substring(9);
        isLoggingEnabled = false;  // Pause data logging
        delay(100);  // Ensure any ongoing logging is completed
        downloadFile(fileName);
        isLoggingEnabled = true;  // Resume data logging
    } else if (command.startsWith("DELETE ")) {
        String fileName = command.substring(7);
        isLoggingEnabled = false;  // Pause data logging
        delay(100);  // Ensure any ongoing logging is completed
        deleteFile(fileName);
        isLoggingEnabled = true;  // Resume data logging
    } else if (command == "READ") {
        printSensorValues();
    } else if (command.startsWith("LOG ")) {
        String message = command.substring(4);
        xSemaphoreTake(sdCardMutex, portMAX_DELAY);
        sdCard.writeFile("/datalog.csv", (message + "\n").c_str());
        xSemaphoreGive(sdCardMutex);
        Serial.println("Message logged to SD card");
    } else if (command == "RESET") {
        // Example command to reset sensors
        initSysVoltageMon();
        initTDS();
        initph();
        initThermistor();
        temperatureSensor.begin();
        Serial.println("Sensors reset");
    } else if (command == "STOP_LOGGING") {
        // Example command to stop logging data
        isLoggingEnabled = false;
        Serial.println("Data logging stopped");
    } else if (command == "START_LOGGING") {
        // Example command to start logging data
        isLoggingEnabled = true;
        Serial.println("Data logging started");
    } else {
        Serial.println("Unknown command");
    }
}

void downloadFile(String fileName) {
    xSemaphoreTake(sdCardMutex, portMAX_DELAY);
    File file = SD_MMC.open("/" + fileName, FILE_READ);
    if (file) {
        Serial.println("START");
        while (file.available()) {
            Serial.write(file.read());
        }
        Serial.println("END");
        file.close();
    } else {
        Serial.println("Failed to open file: " + fileName);
    }
    xSemaphoreGive(sdCardMutex);
}

void deleteFile(String fileName) {
    Serial.println("Attempting to delete file: " + fileName);
    xSemaphoreTake(sdCardMutex, portMAX_DELAY);
    if (SD_MMC.remove("/" + fileName)) {
        Serial.println("DELETE_SUCCESS");
        // Recreate the file with headers
        createDatalogWithHeaders();
    } else {
        Serial.println("DELETE_FAILED");
    }
    xSemaphoreGive(sdCardMutex);
}

void createDatalogWithHeaders() {
    xSemaphoreTake(sdCardMutex, portMAX_DELAY);
    File file = SD_MMC.open("/datalog.csv", FILE_WRITE);
    if (file) {
        file.println("Timestamp,3.3V Rail (V),5V Rail (V),TDS Value (ppm),pH Value,Temperature (°C),DS18B20 Temperature (°C)");
        file.close();
        Serial.println("Datalog file created with headers.");
    } else {
        Serial.println("Failed to create datalog file with headers.");
    }
    xSemaphoreGive(sdCardMutex);
}

void printSensorValues() {
    // Monitor all systems and get values
    float voltage3V3 = getVoltage3V3();
    float voltage5V = getVoltage5V();
    float tdsValue = getTDSValue(voltage5V);
    float phValue = getPHValue(voltage5V);
    float temperature = getTemperature(voltage5V); // Use the 5V reference for the thermistor
    float ds18b20Temperature = temperatureSensor.getTemperature(); // Get temperature from DS18B20

    // Get the current time
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return;
    }
    char timeString[30];
    strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &timeinfo);

    // Print the readings in a tidy format
    Serial.print("Timestamp: ");
    Serial.println(timeString);
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
    Serial.print("DS18B20 Temperature: ");
    Serial.print(ds18b20Temperature, 2);
    Serial.println(" °C");

    // Log data to the SD card in CSV format
    String data = String(timeString) + "," + String(voltage3V3, 3) + "," +
                  String(voltage5V, 3) + "," + String(tdsValue, 0) + "," +
                  String(phValue, 3) + "," + String(temperature, 2) + "," +
                  String(ds18b20Temperature, 2) + "\n";
    xSemaphoreTake(sdCardMutex, portMAX_DELAY);
    if (sdCard.writeFile("/datalog.csv", data.c_str())) {
        Serial.println("Data logged to SD card");
    }
    xSemaphoreGive(sdCardMutex);

    // Update sensor values in WiFiDriver
    wifi.updateSensorValues(voltage3V3, voltage5V, tdsValue, phValue, temperature, ds18b20Temperature);
}

void printLocalTime() {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
    } else {
        Serial.println("Failed to obtain time");
    }
}
