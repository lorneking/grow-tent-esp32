#ifndef DS18B20_H
#define DS18B20_H

#include <OneWire.h>
#include <DallasTemperature.h>

class DS18B20 {
public:
    DS18B20(uint8_t pin);
    void begin();
    float getTemperature();
private:
    OneWire oneWire;
    DallasTemperature sensors;
};

#endif // DS18B20_H
