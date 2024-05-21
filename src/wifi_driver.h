#ifndef WIFI_DRIVER_H
#define WIFI_DRIVER_H

#include <WiFi.h>

class WiFiDriver {
public:
    void begin(const char* ssid, const char* password);
    bool isConnected();
    IPAddress getLocalIP();
    void updateSensorValues(float voltage3V3, float voltage5V, float tdsValue, float phValue, float temperature, float ds18b20Temperature);

private:
    const char* _ssid;
    const char* _password;
    float _voltage3V3;
    float _voltage5V;
    float _tdsValue;
    float _phValue;
    float _temperature;
    float _ds18b20Temperature;
};

#endif // WIFI_DRIVER_H
