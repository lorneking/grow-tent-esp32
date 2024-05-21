#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
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
const char* ntpServer = "pool.ntp.org";
// Time zone offset in seconds (for example, -5*3600 for EST)
const long gmtOffset_sec = -5 * 3600; // Adjust this to your time zone
const int daylightOffset_sec = 3600; // Daylight offset, adjust if needed

WebServer server(80); // Create a web server object that listens on port 80

// Create an instance of the WiFiDriver class
WiFiDriver wifi;

// Pin where the DS18B20 is connected
const uint8_t DS18B20_PIN = 47; // Corrected pin for DS18B20

// Create an instance of the DS18B20 class
DS18B20 temperatureSensor(DS18B20_PIN);

// Create an instance of the SDCardDriver class
SDCardDriver sdCard;

// Create a mutex for SD card access
SemaphoreHandle_t sdCardMutex;

// Function declarations
void handleSerialCommand();
void printSensorValues();
void printLocalTime();
void listFiles();
void serveFile();

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
    xSemaphoreTake(sdCardMutex, portMAX_DELAY);
    File file = SD_MMC.open("/datalog.csv", FILE_READ);
    if (!file) {
        file = SD_MMC.open("/datalog.csv", FILE_WRITE);
        if (file) {
            file.println("Timestamp,3.3V Rail (V),5V Rail (V),TDS Value (ppm),pH Value,Temperature (°C),DS18B20 Temperature (°C)");
            file.close();
        } else {
            Serial.println("Failed to create datalog.csv");
        }
    } else {
        file.close();
    }
    xSemaphoreGive(sdCardMutex);

    // Start the web server
    server.on("/", listFiles);
    server.on("/file", HTTP_GET, serveFile);
    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    // Reconnect if Wi-Fi is disconnected
    if (!wifi.isConnected()) {
        Serial.println("Disconnected from Wi-Fi, reconnecting...");
        wifi.begin(ssid, password);
    }

    // Handle web server client requests
    server.handleClient();

    // Check for serial commands
    if (Serial.available() > 0) {
        handleSerialCommand();
    }

    // Perform regular data logging
    printSensorValues();

    delay(2000); // Update every 2 seconds
}

void handleSerialCommand() {
    String command = Serial.readStringUntil('\n');
    command.trim(); // Remove any trailing whitespace

    if (command == "READ") {
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
        // Implement your logic to stop logging
        Serial.println("Data logging stopped");
    } else if (command == "START_LOGGING") {
        // Example command to start logging data
        // Implement your logic to start logging
        Serial.println("Data logging started");
    } else {
        Serial.println("Unknown command");
    }
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
}

void printLocalTime() {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
    } else {
        Serial.println("Failed to obtain time");
    }
}

void listFiles() {
    xSemaphoreTake(sdCardMutex, portMAX_DELAY);
    File root = SD_MMC.open("/");
    File file = root.openNextFile();

    String fileList = "<html><body><h1>SD Card File List</h1><ul>";
    while (file) {
        fileList += "<li><a href=\"/file?name=";
        fileList += file.name();
        fileList += "\">";
        fileList += file.name();
        fileList += "</a></li>";
        file = root.openNextFile();
    }
    fileList += "</ul></body></html>";

    server.send(200, "text/html", fileList);
    xSemaphoreGive(sdCardMutex);
}

void serveFile() {
    if (server.hasArg("name")) {
        String fileName = server.arg("name");
        xSemaphoreTake(sdCardMutex, portMAX_DELAY);
        File file = SD_MMC.open(fileName.c_str(), FILE_READ);
        if (file) {
            server.streamFile(file, "application/octet-stream");
            file.close();
        } else {
            server.send(404, "text/plain", "File not found");
        }
        xSemaphoreGive(sdCardMutex);
    } else {
        server.send(400, "text/plain", "Bad Request");
    }
}
