#ifndef I2C_DRIVER_H
#define I2C_DRIVER_H

#include <Arduino.h>
#include <Wire.h>

class I2CDriver {
public:
    void begin();
    void scan();
};

#endif // I2C_DRIVER_H
