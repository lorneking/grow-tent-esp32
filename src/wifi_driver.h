#ifndef WIFI_DRIVER_H
#define WIFI_DRIVER_H

#include <WiFi.h>
#include <WebServer.h>

class WiFiDriver {
public:
    void begin(const char* ssid, const char* password);
    bool isConnected();
    IPAddress getLocalIP();
    void handleClient();
    void updateSensorValues(float voltage3V3, float voltage5V, float tdsValue, float phValue, float temperature);

private:
    const char* _ssid;
    const char* _password;
    WebServer server; // Define the server member variable
    float _voltage3V3;
    float _voltage5V;
    float _tdsValue;
    float _phValue;
    float _temperature;
    void handleRoot();
    void handleData(); // Declare the handleData function
    void handleNotFound();
};

#endif // WIFI_DRIVER_H
