#include "wifi_driver.h"

void WiFiDriver::begin(const char* ssid, const char* password) {
    _ssid = ssid;
    _password = password;

    Serial.begin(115200);
    delay(10);

    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(_ssid);

    WiFi.begin(_ssid, _password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("Wi-Fi connected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

bool WiFiDriver::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

IPAddress WiFiDriver::getLocalIP() {
    return WiFi.localIP();
}

void WiFiDriver::updateSensorValues(float voltage3V3, float voltage5V, float tdsValue, float phValue, float temperature, float ds18b20Temperature) {
    _voltage3V3 = voltage3V3;
    _voltage5V = voltage5V;
    _tdsValue = tdsValue;
    _phValue = phValue;
    _temperature = temperature;
    _ds18b20Temperature = ds18b20Temperature;
}
