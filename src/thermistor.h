#ifndef THERMISTOR_H
#define THERMISTOR_H

void initThermistor();
float getTemperature(float vref);
int getRawADCValue();
float getThermistorVoltage(float vref);
float getThermistorResistance(float vref);

#endif // THERMISTOR_H
