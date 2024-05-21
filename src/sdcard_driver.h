#ifndef SDCARD_DRIVER_H
#define SDCARD_DRIVER_H

#include <Arduino.h>
#include <SD_MMC.h>

class SDCardDriver {
public:
    SDCardDriver();
    bool begin();
    bool writeFile(const char* path, const char* message);
    String readFile(const char* path);
private:
    // No custom CS pin required for built-in slot
};

#endif // SDCARD_DRIVER_H
