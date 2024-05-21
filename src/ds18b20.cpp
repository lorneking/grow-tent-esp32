#include "ds18b20.h"

DS18B20::DS18B20(uint8_t pin) : oneWire(pin), sensors(&oneWire) {
}

void DS18B20::begin() {
    sensors.begin();
}

float DS18B20::getTemperature() {
    sensors.requestTemperatures();
    return sensors.getTempCByIndex(0);
}
