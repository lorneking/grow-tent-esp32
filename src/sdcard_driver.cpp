#include "sdcard_driver.h"

#define SD_DATA 40
#define SD_CLK 39
#define SD_CMD 38

SDCardDriver::SDCardDriver() {
}

bool SDCardDriver::begin() {
    // Set the custom pins for SDMMC
    SD_MMC.setPins(SD_CLK, SD_CMD, SD_DATA);

    // Using default SDMMC pin configuration
    if (!SD_MMC.begin("/sdcard", true)) {
        Serial.println("Card Mount Failed");
        return false;
    }
    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("No SD card attached");
        return false;
    }
    Serial.println("SD card initialized.");
    return true;
}

bool SDCardDriver::writeFile(const char* path, const char* message) {
    File file = SD_MMC.open(path, FILE_APPEND);
    if (!file) {
        Serial.println("Failed to open file for writing");
        return false;
    }
    if (file.print(message)) {
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
        return false;
    }
    file.close();
    return true;
}

String SDCardDriver::readFile(const char* path) {
    File file = SD_MMC.open(path);
    if (!file) {
        Serial.println("Failed to open file for reading");
        return "";
    }
    String content;
    while (file.available()) {
        content += char(file.read());
    }
    file.close();
    return content;
}
